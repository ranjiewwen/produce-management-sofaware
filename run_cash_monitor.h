#pragma once

#include "thread.h"
#include "bitmap.h"
#include <vector>
#include <list>
#include "observer.h"

class DataConnection;
class DataPacket;

// class Cash
enum {
  ADC_CHANNEL_BM = 0,
  ADC_CHANNEL_LSM,
  ADC_CHANNEL_RSM,
  ADC_CHANNEL_LM,
  ADC_CHANNEL_RM,
  ADC_CHANNEL_EIR1,
  ADC_CHANNEL_EIR2,
  ADC_CHANNEL_EIR3,
  ADC_CHANNEL_EIR4,
  ADC_CHANNEL_EIR5,
  ADC_CHANNEL_EIR6,
  ADC_CHANNEL_EIR7,
  ADC_CHANNEL_IR1,
  ADC_CHANNEL_IR2,
  ADC_CHANNEL_IR3,
  ADC_CHANNEL_IR4,
  ADC_CHANNEL_IR5,
  ADC_CHANNEL_IR6,
  ADC_CHANNEL_UV,
  ADC_CHANNEL_UVL,
  ADC_CHANNEL_UVR,
  ADC_CHANNEL_HD,
  ADC_CHANNEL_COUNT
};

struct ADCChannel {
  TCHAR name[64];  
  int minValue, maxValue;
  int id;
};

extern ADCChannel g_ADCChannels[];

// class Cash
class Cash {
public:
  enum {
    VALID_INFO = 1,
    VALID_ADC = 2,
    VALID_IMAGE = 4,
  };

  Cash(int id);
  virtual ~Cash();

  int Id() const;
  void SetADCData(void *data, int length);
  void SetCISData(void *data, int length);
  void SetCashInfo(void *data, int length);

  short GetADCSampleCount(int channel);
  const short *GetADCSampleCodes(int channel);
  const short *GetADCSampleValues(int channel);
  const void *GetADCOriginalData(int &length);
  Bitmap *GetSNImage();
  Bitmap *GetTopImage();
  Bitmap *GetBottomImage();

  bool Valid(int flags) const;
  int GetDirection() const;
  int GetDenomination() const;
  int GetVersion() const;
  int GetError() const;
  const char *GetSN() const;

private:
  struct ADCChannel {
    short count;
    short *codes;
    short *values;
  };  
  int         valid_;
  int         id_;
  int         direction_;
  int         denomination_;
  int         version_;
  int         result_;
  int         error_;
  char        sn_[24];
  Bitmap      snImage_;
  void        *adcData_;
  int		  adcDataLength_;
  ADCChannel  adcChannels_[ADC_CHANNEL_COUNT];
  Bitmap      topImage_;
  Bitmap      bottomImage_;

  void CorrectCISImages();
  void CorrectCISImages(int downSampleRate);
};

// class RunCashMonitor
class RunCashMonitor : public Observable {
public:
  enum {
    SUBJECT_CASH_INFO,
    SUBJECT_BEGIN_BUNDLE,
    SUBJECT_ADC_READY,
    SUBJECT_IMAGE_READY,    
    SUBJECT_END_BUNDLE,    
  };
  static RunCashMonitor *GetInstance();  
  void Release();

  int Freeze();
  Cash *GetCash(int index) const;
  void Unfreeze();

private:
  typedef std::vector<Cash *> Cashes;
  
  enum {
    ID_ADC_DATA,
    ID_CIS_DATA,
    ID_CASH_INFO,
    ID_BEGIN_BUNDLE,
    ID_END_BUNDLE,
  };

  static RunCashMonitor *instance_;

  int             refCount_;
  DataConnection  *connection_;
  Thread          thread_;
  CriticalSection criSec_;
  Cashes          cashes_;
  
  RunCashMonitor();
  virtual ~RunCashMonitor();

  bool Start();
  void Stop();
  DWORD Run();

  void OnBeginBundle();
  void OnEndBundle();
  void OnADCData(DataPacket *packet);
  void OnCISData(DataPacket *packet);
  void OnCashInfo(DataPacket *packet);
};