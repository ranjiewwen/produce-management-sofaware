#include "stdafx.h"
#include "run_cash_monitor.h"
#include "device_proxy.h"
#include "device_proxy_inl.h"
#include "thread_inl.h"
#include "run_cash_monitor_inl.h"
#include "debug_logger.h"
#include "local_file_saver.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ADCChannel g_ADCChannels[] = {
  _T("长磁"), 0, 4096, ADC_CHANNEL_BM,
  _T("HD"), 0, 4096, ADC_CHANNEL_HD,
  _T("右中磁"), 0, 4096, ADC_CHANNEL_RM,
  _T("左中磁"), 0, 4096, ADC_CHANNEL_LM,
  _T("右边磁"), 0, 4096, ADC_CHANNEL_RSM,
  _T("左边磁"), 0, 4096, ADC_CHANNEL_LSM,
  _T("红外6"), 0, 5100, ADC_CHANNEL_IR1,
  _T("红外5"), 0, 5100, ADC_CHANNEL_IR2,
  _T("红外4"), 0, 5100, ADC_CHANNEL_IR3,
  _T("红外3"), 0, 5100, ADC_CHANNEL_IR4,
  _T("红外2"), 0, 5100, ADC_CHANNEL_IR5,
  _T("红外1"), 0, 5100, ADC_CHANNEL_IR6,
  _T("EIR1"), 0, 4096, ADC_CHANNEL_EIR1,
  _T("EIR2"), 0, 4096, ADC_CHANNEL_EIR2,
  _T("EIR3"), 0, 4096, ADC_CHANNEL_EIR3,
  _T("EIR4"), 0, 4096, ADC_CHANNEL_EIR4,
  _T("EIR5"), 0, 4096, ADC_CHANNEL_EIR5,
  _T("EIR6"), 0, 4096, ADC_CHANNEL_EIR6,
  _T("中紫外"), 0, 4096, ADC_CHANNEL_UV,
  _T("左右紫外"), 0, 4096, ADC_CHANNEL_UVL,
};

// RunCashMonitor
RunCashMonitor *RunCashMonitor::instance_ = NULL;

RunCashMonitor *RunCashMonitor::GetInstance() {
  if (instance_ == NULL) {
    instance_ = new RunCashMonitor;
    instance_->Start();
  } else {
    instance_->refCount_++;
  }
  return instance_;
}

void RunCashMonitor::Release() {
  ASSERT(instance_ == this);

  if (--refCount_ == 0) {
    delete this;
    instance_ = NULL;
  }
}

RunCashMonitor::RunCashMonitor()
  : refCount_(1)
  , connection_(NULL) {  
}

RunCashMonitor::~RunCashMonitor() {
  Stop();
}

int RunCashMonitor::Freeze() {
  criSec_.Enter();

  return (int)cashes_.size();
}

Cash *RunCashMonitor::GetCash(int index) const {
  ASSERT(index >= 0 && index < (int)cashes_.size());
  return cashes_[index];
}

void RunCashMonitor::Unfreeze() {
  criSec_.Leave();
}

bool RunCashMonitor::Start() {
  if (0) {
    int cisDataLength = 8 + 720 * 3 * 360 * 2;
    char *cisData = new char[cisDataLength];
    ((int *)cisData)[0] = 720 * 3;
    ((int *)cisData)[1] = 360;
    for (int i = 0; i < 10; i++) {
      Cash *cash = new Cash(i + 1);
      cash->SetCISData(cisData, cisDataLength);
      cashes_.push_back(cash);
    }
  }

  connection_ = DeviceProxy::GetInstance()->StartRunCashDetect();
  if (connection_ == NULL) {
    return false;
  }  

  return thread_.Start(new MethodRunnable<RunCashMonitor>(this, &RunCashMonitor::Run));
}

void RunCashMonitor::Stop() {
  if (connection_ != NULL) {
    connection_->Close();
  }

  thread_.Interrupt();
  thread_.WaitForExit(INFINITE);

  if (connection_ != NULL) {
    delete connection_;
  }
}

DWORD RunCashMonitor::Run() {
  DataPacket packet;

  while (!thread_.IsInterrupted()) {
    if (connection_->ReadPacket(&packet)) {
      TRACE("Received a packet: %d\n", packet.GetType());

      switch (packet.GetType()) {
        case ID_BEGIN_BUNDLE:  //3：走钞开始信号
          OnBeginBundle();
          break;
        case ID_END_BUNDLE:  //4：提钞信号
          OnEndBundle();
          break;
        case ID_ADC_DATA:   //0: 主控数据
          OnADCData(&packet);
          break;
        case ID_CIS_DATA:  //1：图像数据
          OnCISData(&packet);
          break;
        case ID_CASH_INFO: //2：钞票信息数据
          OnCashInfo(&packet);
          break;
        default:
          TRACE("WARNING: RunCashMonitor received an unknown packet ID=%d;Length=%d\n", 
              packet.GetType(), packet.GetLength());
      }
    } else {
      Sleep(100);
    }
  }
  return 0;
}

void RunCashMonitor::OnBeginBundle() {
  TRACE("RunCashMonitor::OnBeginBundle\n");

  CriticalSection::ScopedLocker locker(criSec_);

  for (Cashes::iterator i = cashes_.begin(); i != cashes_.end(); ++i) {
    delete (*i);
  }
  cashes_.clear();

  // 在新的路径下保存本地数据 lux
  LocalFileSaver::GetInstance()->CreateCashSavePath();

  NotifyUpdate(SUBJECT_BEGIN_BUNDLE);
}

void RunCashMonitor::OnEndBundle() {
  TRACE("RunCashMonitor::OnEndBundle\n");

  // 保存本地数据 lux
  for (Cashes::iterator i = cashes_.begin(); i != cashes_.end(); i++)
  {
	  Cash *cash = *i;

	  LocalFileSaver::GetInstance()->SaveADCData(cash);
	  LocalFileSaver::GetInstance()->SaveCISData(cash);
	  LocalFileSaver::GetInstance()->SaveCashInfo(cash);
  }


  /*try {
    CFile outFile(_T("d:\\cashes.txt"), CFile::typeBinary | CFile::modeCreate | CFile::modeWrite);
    
    CriticalSection::ScopedLocker locker(criSec_);
    const char header[] = "NO\tSN\tDenomination\tVersion\tDirection\tError\r\n";
    outFile.Write(header, strlen(header));
    for (Cashes::iterator i = cashes_.begin(); i != cashes_.end(); ++i) {
      const Cash *cash = *i;
      CString line;
      line.Format(_T("%d\t%s\t%d\t%02d\t%d\t%d\r\n"), 
          cash->Id(), 
          (LPCTSTR)CA2T(cash->GetSN()), 
          cash->GetDenomination(), 
          cash->GetVersion(),
          cash->GetDirection(),
          cash->GetError());
      CT2AEX<> encodingLine(line, CP_UTF8);
      outFile.Write((const char *)encodingLine, strlen(encodingLine));
    }

    outFile.Close();
  } catch (CFileException *e) {
    e->Delete();
  }*/
  NotifyUpdate(SUBJECT_END_BUNDLE);
 
}

void RunCashMonitor::OnADCData(DataPacket *packet) {
  TRACE("RunCashMonitor::OnADCData id=%d\n", packet->GetCount());

  CriticalSection::ScopedLocker locker(criSec_);

  Cashes::iterator i = cashes_.begin();
  for (; i != cashes_.end(); ++i) {
    if ((*i)->Id() == packet->GetCount()) {
      break;
    }
  }
  Cash *cash = NULL;
  if (i == cashes_.end()) {
    cash = new Cash(packet->GetCount());
    cashes_.push_back(cash);
  } else {
    cash = *i;
  }
  cash->SetADCData(packet->GetData(), packet->GetLength());

  NotifyUpdate(SUBJECT_ADC_READY);
}

void RunCashMonitor::OnCISData(DataPacket *packet) {
  TRACE("RunCashMonitor::OnCISData id=%d\n", packet->GetCount());

  CriticalSection::ScopedLocker locker(criSec_);

  Cashes::iterator i = cashes_.begin();
  for (; i != cashes_.end(); ++i) {
    if ((*i)->Id() == packet->GetCount()) {
      break;
    }
  }
  Cash *cash = NULL;
  if (i == cashes_.end()) {
    cash = new Cash(packet->GetCount());
    cashes_.push_back(cash);
  } else {
    cash = *i;
  }
  cash->SetCISData(packet->GetData(), packet->GetLength());

  NotifyUpdate(SUBJECT_IMAGE_READY);
}

void RunCashMonitor::OnCashInfo(DataPacket *packet) {
  TRACE("RunCashMonitor::OnCashInfo id=%d\n", packet->GetCount());

  CriticalSection::ScopedLocker locker(criSec_);

  Cashes::iterator i = cashes_.begin();
  for (; i != cashes_.end(); ++i) {
    if ((*i)->Id() == packet->GetCount()) {
      break;
    }
  }
  Cash *cash = NULL;
  if (i == cashes_.end()) {
    cash = new Cash(packet->GetCount());
    cashes_.push_back(cash);
  } else {
    cash = *i;
  }
  cash->SetCashInfo(packet->GetData(), packet->GetLength());

  NotifyUpdate(SUBJECT_CASH_INFO);
}

// Cash
Cash::Cash(int id)
  : id_(id)
  , valid_(0)
  , adcData_(NULL)
{
  memset(adcChannels_, 0, sizeof(adcChannels_));
  memset(sn_, 0, _countof(sn_));
}

Cash::~Cash() {
  if (adcData_ != NULL) {
    free(adcData_);
  }
}

void Cash::SetADCData(void *data, int length) {
  TRACE("Cash::SetADCData\n");

  adcDataLength_ = length;
  adcData_ = malloc(length);
  memcpy(adcData_, data, length);
  void *endPtr = (char *)adcData_ + length;
  short *countPtr = (short *)adcData_;
  short *dataPtr = countPtr + ADC_CHANNEL_COUNT;
  ASSERT(dataPtr <= endPtr);
  for (int i = 0; i < ADC_CHANNEL_COUNT; i++) {
    adcChannels_[i].count = countPtr[i];
    adcChannels_[i].codes = dataPtr;
    adcChannels_[i].values = dataPtr + countPtr[i];
    dataPtr += 2 * countPtr[i];
    ASSERT(dataPtr <= endPtr);
  }

  valid_ |= VALID_ADC;
}

class BitBuffer {
public:
  BitBuffer(int initialSize, int glow);
  virtual ~BitBuffer();

  void WriteBits(unsigned char bits, int count);
  int GetSize() const;

private:
  unsigned char *buffer_;
  int capacity_;
  int glow_;
  int offset_;
  int bitOffset_;
};

BitBuffer::BitBuffer(int initialSize, int glow)
  : buffer_(NULL)
  , capacity_(initialSize)
  , offset_(0)
  , bitOffset_(0)
  , glow_(glow) {
  if (glow_ <= 0) {
    glow_ = 1024; 
  }
  if (capacity_ > 0) {
    buffer_ = (unsigned char *)malloc(capacity_);
  }
}

BitBuffer::~BitBuffer() {
  if (buffer_ != NULL) {
    free(buffer_);
  }
}

inline void BitBuffer::WriteBits(unsigned char bits, int count) {
  while (count > 0) {
    if (offset_ >= capacity_) {
      if (buffer_ == NULL) {
        buffer_ = (unsigned char *)malloc(glow_);
      } else {
        buffer_ = (unsigned char *)realloc(buffer_, capacity_ + glow_);
      }
    }
    
    int n = __min(count, 8 - bitOffset_);
    
    if (bitOffset_ == 0) {
      buffer_[offset_] = bits << (8 - n);
    } else {
      buffer_[offset_] |= ((unsigned char)(bits << (8 - n))) >> bitOffset_;
    }

    bitOffset_ += n;

    if (bitOffset_ >= 8) {
      offset_++;
      bitOffset_ = 0;
    }
    
    count -= n;
    bits >>= n;
  }
}

inline int BitBuffer::GetSize() const {
  int size = offset_;
  if (bitOffset_ > 0) {
    size++;
  }
  return size;
}

class BitReader {
public:
  BitReader(const unsigned char *data, int size);

  unsigned char ReadBits(int count);

private:
  const unsigned char *data_;
  int size_;
  int offset_;
  int bitOffset_;
};
BitReader::BitReader(const unsigned char *data, int size)
  : data_(data), size_(size), offset_(0), bitOffset_(0) {
}
unsigned char BitReader::ReadBits(int count) {
  ASSERT(count <= 8);
  
  unsigned char result = 0;
  while (count > 0) {
    int n = __min(8 - bitOffset_, count);
    if (offset_ >= size_) {
      break;
    }
    result = (result << n) | ((unsigned char)(data_[offset_] << bitOffset_) >> (8 - n));
    bitOffset_ += n;
    if (bitOffset_ >= 8) {
      offset_++;
      bitOffset_ = 0;
    }
    count -= n;
  }
  return result;
}

void Cash::SetCISData(void *data, int length) {
  TRACE("Cash::SetCISData\n");

  #pragma pack(push)
  #pragma pack(1)
  struct CISData {
    int width;
    int height;
    char compression[2];
  };
  #pragma pack(pop)
  if (length < sizeof(CISData)) {
    ASSERT(FALSE);
    return;
  }
  CISData *cisData = (CISData *)data;
  unsigned char *src = (unsigned char *)(cisData + 1);
  topImage_.CreateGrayBitmap(cisData->width, cisData->height);
  bottomImage_.CreateGrayBitmap(cisData->width, cisData->height);
  if (!topImage_.IsValid() || !bottomImage_.IsValid()) {
    return;
  }
  if (cisData->compression[0] != 'V' || cisData->compression[1] != 'R') {
	  int imageSize = cisData->width * cisData->height;
	  for (int i = 0; i < 2; i++)
	  {
		  unsigned char *buf = (unsigned char *)(i == 0 ? bottomImage_.GetBits() : topImage_.GetBits());
		  memcpy(buf, src + i * imageSize, imageSize);
	  }
	  CorrectCISImages(2);
	  valid_ |= VALID_IMAGE;
  }
  else
  {
	  TRACE(_T("Image Compression Rate: %f\n"), (double)(length - sizeof(CISData)) / (cisData->width * cisData->height * 2));

	  BitReader reader(src, length - sizeof(CISData));
	  //int imageSize = cisData->width * cisData->height;
	  for (int i = 0; i < 2; i++) {
		  unsigned char *start = (unsigned char *)(i == 0 ? bottomImage_.GetBits() : topImage_.GetBits());
		  unsigned char *p = start;
		  int x = 0, y = 0;
		  while (x < cisData->width) {
			  unsigned int n;
			  unsigned char c;
			  unsigned char f = reader.ReadBits(1);
			  if (f == 0) {
				  n = 1;
				  c = reader.ReadBits(8);
			  }
			  else {
				  n = reader.ReadBits(8) + 1;
				  c = reader.ReadBits(8);
			  }
			  for (; n > 0 && x < cisData->width; n--) {
				  *p = c;
				  if (++y < cisData->height) {
					  p += cisData->width;
				  }
				  else {
					  x++;
					  p = start + x;
					  y = 0;
				  }
			  }
		  }
	  }

	  if (1) {
		  CorrectCISImages(1);
	  }

	  valid_ |= VALID_IMAGE;

	  /*BitBuffer buffer(imageSize * 2, 1024);
	  unsigned char c, n;
	  int i = 0;
	  while (i < imageSize * 2) {
	  c = pixels[i];
	  n = 0;
	  while (++i < imageSize * 2 && n < 255 && pixels[i] == c) {
	  n++;
	  }
	  if (n == 0) {
	  buffer.WriteBits(0, 1);
	  buffer.WriteBits(c, 8);
	  } else {
	  buffer.WriteBits(1, 1);
	  buffer.WriteBits(n, 8);
	  buffer.WriteBits(c, 8);
	  }
	  }

	  TRACE("Image compress: %d->%d(%0.3f)\n",
	  2 * imageSize, buffer.GetSize(),
	  (double)buffer.GetSize() / (imageSize * 2.0));*/
  }
}

void Cash::SetCashInfo(void *data, int length) {
  TRACE("Cash::SetCashInfo\n");

  struct CashData{
    int count;
    int denomination;
    int version;
    int direction;
    int error;
    char sn[32];
    int snImageSize;
    unsigned int snImage[12][32];
  };
  if (length < sizeof(CashData)) {
    ASSERT(FALSE);
    return;
  }  
  CashData *cashData = (CashData *)data;
  //ASSERT(cashData->snImageSize == 12 * 32 * 4);
  denomination_ = cashData->denomination;
  version_ = cashData->version;
  direction_ = cashData->direction;
  error_ = cashData->error;
  strncpy(sn_, cashData->sn, _countof(sn_));
  snImage_.CreateGrayBitmap(32 * 12, 32);
  if (snImage_.IsValid()) {
    unsigned char *dst = (unsigned char *)snImage_.GetBits();
    if (dst != NULL) {
      ZeroMemory(dst, 32 * 12 * 32);
      for (int ch = 0; ch < 12; ch++) {
        for (int x = 0; x < 32; x++) {
          unsigned int value = cashData->snImage[ch][x];
          unsigned char *p = dst + ch * 32 + x;
          for (int y = 0; y < 32; y++) {
            *p = (((value >> y) & 0x1) ? 255 : 0);
            p += 12 * 32;
          }
        }
      }
    }
  }
  valid_ |= VALID_INFO;

  //if (AfxGetApp()->GetProfileInt(_T("Debug"), _T("Save Image"), 0)) {
    //DebugLogger::GetInstance()->SaveCashToEmbeddedPNG(this);
  //}
}

void Cash::CorrectCISImages(int downSampleRate)
{
	CISCorrectionTable *table = DeviceProxy::GetInstance()->GetCISCorrectionTable();

	for (int side = 0; side < CIS_COUNT; side++) 
	{
		Bitmap *image = (side == 0 ? &bottomImage_ : &topImage_);
		ASSERT(image->GetBitsPerPixel() == 8);
		int width = image->GetWidth();
		int height = image->GetHeight();
		unsigned char *pixel = (unsigned char *)image->GetBits();
		if (width == (CIS_IMAGE_WIDTH / downSampleRate * 3))
		{
			for (int y = 0; y < height; y++)
			{
				pixel += (CIS_IMAGE_WIDTH / downSampleRate); // skip uv channel
				for (int color = 0; color < COLOR_COUNT; color++)
				{
					for (int x = 0; x < (CIS_IMAGE_WIDTH / downSampleRate); x++)
					{
						*pixel = table->data[side][color][x * downSampleRate][*pixel];
						pixel++;
					}
				}
			}
		}
		else
		{
			for (int y = 0; y < height; y++)
			{
				pixel += (CIS_IMAGE_WIDTH / downSampleRate + CIS_DATA_WIDTH); // skip uv channel
				for (int color = 0; color < COLOR_COUNT; color++)
				{
					for (int x = 0; x < CIS_IMAGE_WIDTH / downSampleRate; x++)
					{
						*pixel = table->data[side][color][x * downSampleRate][*pixel];
						pixel++;
					}
					pixel += CIS_DATA_WIDTH;
				}
			}
		}
	}
}