#pragma once

#include "singleton.h"
#include "tcp_socket.h"
#include "observer.h"
#include "thread.h"

#define TOP_SIDE           1
#define BOTTOM_SIDE        2
#define BOTH_SIDE          3

#define SERIAL_NUMBER_LENGTH	  24
#define MODEL_NAME_LENGTH		    24
#define FIRMWARE_VERSION_LENGTH 12
#define DISABLED_VALUE			    -1

enum DebugState {
  DEBUG_NONE = 0,
  DEBUG_UPGRADE_DONE = 0x1,
  DEBUG_IR_CALIBRATION_DONE = 0x2,
  DEBUG_MASTER_SIGNAL_DETECT_DONE = 0x4,
  DEBUG_CIS_LIGHT_DONE = 0x8,
  DEBUG_CIS_CALIBRATION_DONE = 0x10,
  DEBUG_OVI_STUDY_DONE = 0x20,
  DEBUG_TAPE_STUDY_DONE = 0x40,
  DEBUG_RUN_CASH_TEST_DONE = 0x80,
};

#pragma pack(push)
#pragma pack(1)
// struct DeviceInfo
struct DeviceInfo {
  char sn[SERIAL_NUMBER_LENGTH]; // 点钞机序列号
  char model[MODEL_NAME_LENGTH]; // 点钞机型号
  char firmwareVersion[FIRMWARE_VERSION_LENGTH]; // 固件版本
  byte numberOfCIS; // CIS个数
  byte numberOfIR; // 红外对管的个数
  byte numberOfMH; // 磁头个数
  byte reserved;
  int cisColorFlags; //  ir/r/b/g  = bit  3/2/1/0
  short cisImageWidth;
  short cisImageHeight;
  int selfTestState; // 机器自检状态
  int debugState[16]; // 调试状态
};
#pragma pack(pop)

// enum CISLightColor
enum CISLightColor {
  CIS_COLOR_RED,
  CIS_COLOR_GREEN,
  CIS_COLOR_BLUE,
  CIS_COLOR_IR,
  CIS_COLOR_COUNT,
};

enum CISLightColorMask {
  CIS_COLOR_MASK_NONE = 0,
  CIS_COLOR_MASK_RED = 1,
  CIS_COLOR_MASK_GREEN = 2,
  CIS_COLOR_MASK_BLUE = 4,
  CIS_COLOR_MASK_WHITE = 7,
  CIS_COLOR_MASK_IR = 8,
  CIS_COLOR_MASK_UV = 10,
};

enum {
  CIS_SEGMENT_COUNT = 3,
};

// struct CISParameter
struct CISParameter {
  int gain[CIS_SEGMENT_COUNT];
  int offset[CIS_SEGMENT_COUNT];
  int exposureTime[CIS_COLOR_COUNT];
};

enum {
  CIS_TOP,
  CIS_BOTTOM,
  CIS_COUNT,
};

enum {
	CIS_IMAGE_WIDTH = 720,
	CIS_IMAGE_HEIGHT = 360,
	CIS_DATA_WIDTH = 16,
};

// union StudyCompletedState
union StudyCompletedState {
  unsigned int u;
  struct {
    int state100V05 : 1;
    int state50V05 : 1;
  };
};

// class DataPacket
class DataPacket {
public:
  DataPacket();
  virtual ~DataPacket();
  void Create(int type, int count, int length);
  int GetLength() const;
  char *GetData();
  int GetType() const;
  int GetCount() const;

private:
  int   type_;
  int   count_;
  int   length_;
  char  *data_;
  int   capacity_;
};

// class DataConnection
class DataConnection {
public:
  DataConnection();
  virtual ~DataConnection();
  bool Open(LPCTSTR address, int port);
  void Close();
  bool WaitConnect(int waitMilliseconds);
  bool ReadPacket(DataPacket *packet);

private:
  TcpSocket   listenSocket_;
  TcpSocket   dataSocket_;
};

enum WaveDataLevel
{
	WAVE_NO_DATA,
	WAVE_HAS_DATA,
};

enum ImageDataLevel
{
	IMAGE_NO_DATA,
	IMAGE_FULL_DATA,
	IMAGE_DOWNSAMPLE_DATA,
};

#pragma pack(push)
#pragma pack(1)
struct DataLevel
{
	sockaddr_in    hostAddr;
	WaveDataLevel  waveDataLevel;
	ImageDataLevel imageDataLevel;
};
#pragma pack(pop)

// class CommandResult
class CommandResult {
public:
  CommandResult();
  virtual ~CommandResult();

  bool IsOk() const;
  int GetStatus() const;
  int GetDataLength() const;
  const void *GetData() const;
  int GetData(void *buffer, int size);
  void *GetDataBuffer(int length);
  void SetStatus(int code);

private:
  enum { STATIC_BUFFER_SIZE = 128 };

  int   status_;
  char  staticBuffer_[STATIC_BUFFER_SIZE];
  char  *dataBuffer_;
  int   dataLength_;
};

enum {
  COLOR_GREEN,
  COLOR_IR,
  COLOR_COUNT,
};

struct CISCorrectionTable {
  unsigned char data[CIS_COUNT][COLOR_COUNT][CIS_IMAGE_WIDTH][256];
};

// class DeviceProxy
class ProgressTracker;
class Bitmap;

class DeviceProxy : public Observable {
public:
  enum {
    SUBJECT_CONNECTED,
    SUBJECT_DISCONNECTED,
  };

  bool Connect(LPCTSTR address, int port);
  bool IsConnected() const;
  void Disconnect();

  bool GetDeviceInfo(DeviceInfo *info);
  bool GetMacAddr(char *macBuf, unsigned int bufLen);
  bool Upgrade(LPCTSTR firmwareFile, LPCTSTR newVersion, ProgressTracker *tracker);
  bool Restart();
  bool UpdateDebugState(int state, int mask);
  bool Echo();

  bool SetIRParameters(const int *resistances);
  bool GetIRParameters(int *resistances);
  bool GetIRValues(int *values);
  bool GetIRValues(const int *resistances, int *values);
  bool UpdateIRParameters(const int *resistances);

  DataConnection *StartMasterSignalDetect();

  bool LightCIS(int side, int colorMask);
  bool UnlightCIS(int side);
  bool SetCISParameter(const CISParameter parameter[CIS_COUNT]);
  bool TakeCISImage(int id, Bitmap *image);
  bool TakeCISImage(const CISParameter parameter[CIS_COUNT], int id, Bitmap *image);
  bool UpdateCISParameter(const CISParameter parameter[CIS_COUNT]);
  bool UpdateCISCorrectionTable(const unsigned char *table, int size);
  CISCorrectionTable *GetCISCorrectionTable();

  bool GetStudyCompletedState(StudyCompletedState *state);
  DataConnection *StartOVIStudy();
  DataConnection *StartTapeStudy();
  bool TapeStudy(const void *data, int dataLength);

  bool StartMotor(int milliseconds);

  DataConnection *StartRunCashDetect();
  DataConnection *StartSignalCollect();

  bool DisableDebug();
  bool CheckConnection();
  bool SyncClock();
  bool SetSerialNumber(LPCTSTR sn);

private:
  TcpSocket           connection_;
  DeviceInfo          deviceInfo_;
  CISCorrectionTable  cisCorrectionTable_;
  bool                connected_;
  CriticalSection     criSec_;

  DeviceProxy();
  virtual ~DeviceProxy();

  bool SendCommand(int id);
  bool SendCommand(int id, const void *data, int dataLength);
  bool SendCommand(int id, int count, const void *data, int dataLength);
  bool SendCommand(int id, const void *data, int dataLength, CommandResult *result);
  bool SendCommand(int id, int count, const void *data, int dataLength, CommandResult *result);
  bool SendCommandNoResult(int id, int count, const void *data, int dataLength);
  bool ReadResult(CommandResult *result);
  DataConnection *OpenDataConnection(int commandId, const void *data, int dataLength);
  void InitCISCorrectionTable();

  DECLARE_SINGLETON(DeviceProxy)
};