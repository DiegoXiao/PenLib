#include "stdafx.h"
#include <string>
#include <tchar.h>
#include <atlstr.h>
#include <iostream>
#include "pen.h"
long __stdcall Pen::OnConnected(ConnectionInterface* connection) {
  _tprintf(__T("Connect %s success\n"), connection->GetRemoteAddr());
  //初始化指令
  OnPenEvent(connection, 0, 1, PEN_ON_CONNECT, 0);
  return 0;
}

long __stdcall Pen::OnConnectFailed(ConnectionInterface* connection) {
  _tprintf(__T("Connect %s error\n"), connection->GetRemoteAddr());
  connection->Close();
  return 0;
}

long __stdcall Pen::OnRecved(ConnectionInterface* connection, char* buffer, long length) {
  OnPenEvent(connection, reinterpret_cast<byte*>(buffer), length, 0, 0);
  return 0;
}

long __stdcall Pen::OnConnectClose(ConnectionInterface* connection) {
  connection->Close();
  OnPenEvent(connection, 0, 1, PEN_ON_DISCONNECT, 0);
  return 0;
}

std::vector<CString> CallGetALLBluetoothRemoteDeviceAddr() {
  static std::vector<CString> (_stdcall *function)() = 0;
  const TCHAR library_name[] = _TEXT("..\\..\\lib\\PenLib.dll");
  const char function_name[] = "GetALLBluetoothRemoteDeviceAddr";
  HINSTANCE handle_instance = LoadLibrary(library_name);
  if (handle_instance)
    *(FARPROC*)&function = GetProcAddress(handle_instance, function_name);
  else
    return std::vector<CString>();
  if (function == nullptr)
    return std::vector<CString>();
  return function();
}

long CallCreateConnection(ConnectionInterface** connection) {
  static long (_stdcall *function)(ConnectionInterface**) = 0;
  const TCHAR library_name[] = _TEXT("..\\..\\lib\\PenLib.dll");
  const char function_name[] = "CreateConnection";
  HINSTANCE handle_instance = LoadLibrary(library_name);
  if (handle_instance)
    *(FARPROC*)&function = GetProcAddress(handle_instance, function_name);
  else
    return -1;
  if (function == nullptr)
    return -1;
  return function(connection);
}

void OnPenEvent(ConnectionInterface* sender, byte* data, long length, long sub_command, long sub_data) {
  // dispatch packet
  byte* buf = (byte*)malloc(sizeof(byte)*(length));
  memset(buf, 0, length);
  if (!sub_command)
    memcpy(buf, data, sizeof(byte)*length);
  long header_size = 4;
  byte stx = 0xC0;
  //_tprintf(_T("\n"));
  //for (long k = 0; k < length; k++) {
  //_tprintf(_T("%02x "), buf[k]);
  //}
  //_tprintf(_T("\n"));
  for (long i = 0; i < (int)length; i++) {
    if (buf[i] != stx && !sub_command)
      continue;
    CMD m = (CMD)buf[i + 1];
    long cmd = (int)m;
    if (sub_command == PEN_ON_CONNECT)
      cmd = PEN_ON_CONNECT;
    else if (sub_command == PEN_ON_DISCONNECT)
      cmd = PEN_ON_DISCONNECT;
    switch (cmd) {
    case PEN_ON_CONNECT: {
      if (sub_data == WCL_E_SUCCESS) {
        _tprintf(_T("[%s]: Connectin has been established\n"), sender->GetRemoteAddr());
        _tprintf(_T("[%s]: Sending ATI command\n"), sender->GetRemoteAddr());
        char buf[4] = { 0 };
        buf[0] = 'A';
        buf[1] = 'T';
        buf[2] = 'I';
        buf[3] = char(13);
        long res = sender->Send(buf, 4);
        if (res != 4)
          _tprintf(_T("[%s]: Unable send ATI command\n"), sender->GetRemoteAddr());
        else
          _tprintf(_T("[%s]: Command sent success. Waiting for answer\n"), sender->GetRemoteAddr());
      }
      else
        _tprintf(_T("[%s]: Unable establish connection to remote device\n"), sender->GetRemoteAddr());
    }
    break;
    case PEN_ON_DISCONNECT: {
      _tprintf(_T("[%s]: Disconnected\n"), sender->GetRemoteAddr());
    }
    break;
    case A_PenOnState: {
      _tprintf(_T("[%s]: Init\n"), sender->GetRemoteAddr());
      byte data_array[14] = { 0xc0, 0x02, 0x09 ,0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0xc1 };
      sender->Send(reinterpret_cast<char *>(data_array), sizeof(data_array));
      i += buf[i + 3] + header_size;
    }
    break;
    case A_PasswordRequest: {
      _tprintf(_T("[%s]: reqPassword\n"), sender->GetRemoteAddr());
      byte data_array[21] = { 0xc0, 0x0e, 0x10, 0x00, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc1 };
      sender->Send(reinterpret_cast<char *>(data_array), sizeof(data_array));
      i += buf[i + 3] + header_size;
    }
    break;
    case A_DotData: {
      long gab = buf[i + 4];
      long x = buf[i + 5] + (buf[i + 6] << 8);
      long y = buf[i + 7] + (buf[i + 8] << 8);
      long fx = buf[i + 9];
      long fy = buf[i + 10];
      long force = buf[i + 11];
      CString str;
      str.Format(_T("[%s]: DOT %d , %d , %d, %d, %d, %d\n"), sender->GetRemoteAddr(), x, y, fx, fy, force, gab);
      _tprintf(str);
      i += buf[i + 3] + header_size;
    }
    break;
    case A_DotUpDownDataNew: {
      int updown = buf[i + 12];
      if (updown == 0x00) _tprintf(_T("[%s]: Start\n"), sender->GetRemoteAddr());
      if (updown == 0x01) _tprintf(_T("[%s]: End\n"), sender->GetRemoteAddr());
      i += buf[i + 3] + header_size;
    }
    break;
    case A_PenStatusResponse: {
      byte data_array[7] = { 0xc0, 0x0b, 0x02, 0x00, 0x03, 0x00, 0xc1 };
      sender->Send(reinterpret_cast<char *>(data_array), sizeof(data_array));
      i += buf[i + 3] + header_size;
      _tprintf(_T("[%s]: PenStatus\n"), sender->GetRemoteAddr());
    }
    break;
    case A_DotIDChange: {
      long sec = buf[i + 7];
      long owner = buf[i + 4] + (buf[i + 5] << 8) + (buf[i + 6] << 16);
      long note = buf[i + 8] + (buf[i + 9] << 8) + (buf[i + 10] << 16) + (buf[i + 11] << 24);
      long page = buf[i + 12] + (buf[i + 13] << 8) + (buf[i + 14] << 16) + (buf[i + 15] << 24);
      CString str;
      str.Format(_T("[%s]: Page Info:   S: %d , O: %d , N: %d, P: %d\n"), sender->GetRemoteAddr(), sec, owner, note, page);
      _tprintf(str);
      i += buf[i + 3] + header_size;
    }
    break;
    case A_UsingNoteNotifyResponse: {
      _tprintf(_T("[%s]: Validpage\n"), sender->GetRemoteAddr());
      i += buf[i + 3] + header_size;
    }
    break;
    default: {
      CString str;
      str.Format(_T("[%s]: Not implemented PCMD %x\n"), sender->GetRemoteAddr(), buf[i + 1]);
      _tprintf(str);
    }
    break;
    }
  }
  free(buf);
}

long Pen::Init() {
  //枚举所有设备
  std::vector<CString> addr_vector = CallGetALLBluetoothRemoteDeviceAddr();
  if (addr_vector.size() == 0) {
    _tprintf(__T("No remote device error\n"));
    return -1;
  }
  std::string command;
  int number = addr_vector.size();
  CString string_addr;
  TCHAR addr[20] = { 0 };
  for (int i = 0; i < static_cast<int>(number); i++) {
    string_addr = addr_vector[i];
    memset(addr, 0, sizeof(addr));
    wcscpy_s(addr, 20, string_addr);
    //选择设备
    _tprintf(__T("Device[%s]\n"), addr);
    _tprintf(__T("Please enter 'Y' to choice this device others to skip\n"));
    std::cin >> command;
    if (command == "Y") {
      if (CallCreateConnection(&connection_) < 0) {
        _tprintf(__T("CallCreateConnection error\n"));
        return -1;
      }
      connection_->SetRemoteAddr(addr);
      //初始化回调
      connection_->SetSink(static_cast<ConnectionSinkInterface*>(this));
      break;
    }
  }
  return 0;
}

long Pen::Run() {
  if (connection_) {
    _tprintf(__T("Connect to[%s]...\n"), connection_->GetRemoteAddr());
    if (connection_->Connect() < 0) {
      _tprintf(__T("Connect error\n"));
      return -1;
    }
  }
  return 0;
}

long Pen::Close(){
  if (connection_) {
    connection_->Close();
  }
  return 0;
}

BOOL WINAPI HandlerRounte(DWORD type) {
  //捕获控制台关闭消息
  switch (type) {
  case CTRL_CLOSE_EVENT: {
    pen.Close();
    SetEvent(end_event_);
    return TRUE;
  }
  default:
    return FALSE;
  }
  return FALSE;
}

long _tmain(int argc, _TCHAR* argv[]) {
  //设置窗口消息处理
  SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandlerRounte, TRUE);
  //初始化笔信息
  if (pen.Init() < 0) {
    _tprintf(__T("Pen Init error\n"));
    return -1;
  }
  //选择笔运行其处理程序
  if (pen.Run() < 0) {
    _tprintf(__T("Pen Run error\n"));
    return -1;
  }
  //主线程挂起直到收到窗口关闭信号
  end_event_ = CreateEvent(0, TRUE, FALSE, 0);
  HANDLE handle_event[] = { end_event_ };
  WaitForMultipleObjects(LENGTH(handle_event), handle_event, TRUE, INFINITE);
  return 0;
}