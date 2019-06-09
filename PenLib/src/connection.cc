#include "stdafx.h"
#include <tchar.h>
#include "connection.h"
#pragma comment(lib,"Ws2_32.lib")
#pragma comment ( lib, "Irprops.lib")
#pragma comment ( lib, "Bthprops.lib")
Connection::Connection() {
  memset(&socket_addr_, 0, sizeof(struct _SOCKADDR_BTH));
}

Connection::~Connection() {
  sink_ = nullptr;
}

long Connection::ConnectSocket(const SOCKADDR_BTH& socket_addr_server, long millisecond) {
  if (millisecond == -1) {
    return connect(socket_fd_, (LPSOCKADDR)&socket_addr_server, sizeof(SOCKADDR_BTH));
  }
  ULONG non_blocking = 1;
  ULONG blocking = 0;
  long result = ioctlsocket(socket_fd_, FIONBIO, &non_blocking);
  if (result == SOCKET_ERROR) {
    return result;
  }
  result = SOCKET_ERROR;
  if (connect(socket_fd_, (LPSOCKADDR)&socket_addr_server, sizeof(SOCKADDR_BTH)) == SOCKET_ERROR) {
    struct timeval time_value;
    memset(&time_value, 0, sizeof(struct timeval));
    fd_set write_fd;
    memset(&write_fd, 0, sizeof(struct fd_set));
    // 设置连接超时时间  
    time_value.tv_sec = millisecond / 1000; // 秒数  
    time_value.tv_usec = millisecond % 1000; // 毫秒  
    FD_ZERO(&write_fd);
    FD_SET(socket_fd_, &write_fd);
    result = select(socket_fd_ + 1, 0, &write_fd, 0, &time_value);
    if (result > 0) {
      if (FD_ISSET(socket_fd_, &write_fd)) {
        long error = 0;
        int length = sizeof(error);  
        if (!(getsockopt(socket_fd_, SOL_SOCKET, SO_ERROR, (char*)&error, &length) != 0 || error != 0)) {
          result = 0;
        }
      }
    }
    else if (result == 0) {
      result = -2;
    }
  }
  if (ioctlsocket(socket_fd_, FIONBIO, &blocking) == SOCKET_ERROR) {
    result = SOCKET_ERROR;
  }
  return result;
}

long __stdcall Connection::Connect() {
  socket_fd_ = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
  if (socket_fd_ < 0) {
    _tprintf(__T("socket error\n"));
    return -1;
  }
  memset(&socket_addr_, 0, sizeof(SOCKADDR_BTH));
  TCHAR string_remote_addr[20] = { 0 };
  _tcsncpy_s(string_remote_addr, _countof(string_remote_addr), remote_device_addr_ + 1, 17);
  int length = sizeof(SOCKADDR_BTH);
  if (WSAStringToAddress(string_remote_addr, AF_BTH, 0, (LPSOCKADDR)&socket_addr_, &length) == SOCKET_ERROR) {
    _tprintf(__T("WSAStringToAddress error\n"));
    return -1;
  }
  socket_addr_.addressFamily = AF_BTH;
  //蓝牙笔端口
  socket_addr_.port = BT_PORT_DYN_FIRST;
  socket_addr_.serviceClassId = SerialPortServiceClass_UUID;
  //设置超时时间为10秒
  if (ConnectSocket(socket_addr_, 10000) != 0) {
    _tprintf(__T("Connect error!\n"));
    sink_->OnConnectFailed(this);
    return -1;
  }
  sink_->OnConnected(this);
  DWORD id = 0;
  HANDLE thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RecvThread, (LPVOID)this, 0, &id);
  if (WaitForSingleObject(thread, INFINITE) == WAIT_TIMEOUT) {
    CloseHandle(thread);
    thread = nullptr;
  }
  return 0;
}

long __stdcall Connection::Close() {
  if (socket_fd_) {
    closesocket(socket_fd_);
    socket_fd_ = 0;
    WSACleanup();
  }
  return 0;
}

long __stdcall Connection::SetSink(ConnectionSinkInterface* sink) {
  sink_ = sink;
  return 0;
}

long __stdcall Connection::SetUserData(ReferenceInterface* user_data) {
  user_data_ = user_data;
  return 0;
}

long __stdcall Connection::GetUserData(ReferenceInterface** user_data) {
  if (!user_data_) {
    return -1;
  }
  *user_data = user_data_;
  (*user_data)->AddRef();
  return 0;
}

long __stdcall Connection::SetRemoteAddr(TCHAR* remote_addr) {
  memset(remote_device_addr_, 0, sizeof(remote_device_addr_));
  _sntprintf_s(remote_device_addr_, sizeof(remote_device_addr_), remote_addr);
  return 0;
}

TCHAR* __stdcall Connection::GetRemoteAddr() {
  return remote_device_addr_;
}

long __stdcall Connection::Send(char* data, long length) {
  //设置发送超时
  //long timeout = 2000;//2秒
  //setsockopt(socket_fd_, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(int));
  long result = send(socket_fd_, data, length, 0);
  if (result != length) {
    //中断连接
    sink_->OnConnectClose(this);
    return -1;
  }
  return 0;
}

long Connection::Recv(char* data, long length) {
  //设置接收超时
  //long timeout = 10000;//10秒
  //setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO,(char *)&timeout, sizeof(int));
  return recv(socket_fd_, data, length, 0);
}

long WINAPI Connection::RecvThread(LPVOID parameter)
{
  Connection* connection = static_cast<Connection*>(parameter);
  while (1) {
    char data[1025] = { 0 };
    long result = connection->Recv(data, 1024);
    if (result > 0) {
      connection->sink_->OnRecved(connection, data, result);
    }
    else {
      //中断连接
      connection->sink_->OnConnectClose(connection);
      break;
    }
  }
  return 0;
}

long __stdcall CreateConnection(ConnectionInterface** connection) {
  try {
    ConnectionInterface* connection_interface = new Connection();
    *connection = connection_interface;
    (*connection)->AddRef();
  }
  catch (...) {
    return -1;
  } 
  return 0;
}

std::vector<CString> __stdcall GetALLBluetoothRemoteDeviceAddr() {
  // 网络初始化  
  WORD version_requested = MAKEWORD(2, 2);
  WSADATA wsa_data;
  memset(&wsa_data, 0, sizeof(WSADATA));
  WSAStartup(version_requested, &wsa_data);
  HBLUETOOTH_RADIO_FIND handle_radio_find = nullptr;
  HANDLE handle_Localradio = nullptr;
  HBLUETOOTH_DEVICE_FIND handle_device_find = nullptr;
  BLUETOOTH_FIND_RADIO_PARAMS bluetooth_find_radio_params = { sizeof(BLUETOOTH_FIND_RADIO_PARAMS) };
  BLUETOOTH_RADIO_INFO bluetooth_radio_info = { sizeof(BLUETOOTH_RADIO_INFO) };
  BLUETOOTH_DEVICE_SEARCH_PARAMS bluetooth_device_search_params = { sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS) };
  BLUETOOTH_DEVICE_INFO bluetooth_device_info = { sizeof(BLUETOOTH_DEVICE_INFO) };
  SOCKADDR_BTH socket_addr;
  memset(&socket_addr, 0, sizeof(SOCKADDR_BTH));
  WCHAR addr_name[20] = {0};
  DWORD len = sizeof(addr_name);
  BOOL bluetoothandle_radio_find = FALSE;
  BOOL bluetoothandle_device_find = FALSE;
  std::vector<CString> addrs;

  handle_radio_find = BluetoothFindFirstRadio(&bluetooth_find_radio_params, &handle_Localradio);
  if (handle_radio_find != nullptr) {
    bluetoothandle_radio_find = TRUE;
  }
  else {
    _tprintf(__T("No Radio Error\n"));
    BluetoothFindRadioClose(handle_radio_find);
    CloseHandle(handle_Localradio);
    return std::vector<CString>();
  }
  while (bluetoothandle_radio_find) {
    //本机设备
    if (BluetoothGetRadioInfo(handle_Localradio, &bluetooth_radio_info) == ERROR_SUCCESS) {
      //本机设备地址
      memset(&socket_addr, 0, sizeof(SOCKADDR_BTH));
      socket_addr.addressFamily = AF_BTH;
      socket_addr.btAddr = bluetooth_radio_info.address.ullLong;
      memset(addr_name, 0, sizeof(addr_name));
      if (WSAAddressToString((LPSOCKADDR)&socket_addr, sizeof(SOCKADDR_BTH), 0, addr_name, &len) == SOCKET_ERROR) {
        _tprintf(__T("WSAAddressToString error\n"));
        CloseHandle(handle_Localradio);
        BluetoothFindRadioClose(handle_radio_find);
        return std::vector<CString>();
      }
      bluetooth_device_search_params.hRadio = handle_radio_find;
      bluetooth_device_search_params.fReturnAuthenticated = TRUE;
      bluetooth_device_search_params.fReturnConnected = TRUE;
      bluetooth_device_search_params.fReturnRemembered = TRUE;
      bluetooth_device_search_params.fReturnUnknown = TRUE;
      bluetooth_device_search_params.cTimeoutMultiplier = 30;
      handle_device_find = BluetoothFindFirstDevice(&bluetooth_device_search_params, &bluetooth_device_info);
      bluetoothandle_device_find = FALSE;
      if (handle_device_find != nullptr) {
        bluetoothandle_device_find = TRUE;
      }
      else {
        _tprintf(__T("No Device error\n"));
        BluetoothFindDeviceClose(handle_device_find);
        CloseHandle(handle_Localradio);
        BluetoothFindRadioClose(handle_radio_find);
        return std::vector<CString>();
      }
      while (bluetoothandle_device_find) {
        //远程设备地址
        memset(&socket_addr, 0, sizeof(SOCKADDR_BTH));
        socket_addr.addressFamily = AF_BTH;
        socket_addr.btAddr = bluetooth_device_info.Address.ullLong;
        memset(addr_name, 0, sizeof(addr_name));
        if (WSAAddressToString((LPSOCKADDR)&socket_addr, sizeof(SOCKADDR_BTH), 0, addr_name, &len) == SOCKET_ERROR) {
          _tprintf(__T("WSAAddressToString error\n"));
          BluetoothFindDeviceClose(handle_device_find);
          CloseHandle(handle_Localradio);
          BluetoothFindRadioClose(handle_radio_find);
          return std::vector<CString>();
        }
        //只搜索蓝牙点阵笔的地址	
        if (wcsncmp(addr_name, __T("(9C:7B:D2"), wcslen(__T("(9C:7B:D2"))) == 0) {
          //只搜索链接了的蓝牙点阵笔
          if (bluetooth_device_info.fAuthenticated) {
            _tprintf(__T("Find device %s\n"), addr_name);
            addrs.push_back(addr_name);
          }
        }
        bluetoothandle_device_find = BluetoothFindNextDevice(handle_device_find, &bluetooth_device_info);
      }
      BluetoothFindDeviceClose(handle_device_find);
    }
    CloseHandle(handle_Localradio);
    bluetoothandle_radio_find = BluetoothFindNextRadio(handle_radio_find, &handle_Localradio);
  }
  BluetoothFindRadioClose(handle_radio_find);
  return addrs;
}