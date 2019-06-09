#ifndef PEN_PEN_H_
#define PEN_PEN_H_
#include <vector>
#include "../../include/connection_interface.h"
enum CMD
{
  A_PenOnState = 0x01,
  P_PenOnResponse = 0x02,
  P_RTCset = 0x03,
  A_RTCsetResponse = 0x04,
  P_HoverOnOff = 0x05,
  A_HoverOnOffResponse = 0x06,
  P_ForceCalibrate = 0x07,
  A_ForceCalibrateResponse = 0x08,
  P_AutoShutdownTime = 0x09,
  A_AutoShutdownTimeResponse = 0x0A,
  P_PenSensitivity = 0x2C,
  A_PenSensitivityResponse = 0x2D,
  P_PenColorSet = 0x28,
  A_PenColorSetResponse = 0x29,
  P_AutoPowerOnSet = 0x2A,
  A_AutoPowerOnResponse = 0x2B,
  P_BeepSet = 0x2E,
  A_BeepSetResponse = 0x2F,
  P_UsingNoteNotify = 0x0B,
  A_UsingNoteNotifyResponse = 0x0C,
  A_PasswordRequest = 0x0D,
  P_PasswordResponse = 0x0E,
  P_PasswordSet = 0x0F,
  A_PasswordSetResponse = 0x10,
  A_DotData = 0x11,
  A_DotUpDownData = 0x13,
  P_DotUpDownResponse = 0x14,
  A_DotIDChange = 0x15,
  A_DotUpDownDataNew = 0x16,
  P_PenStatusRequest = 0x21,
  A_PenStatusOldResponse = 0x22,
  A_PenStatusResponse = 0x25,
  P_OfflineDataRequest = 0x47,
  A_OfflineDataInfo = 0x49,
  A_OfflineFileInfo = 0x41,
  P_OfflineFileInfoResponse = 0x42,
  A_OfflineChunk = 0x43,
  P_OfflineChunkResponse = 0x44,
  A_OfflineResultResponse = 0x48,
  P_OfflineNoteList = 0x45,
  A_OfflineNoteListResponse = 0x46,
  P_OfflineDataRemove = 0x4A,
  A_OfflineDataRemoveResponse = 0x4B,
  P_PenSWUpgradeCommand = 0x51,
  A_PenSWUpgradeRequest = 0x52,
  P_PenSWUpgradeResponse = 0x53,
  A_PenSWUpgradeStatus = 0x54
};
#define PEN_PACKET_START  0xC0
#define PEN_PACKET_END    0xC1
#define PEN_ON_CONNECT    10000
#define PEN_ON_DISCONNECT 10001
#define WCL_E_SUCCESS	  0
typedef unsigned char byte;
HANDLE end_event_ = nullptr;
#define LENGTH(x) sizeof(x)/sizeof(x[0])
typedef SharedPointer<ConnectionInterface> ConnectionPtr;
class Pen : public ConnectionSinkInterface {
public:
  long Init();
  long Run();
  long Close();
private:
  virtual long __stdcall OnConnected(ConnectionInterface* connection);
  virtual long __stdcall OnConnectFailed(ConnectionInterface* connection);
  virtual long __stdcall OnRecved(ConnectionInterface* connection, char* buffer, long length);
  virtual long __stdcall OnConnectClose(ConnectionInterface* connection);
  ConnectionPtr connection_ = nullptr;
};
Pen pen;
std::vector<CString> CallGetALLBluetoothRemoteDeviceAddr();
long CallCreateConnection(ConnectionInterface** connection);
void OnPenEvent(ConnectionInterface* sender, byte* data, long length, long sub_command, long sub_data);
BOOL WINAPI HandlerRounte(DWORD type);
#endif
