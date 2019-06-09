#ifndef PEN_LIB_CONNECTION_INTERFACE_H_
#define PEN_LIB_CONNECTION_INTERFACE_H_
#include <vector>
#include <atlstr.h>
#include "reference.h"
struct ConnectionInterface;
struct ConnectionSinkInterface {
  virtual long __stdcall OnConnected(ConnectionInterface* connection) = 0;
  virtual long __stdcall OnConnectFailed(ConnectionInterface* connection) = 0;
  virtual long __stdcall OnRecved(ConnectionInterface* connection, char* buffer, long length) = 0;
  virtual long __stdcall OnConnectClose(ConnectionInterface* connection) = 0;
};
struct ConnectionInterface : ReferenceInterface {
  virtual long __stdcall Connect() = 0;
  virtual long __stdcall Close() = 0;
  virtual long __stdcall SetSink(ConnectionSinkInterface* sink) = 0;
  virtual long __stdcall SetUserData(ReferenceInterface* user_data) = 0;
  virtual long __stdcall GetUserData(ReferenceInterface** user_data) = 0;
  virtual long __stdcall SetRemoteAddr(TCHAR* remote_addr) = 0;
  virtual TCHAR* __stdcall GetRemoteAddr() = 0;
  virtual long __stdcall Send(char* data, long length) = 0;
};
std::vector<CString> __stdcall GetALLBluetoothRemoteDeviceAddr();
long __stdcall CreateConnection(ConnectionInterface** connection);
#endif //PEN_LIB_CONNECTION_INTERFACE_H_