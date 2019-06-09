#ifndef PEN_LIB_CONNECTION_H_
#define PEN_LIB_CONNECTION_H_
#include <Winsock2.h>  
#include <ws2bth.h>
#include <Bthsdpdef.h>  
#include <BluetoothAPIs.h>
#ifndef NS_BTH  
#include "ws2bth.h" //如果没有定义NS_BTH宏，则将PSDK头文件包含进来  
#endif   
#include <vector>
#include "../include/connection_interface.h"
class Connection : public Reference<ConnectionInterface> {
public:
  Connection();
  virtual ~Connection();
  virtual long __stdcall Connect();
  virtual long __stdcall Close();
  virtual long __stdcall SetSink(ConnectionSinkInterface* sink);
  virtual long __stdcall SetUserData(ReferenceInterface* user_data);
  virtual long __stdcall GetUserData(ReferenceInterface** user_data);
  virtual long __stdcall SetRemoteAddr(TCHAR* remote_addr);
  virtual TCHAR* __stdcall GetRemoteAddr();
  virtual long __stdcall Send(char* data, long length);
  long Recv(char* data, long length);
  long ConnectSocket(const SOCKADDR_BTH& socket_addr_server, long millisecond);
  static long WINAPI RecvThread(LPVOID parameter);
private:
  SharedPointer<ReferenceInterface> user_data_;
  ConnectionSinkInterface* sink_ = nullptr;
  SOCKET socket_fd_ = 0;
  SOCKADDR_BTH socket_addr_;
  TCHAR remote_device_addr_[20] = {0};
};
#endif
