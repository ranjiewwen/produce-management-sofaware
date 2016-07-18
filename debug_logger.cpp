#include "stdafx.h"
#include "debug_logger.h"
#include "run_cash_monitor.h"
#include "run_cash_monitor_inl.h"
#include "png.h"
#include "device_proxy.h"
#include "device_proxy_inl.h"
#include "working_parameters.h"
#include "working_parameters_inl.h"
#include "common.h"
#include "error.h"
#include "ir_calibration.h"
#include "sensor_testing.h"
#include "cis_calibrator.h"
#include "Computer_Information.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SINGLETON(DebugLogger)

DebugLogger::DebugLogger() {
  logPath_[0] = _T('\0');
}

DebugLogger::~DebugLogger() {
}

bool DebugLogger::Init() {
  LastErrorHolder errorHolder;

  GetAppDataPath(logPath_);
  PathAddBackslash(logPath_);
  _tcscat(logPath_, _T("log"));
  if (!CreateDirectoryRecusively(logPath_)) {
    errorHolder.SaveLastError();
    return false;
  }
 
  return true;
}

bool DebugLogger::WriteRecord(const char *tag, const void *data, int length) {
  #pragma pack(push)
  #pragma pack(1)
  struct RecordHeader {
    char    sync[4];
    char    tag[4];  
    int     length;
    __int64 timeStamp;
    byte    checkSum;
  };
  #pragma pack(pop)

  RecordHeader hd;
  ZeroMemory(&hd, sizeof(hd));
  strncpy(hd.sync, "CMR ", 4);
  strncpy(hd.tag, tag, 4);
  hd.length = length;
  hd.timeStamp = _time64(NULL);
  
  if (1) { // write record header.
    const byte *p = (const byte *)&hd;
    for (int i = 0; i < sizeof(hd); i++) {
      hd.checkSum += p[i];
    }
  }
  if (length > 0) { // write record data.
    const byte *p = (const byte *)data;
    for (int i = 0; i < length; i++) {
      hd.checkSum += p[i];
    }
  }

  try {
    logFile_.Write(&hd, sizeof(hd));

    if (length > 0) {
      logFile_.Write(data, length);
    }
  } catch (CFileException *e) {
    e->Delete();
    return false;
  }

  return true;
}

bool DebugLogger::IsUploadDetail(int level, int result)
{
	if (level == 3)
	{
		return true;
	}
	else if ((level == 2) && (result != 0))
	{
		return true;
	}

	return false;
}

void DebugLogger::StartDebug() {
  #pragma pack(push)
  #pragma pack(1)
  struct RecordData {
    char            appVersion[16];
    char            sn[SERIAL_NUMBER_LENGTH];
    char            op[64];
    char            offline;
	char			modelID[4];
	char			terminalID[200];
    char            firmwareVersion[FIRMWARE_VERSION_LENGTH];
    int             selfTestState;
    int             debugState[16];
	char			mac[20];
  };
  #pragma pack(pop)

  if (logFile_.m_hFile != INVALID_HANDLE_VALUE) {
    logFile_.Close();
  }

  // 根据可变配置文件中的配置项决定是否上传数据 lux
  VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();
  int uploadLevel = config->GetIntParameter(_T("UploadLevel"), 1);
  if (uploadLevel == 0)
  {
	  return;
  }

  DeviceInfo deviceInfo;
  if (!DeviceProxy::GetInstance()->GetDeviceInfo(&deviceInfo)) {
    return;
  }

  CString fileName(logPath_);
  fileName.AppendChar(_T('\\'));
  for (int i = 0; i < _countof(deviceInfo.sn) && deviceInfo.sn[i] != '\0'; i++) {
    char ch = deviceInfo.sn[i];
    if (ch == ' ' || 
        ch == '\\' || 
        ch == '//' ||
        ch == ':' ||
        ch == '*' || 
        ch == '|' || 
        ch == '?' || 
        ch == '<' || 
        ch == '>') {
      fileName.AppendChar(_T('_'));
    } else {
      fileName.AppendChar(ch);
    }
  }
  fileName.AppendChar(_T('_'));   
  SYSTEMTIME now;
  GetLocalTime(&now);
  fileName.AppendFormat(_T("%04d%02d%02d_%02d%02d%02d.dat"), 
      (int)now.wYear, (int)now.wMonth, (int)now.wDay, 
      (int)now.wHour, (int)now.wMinute, (int)now.wSecond);

  try {
    logFile_.Open(fileName, CFile::typeBinary | CFile::modeCreate | CFile::shareExclusive | CFile::modeWrite);
  } catch (CFileException *e) {
    e->Delete();
  }

  RecordData data;
  ZeroMemory(&data, sizeof(data));
  CString appVersion = AfxGetApp()->GetProfileString(_T(""), _T("Version"));
  strncpy(data.appVersion, CT2A(appVersion, CP_UTF8), _countof(data.appVersion));
  strncpy(data.sn, deviceInfo.sn, _countof(data.sn));
  strncpy(data.op, CT2A(WorkingParameters::GetInstance()->GetUserName(), CP_UTF8), _countof(data.op));
  int curID = WorkingParameters::GetInstance()->GetCurrentModelId();
  strncpy(data.modelID, (char *)&curID, _countof(data.modelID));
  TCHAR terminalID[200] = { 0 };
  GetTerminalID(terminalID);
  strncpy(data.terminalID, CT2A(terminalID, CP_UTF8), _countof(data.terminalID));
  strncpy(data.firmwareVersion, deviceInfo.firmwareVersion, _countof(data.firmwareVersion));
  data.offline = WorkingParameters::GetInstance()->IsOffline() ? 1 : 0;
  data.selfTestState = deviceInfo.selfTestState;
  ASSERT(sizeof(data.debugState) == sizeof(deviceInfo.debugState));
  memcpy(data.debugState, deviceInfo.debugState, sizeof(data.debugState));
  if (!DeviceProxy::GetInstance()->GetMacAddr(data.mac, _countof(data.mac)))
  {
	  memset(data.mac, 0, _countof(data.mac));
  }

  WriteRecord("SDG ", &data, sizeof(data));
}

void DebugLogger::Upgrade(LPCTSTR toVersion, int result) {
  #pragma pack(push)
  #pragma pack(1)
  struct RecordData {
    int             result;
    char            toVersion[24];    
  };
  #pragma pack(pop)

  // 根据可变配置文件中的配置项决定是否上传数据 lux
  VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();
  int uploadLevel = config->GetIntParameter(_T("UploadLevel"), 1);
  if (uploadLevel == 0)
  {
	  return;
  }
  if (logFile_.m_hFile == INVALID_HANDLE_VALUE) {
	  return;
  }

  if (IsUploadDetail(uploadLevel, result))
  {
	  RecordData data;
	  ZeroMemory(&data, sizeof(data));
	  strncpy(data.toVersion, CT2A(toVersion, CP_UTF8), _countof(data.toVersion));
	  data.result = result;

	  WriteRecord("UPG ", &data, sizeof(data));
  }
  else
  {
	  WriteRecord("UPG ", &result, sizeof(result));
  }
}

void DebugLogger::CalibrateInfrared(int result, const IRState states[6]) {
  #pragma pack(push)
  #pragma pack(1)
  struct RecordData {
    int             result;
    IRState         states[6];
  };
  #pragma pack(pop)

  // 根据可变配置文件中的配置项决定是否上传数据 lux
  VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();
  int uploadLevel = config->GetIntParameter(_T("UploadLevel"), 1);
  if (uploadLevel == 0)
  {
	  return;
  }
  if (logFile_.m_hFile == INVALID_HANDLE_VALUE) {
	  return;
  }

  if (IsUploadDetail(uploadLevel, result))
  {
	  RecordData data;
	  ZeroMemory(&data, sizeof(data));
	  data.result = result;
	  memcpy(data.states, states, sizeof(data.states));

	  WriteRecord("IRC ", &data, sizeof(data));
  }
  else
  {
	  WriteRecord("IRC ", &result, sizeof(result));
  }
  
}

void DebugLogger::DetectMagneticSignal(int result, int object, const MagneticSignal *signals) {
  #pragma pack(push)
  #pragma pack(1)
  struct RecordData {
    int             result;
    int             object; // 0: paper; 1: rmb 100 v05
  };
  #pragma pack(pop)

  // 根据可变配置文件中的配置项决定是否上传数据 lux
  VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();
  int uploadLevel = config->GetIntParameter(_T("UploadLevel"), 1);
  if (uploadLevel == 0)
  {
	  return;
  }
  if (logFile_.m_hFile == INVALID_HANDLE_VALUE) {
	  return;
  }

  if (IsUploadDetail(uploadLevel, result))
  {
	  int dataLength = sizeof(RecordData);

	  for (int i = 0; i < MAGNETIC_HEADER_COUNT; i++) {
		  dataLength += 2; // tag
		  dataLength += 4; // sample count
		  dataLength += signals[i].length * 4; // codes + values
	  }
	  RecordData *data = (RecordData *)malloc(dataLength);
	  data->result = result;
	  data->object = object;
	  char *p = (char *)(data + 1);
	  char *tags[] = {
		  "HD", "BM", "LM", "RM", "LS", "RS",
	  };
	  for (int i = 0; i < MAGNETIC_HEADER_COUNT; i++) {
		  // tag
		  strncpy(p, tags[i], 2);
		  p += 2;
		  // sample count
		  *(int *)p = signals[i].length;
		  p += 4;
		  // codes
		  memcpy(p, signals[i].codes, signals[i].length * 2);
		  p += signals[i].length * 2;
		  // values
		  memcpy(p, signals[i].values, signals[i].length * 2);
		  p += signals[i].length * 2;
	  }

	  WriteRecord("MHD ", data, dataLength);

	  free(data);
  }
  else
  {
	  RecordData data;
	  ZeroMemory(&data, sizeof(data));
	  data.result = result;
	  data.object = object;

	  WriteRecord("MHD ", &data, sizeof(data));
  }

}

void DebugLogger::CalibrateCIS(int step, const CISErrorInfo *error, const CISParameter *parameters, const CISImageInfo *images, int imageCount) {
  #pragma pack(push)
  #pragma pack(1)
  struct RecordData {
    int             step; // 0: black; 1: white; 2: correction
    CISErrorInfo    error;
    CISParameter    parameters[CIS_COUNT];
  };

  struct BasicData
  {
	  int			step;
	  CISErrorInfo	error;
  };
  #pragma pack(pop)

  // 根据可变配置文件中的配置项决定是否上传数据 lux
  VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();
  int uploadLevel = config->GetIntParameter(_T("UploadLevel"), 1);
  if (uploadLevel == 0)
  {
	  return;
  }
  if (logFile_.m_hFile == INVALID_HANDLE_VALUE) {
	  return;
  }

  int result = 0;
  if (error == NULL)
  {
	  result = 0;
  }
  else
  {
	  result = error->result;
  }

  if (IsUploadDetail(uploadLevel, result))
  {
	  int dataLength = sizeof(RecordData);
	  for (int i = 0; i < imageCount; i++) {
		  dataLength += sizeof(images[0].tag); // tag
		  dataLength += 4;
		  dataLength += images[i].image->GetWidth() * images[i].image->GetHeight();
	  }
	  RecordData *data = (RecordData *)malloc(dataLength);
	  memset(data, 0, sizeof(RecordData));
	  if (error != NULL) {
		  memcpy(&data->error, error, sizeof(CISErrorInfo));
	  }
	  data->step = step;
	  memcpy(data->parameters, parameters, CIS_COUNT * sizeof(CISParameter));
	  char *p = (char *)(data + 1);
	  for (int i = 0; i < imageCount; i++) {
		  int imageSize = images[i].image->GetWidth() * images[i].image->GetHeight();
		  memcpy(p, images[i].tag, sizeof(images[i].tag));
		  short *size = (short *)(p + sizeof(images[i].tag));
		  size[0] = (short)images[i].image->GetWidth();
		  size[1] = (short)images[i].image->GetHeight();
		  memcpy(size + 2, images[i].image->GetBits(), imageSize);
		  p += sizeof(images[i].tag) + 4 + imageSize;
	  }

	  WriteRecord("CIS ", data, dataLength);

	  free(data);
  }
  else
  {
	  BasicData data;
	  ZeroMemory(&data, sizeof(data));
	  data.step = step;
	  if (error != NULL)
	  {
		  memcpy(&data.error, error, sizeof(CISErrorInfo));
	  }

	  WriteRecord("CIS ", &data, sizeof(data));
  }

}

bool DebugLogger::SaveCash(Cash *cash) {
  if (!cash->Valid(Cash::VALID_IMAGE)) {
    return false;
  }

  Bitmap *topImage = cash->GetTopImage();
  ASSERT(topImage != NULL);
  Bitmap *bottomImage = cash->GetBottomImage();
  ASSERT(bottomImage != NULL);

  for (int i = 0; i < 2; i++) {
    Bitmap *bitmap = (i == 0 ? topImage : bottomImage);
    GUID iid;
    CoCreateGuid(&iid);
    LPOLESTR fileName = NULL;
    StringFromCLSID(iid, &fileName);
    TCHAR filePath[MAX_PATH] = _T("e:\\cis\\");
    _tcscat(filePath, fileName);
    _tcscat(filePath, _T(".bmp"));

    bitmap->SaveToFile(filePath);
  }

  //FILE *fp;
  //png_structp pngPtr;
  //png_infop infoPtr;
  //png_colorp palette;

  ///* Open the file */
  //fp = fopen(CW2A(filePath), "wb");
  //if (fp == NULL) {
  //  return false;
  //}

  ///* Create and initialize the png_struct with the desired error handler
  //* functions.  If you want to use the default stderr and longjump method,
  //* you can supply NULL for the last three parameters.  We also check that
  //* the library version is compatible with the one used at compile time,
  //* in case we are using dynamically linked libraries.  REQUIRED.
  //*/
  //pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
  //  png_voidp user_error_ptr, user_error_fn, user_warning_fn);

  //if (pngPtr == NULL) {
  //  fclose(fp);
  //  return false;
  //}

  ///* Allocate/initialize the image information data.  REQUIRED */
  //infoPtr = png_create_info_struct(pngPtr);
  //if (infoPtr == NULL) {
  //  fclose(fp);
  //  png_destroy_write_struct(&pngPtr,  NULL);
  //  return false;
  //}

  ///* Set error handling.  REQUIRED if you aren't supplying your own
  //* error handling functions in the png_create_write_struct() call.
  //*/
  //if (setjmp(png_jmpbuf(pngPtr))) {
  //  /* If we get here, we had a problem writing the file */
  //  fclose(fp);
  //  png_destroy_write_struct(&pngPtr, &infoPtr);
  //  return false;
  //}

  ///* Set up the output control if you are using standard C streams */
  //png_init_io(pngPtr, fp);


  ///* This is the hard way */

  ///* Set the image information here.  Width and height are up to 2^31,
  //* bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
  //* the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
  //* PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
  //* or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
  //* PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
  //* currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
  //*/
  //png_set_IHDR(pngPtr, infoPtr, 
  //    topImage->GetWidth(), 2 * topImage->GetHeight(), 
  //    8, PNG_COLOR_TYPE_GRAY,
  //    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  //

  ///* Write comments into the image */
  //{
  //  png_text textPtr;
  //  
  //  char key[]="Description";
  //  char text[]="<long text>";
  //  textPtr.key = key;
  //  textPtr.text = text;
  //  textPtr.compression = PNG_TEXT_COMPRESSION_zTXt;
  //  textPtr.itxt_length = 0;
  //  textPtr.lang = NULL;
  //  textPtr.lang_key = NULL;

  //  png_set_text(write_ptr, write_info_ptr, textPtr, 1);
  //}

  ///* Other optional chunks like cHRM, bKGD, tRNS, tIME, oFFs, pHYs */

  ///* Note that if sRGB is present the gAMA and cHRM chunks must be ignored
  //* on read and, if your application chooses to write them, they must
  //* be written in accordance with the sRGB profile
  //*/

  ///* Write the file header information.  REQUIRED */
  //png_write_info(pngPtr, infoPtr);
  //
  ///* The easiest way to write the image (you may have a different memory
  //* layout, however, so choose what fits your needs best).  You need to
  //* use the first method if you aren't handling interlacing yourself.
  //*/
  //png_uint_32 k, height, width;

  ///* In this example, "image" is a one-dimensional array of bytes */
  //png_byte image[height*width*bytes_per_pixel];

  //png_bytep row_pointers[height];

  //if (height > PNG_UINT_32_MAX/(sizeof (png_bytep)))
  //  png_error (pngPtr, "Image is too tall to process in memory");

  ///* Set up pointers into your "image" byte array */
  //for (k = 0; k < height; k++)
  //  row_pointers[k] = image + k*width*bytes_per_pixel;

  //png_write_image(pngPtr, row_pointers);

  ///* The other way to write the image - deal with interlacing */

  ///* You can write optional chunks like tEXt, zTXt, and tIME at the end
  //* as well.  Shouldn't be necessary in 1.2.0 and up as all the public
  //* chunks are supported and you can use png_set_unknown_chunks() to
  //* register unknown chunks into the info structure to be written out.
  //*/

  ///* It is REQUIRED to call this to finish writing the rest of the file */
  //png_write_end(pngPtr, infoPtr);
  //
  ///* Whenever you use png_free() it is a good idea to set the pointer to
  //* NULL in case your application inadvertently tries to png_free() it
  //* again.  When png_free() sees a NULL it returns without action, thus
  //* avoiding the double-free security problem.
  //*/

  ///* Clean up after the write, and free any memory allocated */
  //png_destroy_write_struct(&pngPtr, &infoPtr);

  ///* Close the file */
  //fclose(fp);

  return true;
}

void DebugLogger::EndDebug() 
{
	// 根据可变配置文件中的配置项决定是否上传数据 lux
	VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();
	int uploadLevel = config->GetIntParameter(_T("UploadLevel"), 1);
	if (uploadLevel == 0)
	{
		return;
	}
	if (logFile_.m_hFile == INVALID_HANDLE_VALUE) {
		return;
	}
	char noData = 0;

	WriteRecord("END ", &noData, 1);
}