#include "stdafx.h"
#include "working_parameters.h"
#include <list>
#include "error.h"
#include "ftp_client.h"
#include "ftp_client_inl.h"
#include "common.h"
#include "application.h"
#include "md5.h"
#include "Computer_Information.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static unsigned char CodeBook[256] = {
	0x48, 0x1f, 0xb4, 0xf1, 0xe8, 0x94, 0xb2, 0xf2, 0xd9, 0xb8, 0x17, 0x46, 0xa9, 0x00, 0x3a, 0x56, 
	0x7a, 0x30, 0x66, 0xc9, 0xc1, 0xdd, 0x01, 0x75, 0x49, 0x14, 0x0d, 0xa8, 0xc7, 0x8e, 0x5d, 0x7b, 
	0x7d, 0xa7, 0xc4, 0x16, 0xbe, 0x61, 0x91, 0x31, 0xa0, 0xf0, 0x35, 0xfe, 0xc0, 0x60, 0x6e, 0x2a, 
	0xf6, 0x24, 0x2c, 0xce, 0x6a, 0xc8, 0x4a, 0xad, 0x52, 0x8f, 0xe1, 0x2b, 0x6b, 0x25, 0x41, 0xf7, 
	0xa3, 0x1d, 0xd6, 0x0e, 0xfa, 0x81, 0x62, 0xc6, 0x9b, 0x2f, 0x28, 0x34, 0xbc, 0x47, 0xba, 0xb9, 
	0xeb, 0xcf, 0x9d, 0x32, 0xfb, 0x58, 0x07, 0x7c, 0x1a, 0xdc, 0x69, 0x4d, 0xcd, 0x9f, 0x86, 0xe2, 
	0x38, 0x98, 0x9e, 0xfd, 0x88, 0x1c, 0xa6, 0x12, 0xed, 0x9c, 0x20, 0xb5, 0xdf, 0x99, 0xb6, 0x54, 
	0x13, 0xf8, 0x80, 0xaa, 0x21, 0x5a, 0xd4, 0x55, 0x44, 0x06, 0xe0, 0x76, 0x9a, 0x05, 0xf5, 0x5e, 
	0xda, 0xec, 0xbd, 0xd2, 0xa5, 0xde, 0x40, 0xff, 0xfc, 0x42, 0x39, 0x3b, 0x71, 0x6d, 0x4f, 0x10, 
	0x78, 0x92, 0xf9, 0xd3, 0xd1, 0x96, 0x1b, 0x90, 0x7f, 0x5c, 0x3e, 0xb1, 0x0a, 0x45, 0x2e, 0x19, 
	0xa1, 0x0c, 0x6c, 0xe6, 0x26, 0xca, 0x85, 0x4b, 0x23, 0x4e, 0xcb, 0x8a, 0xf4, 0x7e, 0x15, 0x08, 
	0x3f, 0x8c, 0x0f, 0xb0, 0x33, 0x57, 0x04, 0xc3, 0xaf, 0xa2, 0x4c, 0x3d, 0x93, 0xc2, 0x09, 0x29, 
	0x84, 0x36, 0xbf, 0x79, 0xee, 0x11, 0x72, 0x65, 0xef, 0xe4, 0x8b, 0x22, 0xe9, 0x3c, 0x18, 0xb7, 
	0xcc, 0x5f, 0x5b, 0x1e, 0x27, 0xe5, 0x87, 0x50, 0x6f, 0x97, 0x43, 0x51, 0xdb, 0xe3, 0xb3, 0xab, 
	0x73, 0xa4, 0xd8, 0xd0, 0x64, 0xbb, 0x02, 0xac, 0x77, 0x37, 0x8d, 0xea, 0xe7, 0x68, 0x0b, 0x70, 
	0x89, 0x74, 0x59, 0xc5, 0x95, 0xd7, 0xf3, 0xd5, 0xae, 0x67, 0x63, 0x82, 0x2d, 0x53, 0x03, 0x83, 
};

inline int Hex(TCHAR ch) {
  if (ch >= _T('0') && ch <= _T('9')) {
    return ch - _T('0');
  } else if (ch >= _T('a') && ch <= _T('f')) {
    return ch - _T('a') + 10;
  } else if (ch >= _T('A') && ch <= _T('F')) {
    return ch - _T('A') + 10;
  }
  return 0;
}

CString DecodePassword(LPCTSTR crypted) {
  CString decrypted;
  LPCTSTR p = crypted;
  while (*p != _T('\0') && *(p + 1) != _T('\0')) {
    int code = (Hex(*p) << 4) | Hex(*(p + 1));
    decrypted.AppendChar(CodeBook[code]);
    p += 2;
  }
  return decrypted;
}

IMPLEMENT_SINGLETON(WorkingParameters)

WorkingParameters::WorkingParameters() : appUpdated_(false), currentModelId_(-1), currentPermission_(3) {  
}

WorkingParameters::~WorkingParameters() {  
}

bool WorkingParameters::IsFTPInfoBlank()
{
	TCHAR configFilePath[MAX_PATH];
	GetModuleFileName(NULL, configFilePath, _countof(configFilePath));
	PathRemoveFileSpec(configFilePath);
	PathAddBackslash(configFilePath);
	_tcscat(configFilePath, _T("FTPCONFIG.ini"));

	TCHAR userName[128];
	GetPrivateProfileString(TEXT("Config"), TEXT("User"), NULL, userName, _countof(userName), configFilePath);
	TCHAR password[128];
	GetPrivateProfileString(TEXT("Config"), TEXT("Password"), NULL, password, _countof(password), configFilePath);

	if ((wcscmp(userName, _T("")) == 0) || (wcscmp(password, _T("")) == 0))
	{
		return true;
	}
	else
	{
		return false;
	}
}

CString CodePassword(LPCTSTR decrypted)
{
	CString crypted;
	char cDecrypted[256] = { 0 };
	strcpy(cDecrypted, CT2A(decrypted));
	char *p = cDecrypted;
	while (*p != '\0')
	{
		for (int i = 0; i < 256; i++)
		{
			if (*p == CodeBook[i])
			{
				TCHAR code[3] = {0};
				wsprintf((LPWSTR)code, _T("%02x"), i);
				crypted.AppendChar(code[0]);
				crypted.AppendChar(code[1]);
				break;
			}
		}

		p++;
	}

	return crypted;
}

bool WorkingParameters::SetFTPInfo(LPCTSTR userName, LPCTSTR password)
{
	TCHAR configFilePath[MAX_PATH];
	GetModuleFileName(NULL, configFilePath, _countof(configFilePath));
	PathRemoveFileSpec(configFilePath);
	PathAddBackslash(configFilePath);
	_tcscat(configFilePath, _T("FTPCONFIG.ini"));

	TCHAR serverIP[128];
	GetPrivateProfileString(TEXT("Config"), TEXT("ServerIP"), NULL, serverIP, _countof(serverIP), configFilePath);
	int serverPort = GetPrivateProfileInt(TEXT("Config"), TEXT("ServerPort"), 21, configFilePath);

	FtpClient client;
	if (!client.Connect(serverIP, serverPort, userName, DecodePassword(password))) 
	{
		return false;
	}
	else
	{
		WritePrivateProfileString(TEXT("Config"), TEXT("User"), CodePassword(userName), configFilePath);
		WritePrivateProfileString(TEXT("Config"), TEXT("Password"), password, configFilePath);

		WritePrivateProfileString(TEXT("Upload"), TEXT("User"), CodePassword(userName), configFilePath);
		WritePrivateProfileString(TEXT("Upload"), TEXT("Password"), password, configFilePath);

		return true;
	}
}

bool WorkingParameters::LoadFromServer() {
  LastErrorHolder errorHolder;

  TCHAR configFilePath[MAX_PATH];
  GetModuleFileName(NULL, configFilePath, _countof(configFilePath));
  PathRemoveFileSpec(configFilePath);
  PathAddBackslash(configFilePath);
  _tcscat(configFilePath, _T("FTPCONFIG.ini"));

  TCHAR serverIP[128];
  GetPrivateProfileString(TEXT("Config"), TEXT("ServerIP"), NULL, serverIP, _countof(serverIP), configFilePath);
  int serverPort = GetPrivateProfileInt(TEXT("Config"), TEXT("ServerPort"), 21, configFilePath);
  TCHAR userName[128];
  GetPrivateProfileString(TEXT("Config"), TEXT("User"), NULL, userName, _countof(userName), configFilePath);
  TCHAR password[128];
  GetPrivateProfileString(TEXT("Config"), TEXT("Password"), NULL, password, _countof(password), configFilePath);
  TCHAR remotePath[MAX_PATH];
  GetPrivateProfileString(TEXT("Config"), TEXT("RemotePath"), NULL, remotePath, _countof(remotePath), configFilePath);

  FtpClient client;
  if (!client.Connect(serverIP, serverPort, DecodePassword(userName), DecodePassword(password))) {
	errorHolder.SaveLastError();
    return false;
  }

  TCHAR appConfigPath[MAX_PATH];
  GetAppDataPath(appConfigPath);
  PathAddBackslash(appConfigPath);
  _tcscat(appConfigPath, _T("config"));
  if (!CreateDirectoryRecusively(appConfigPath)) {
    errorHolder.SaveLastError();
    return false;
  }
  PathAddBackslash(appConfigPath);

  TCHAR remoteManifestFilePath[MAX_PATH];
  _tcscpy(remoteManifestFilePath, remotePath);
  _tcscat(remoteManifestFilePath, _T("manifest.xml"));
  TCHAR localManifestFilePath[MAX_PATH];
  _tcscpy(localManifestFilePath, appConfigPath);
  _tcscat(localManifestFilePath, _T("manifest.xml"));
  if (!client.GetFile(remoteManifestFilePath, localManifestFilePath)) {
    errorHolder.SaveLastError();
    return false;
  }

  if (!LoadXML()) {
    errorHolder.SaveLastError();
    return false;
  }

  if (!theApp.IsSkipUpdate()) {
    ParameterBlock verBlock = SelectBlock(_T("Application\\LastVersion"));
    if (!verBlock.IsNull()) {
      CString lastVersion = verBlock.GetStringParameter(_T("version"), NULL);
	  // 当且仅当服务器版本号高于当前版本号时才升级
	  int lastV1, lastV2, lastV3, lastV4;
	  lastV1 = lastV2 = lastV3 = lastV4 = 999;
	  swscanf(lastVersion, _T("%d.%d.%d.%d"), &lastV1, &lastV2, &lastV3, &lastV4);
	  CString curVersion = AfxGetApp()->GetProfileString(_T(""), _T("Version"));
	  int curV1, curV2, curV3, curV4;
	  curV1 = curV2 = curV3 = curV4 = 0;
	  swscanf(curVersion, _T("%d.%d.%d.%d"), &curV1, &curV2, &curV3, &curV4);
	  bool skip = false;
	  if ((lastV1 * 1000 + lastV2 * 100 + lastV3 * 10 + lastV4) > (curV1 * 1000 + curV2 * 100 + curV3 * 10 + curV4))
	  {
      //if (lastVersion != AfxGetApp()->GetProfileString(_T(""), _T("Version"))) {
        CString remotePath = verBlock.GetStringParameter(_T("path"), NULL);
        TCHAR updateFilePath[MAX_PATH];
        _tcscpy(updateFilePath, appConfigPath);
        _tcscat(updateFilePath, _T("app_update.dat"));
        if (!client.GetFile(remotePath, updateFilePath)) {
          errorHolder.SaveLastError();
          return false;
        }
        appUpdated_ = true;
        appUpdateFilePath_ = updateFilePath;
        lastAppVersion_ = lastVersion;
      }
    }
  }

  // update fireware files from server.
  ParameterBlock deviceBlock = SelectBlock(_T("Device"));
  if (deviceBlock.IsNull()) {
    errorHolder.SetError(ERROR_SERVER_CONFIG);
    return false;
  }

  ParameterBlock modelBlock = deviceBlock.GetFirstChildBlock();
  if (modelBlock.IsNull()) {
    errorHolder.SetError(ERROR_SERVER_CONFIG);
    return false;
  }
  for (; !modelBlock.IsNull(); modelBlock = modelBlock.GetNextBlock()) {
    if (modelBlock.GetName() == _T("Model")) {
      int id = modelBlock.GetIntParameter(_T("id"), -1);
      if (id == -1) {
        errorHolder.SetError(ERROR_SERVER_CONFIG);
        return false;
      }
      CString localFirewarePath(appConfigPath);      
      localFirewarePath.AppendFormat(_T("firmware_%d.dat"), id);
      ParameterBlock firmwareBlock = modelBlock.SelectBlock(_T("Firmware"));
      if (firmwareBlock.IsNull()) {
        errorHolder.SetError(ERROR_SERVER_CONFIG);
        return false;
      }
      bool getFile = true;
      CString remoteCheckSum = firmwareBlock.GetStringParameter(_T("checkSum"), NULL);
      CString localCheckSum;
      if (!remoteCheckSum.IsEmpty() && MD5::CalcFile(localFirewarePath, &localCheckSum)) {
        getFile = (remoteCheckSum != localCheckSum);
      }
      if (getFile) {
        CString path = firmwareBlock.GetStringParameter(_T("path"), NULL);
        if (!client.GetFile(path, localFirewarePath)) {
          errorHolder.SaveLastError();
          return false;
        }
      }
    }
  }

  return true;
}

bool WorkingParameters::LoadConfigFromServer()
{
	LastErrorHolder errorHolder;

	TCHAR configFilePath[MAX_PATH];
	GetModuleFileName(NULL, configFilePath, _countof(configFilePath));
	PathRemoveFileSpec(configFilePath);
	PathAddBackslash(configFilePath);
	_tcscat(configFilePath, _T("FTPCONFIG.ini"));

	TCHAR serverIP[128];
	GetPrivateProfileString(TEXT("Config"), TEXT("ServerIP"), NULL, serverIP, _countof(serverIP), configFilePath);
	int serverPort = GetPrivateProfileInt(TEXT("Config"), TEXT("ServerPort"), 21, configFilePath);
	TCHAR userName[128];
	GetPrivateProfileString(TEXT("Config"), TEXT("User"), NULL, userName, _countof(userName), configFilePath);
	TCHAR password[128];
	GetPrivateProfileString(TEXT("Config"), TEXT("Password"), NULL, password, _countof(password), configFilePath);

	FtpClient client;
	if (!client.Connect(serverIP, serverPort, DecodePassword(userName), DecodePassword(password))) 
	{
		errorHolder.SaveLastError();
		return false;
	}

	TCHAR appConfigPath[MAX_PATH];
	GetAppDataPath(appConfigPath);
	PathAddBackslash(appConfigPath);
	_tcscat(appConfigPath, _T("config"));
	if (!CreateDirectoryRecusively(appConfigPath)) 
	{
		errorHolder.SaveLastError();
		return false;
	}
	PathAddBackslash(appConfigPath);

	TCHAR remoteTerminalPath[MAX_PATH];
	GetPrivateProfileString(TEXT("Config"), TEXT("RemoteTerminalPath"), NULL, remoteTerminalPath, _countof(remoteTerminalPath), configFilePath);

	// 获取机器识别码 lux
	TCHAR id[200] = { 0 };
	GetTerminalID(id);

	// 将机器识别码写入配置文件中，方便查询
	WritePrivateProfileString(TEXT("Config"), TEXT("TerminalID"), id, configFilePath);

	// 根据设备唯一识别码下载相应可变配置文件 lux
	// 为了防止因服务器下载失败而清除本地配置文件，以临时文件的形式先保存，下载成功以后再覆盖原文件
	TCHAR tempFilePath[MAX_PATH];
	GetTempPath(_countof(tempFilePath), tempFilePath);
	GetTempFileName(tempFilePath, _T("~"), 0, tempFilePath);
	TCHAR remoteConfigFilePath[MAX_PATH];
	_tcscpy(remoteConfigFilePath, remoteTerminalPath);
	_tcscat(remoteConfigFilePath, id);
	_tcscat(remoteConfigFilePath, _T(".ini"));
	if (!client.GetFile(remoteConfigFilePath, tempFilePath))
	{
		errorHolder.SaveLastError();
		return false;
	}

	TCHAR localConfigFilePath[MAX_PATH];
	_tcscpy(localConfigFilePath, appConfigPath);
	_tcscat(localConfigFilePath, _T("model.ini"));
	
	BOOL bMoveSuccess = MoveFileEx(tempFilePath, localConfigFilePath, MOVEFILE_COPY_ALLOWED + MOVEFILE_REPLACE_EXISTING);
	if (bMoveSuccess)
	{
		DeleteFile(tempFilePath);
	}
	else
	{
		DeleteFile(tempFilePath);
		return false;
	}

	if (!LoadConfig())
	{
		errorHolder.SaveLastError();
		return false;
	}

	return true;
}

bool WorkingParameters::LoadXML() {
  LastErrorHolder errorHolder;

  xmlDoc_.Release();

  HRESULT hr = xmlDoc_.CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER);
  if (FAILED(hr)) {
    errorHolder.SetError(ERROR_XML_COMPONENT_DAMAGE);
    return false;
  }

  TCHAR configFilePath[MAX_PATH];
  GetAppDataPath(configFilePath);
  PathAddBackslash(configFilePath);
  _tcscat(configFilePath, _T("config\\manifest.xml"));
  CComVariant varPath(configFilePath);
  VARIANT_BOOL successful;
  hr = xmlDoc_->load(varPath, &successful);
  if (FAILED(hr)) {
    errorHolder.SetError(ERROR_CONFIG_UNAVAILABLE);
    return false;
  }

  IXMLDOMElement *root = NULL;
  xmlDoc_->get_documentElement(&root);
  if (root == NULL) {
    errorHolder.SetError(ERROR_SERVER_CONFIG);
    return false;
  }
  
  Reset(root);

  ParameterBlock deviceBlock = SelectBlock(_T("Device"));
  if (deviceBlock.IsNull()) {
    errorHolder.SetError(ERROR_SERVER_CONFIG);
    return false;
  }

  currentModelParameters_ = deviceBlock.GetFirstChildBlock();
  if (currentModelParameters_.IsNull()) {
    errorHolder.SetError(ERROR_SERVER_CONFIG);
    return false;
  }
  currentModelId_ = currentModelParameters_.GetIntParameter(_T("id"), -1);

  return true;
}

bool WorkingParameters::LoadConfig()
{
	LastErrorHolder errorHolder;

	TCHAR configFilePath[MAX_PATH];
	GetAppDataPath(configFilePath);
	PathAddBackslash(configFilePath);
	_tcscat(configFilePath, _T("config\\model.ini"));
	currentConfigParameters_ = VariableConfigBlock(configFilePath);
	if (currentConfigParameters_.IsNull())
	{
		errorHolder.SetError(ERROR_CONFIG_UNAVAILABLE);
		return false;
	}

	return true;
}

bool WorkingParameters::SetCurrentModel(int id) {
  ParameterBlock device = SelectBlock(_T("Device"));
  if (device.IsNull()) {
    return false;
  }
  for (ParameterBlock block = device.GetFirstChildBlock(); !block.IsNull(); block = block.GetNextBlock()) {
    if (block.GetIntParameter(_T("id"), -1) == id) {
      currentModelId_ = id;
      currentModelParameters_ = block;
      return true;
    }
  }
  return false;
}

ParameterBlock *WorkingParameters::GetCurrentModelParameters() {
  return &currentModelParameters_;
}

VariableConfigBlock *WorkingParameters::GetCurrentConfigParameters()
{
	return &currentConfigParameters_;
}

int WorkingParameters::Login(LPCTSTR userName, LPCTSTR password) {
  ParameterBlock usersBlock = SelectBlock(_T("Users"));
  ParameterBlock user = usersBlock.GetFirstChildBlock();
  for (; !user.IsNull(); user = user.GetNextBlock()) {
    if (user.GetName() == _T("User")) {
      if (user.GetStringParameter(_T("name"), NULL) == userName) {
        if (user.GetStringParameter(_T("password"), NULL) == MD5::CalcString(password)) {
			userName_.Format(L"%s", userName);
			currentPermission_ = user.GetIntParameter(_T("permission"), 3);
			return currentPermission_;
        }
      }
    }
  }
  return -1;
}

// ParameterBlock
const ParameterBlock ParameterBlock::Null;

static IXMLDOMElement *FindFirstElement(IXMLDOMElement *parent, LPCTSTR name, int length) {
  if (parent == NULL) {
    return NULL;
  }
  if (length == -1) {
    length = _tcslen(name);
  }
  CComPtr<IXMLDOMNode> child;
  parent->get_firstChild(&child);
  while (child != NULL) {
    CComQIPtr<IXMLDOMElement> e(child);
    if (e != NULL) {
      CComBSTR tagName;
      e->get_tagName(&tagName);
      if (tagName.Length() == length && wcsncmp(tagName, name, length) == 0) {
        return e.Detach();
      }
    }
    CComPtr<IXMLDOMNode> next;
    child->get_nextSibling(&next);
    child = next;
  }
  return NULL;
}

ParameterBlock::ParameterBlock() : e_(NULL) {
}

ParameterBlock::ParameterBlock(IXMLDOMElement *e) : e_(e) {
}

ParameterBlock::ParameterBlock(const ParameterBlock &other) : e_(other.e_) {
  if (e_ != NULL) {
    e_->AddRef();
  }
}

ParameterBlock::~ParameterBlock() {
  if (e_ != NULL) {
    e_->Release();
  }
}

void ParameterBlock::Reset(IXMLDOMElement *e) {
  if (e_ == e) {
    return;
  }
  if (e_ != NULL) {
    e_->Release();
  }
  e_ = e;
}

ParameterBlock &ParameterBlock::operator=(const ParameterBlock &other) {
  IXMLDOMElement *e = other.e_;

  if (e_ == e) {
    return *this;
  }
  if (e != NULL) {
    e->AddRef();
  }
  if (e_ != NULL) {
    e_->Release();
  }
  e_ = e;

  return *this;
}

CString ParameterBlock::GetName() const {
  if (e_ == NULL) {
    return _T("");
  }
  CComBSTR tagName;
  e_->get_tagName(&tagName);
  return CString(tagName);
}

int ParameterBlock::GetIntParameter(LPCTSTR name, int defaultValue) {
  if (e_ == NULL) {
    return defaultValue;
  }
  CComVariant value;
  e_->getAttribute(CComBSTR(name), &value);
  if (FAILED(value.ChangeType(VT_INT))) {
    return defaultValue;
  }
  return value.intVal;
}

double ParameterBlock::GetFloatParameter(LPCTSTR name, double defaultValue) {
  if (e_ == NULL) {
    return defaultValue;
  }
  CComVariant value;
  e_->getAttribute(CComBSTR(name), &value);
  if (FAILED(value.ChangeType(VT_R8))) {
    return defaultValue;
  }
  return value.dblVal;
}

CString ParameterBlock::GetStringParameter(LPCTSTR name, LPCTSTR defaultValue) {
  if (e_ == NULL) {
    return defaultValue;
  }
  CComVariant value;
  e_->getAttribute(CComBSTR(name), &value);
  if (FAILED(value.ChangeType(VT_BSTR))) {
    return defaultValue;
  }
  return CString(value.bstrVal);
}

ParameterBlock ParameterBlock::SelectBlock(LPCTSTR path) {
  IXMLDOMElement *result = e_;
  LPCTSTR start = path, end = path;
  while (result != NULL && (end = _tcschr(start, _T('\\'))) != NULL) {
    result = FindFirstElement(result, start, end - start);    
    start = end + 1;
  }
  if (*start != _T('\0') && result != NULL) {
    result = FindFirstElement(result, start, -1);
  }
  return ParameterBlock(result);
}

ParameterBlock ParameterBlock::GetFirstChildBlock() {
  if (e_ == NULL) {
    return ParameterBlock::Null;
  }
  CComPtr<IXMLDOMNode> node;
  e_->get_firstChild(&node);
  if (node == NULL) {
    return ParameterBlock::Null;
  }
  IXMLDOMElement *child = NULL;
  node->QueryInterface(IID_IXMLDOMElement, (void **)&child);
  return ParameterBlock(child);
}

ParameterBlock ParameterBlock::GetNextBlock() {
  if (e_ == NULL) {
    return ParameterBlock::Null;
  }
  CComPtr<IXMLDOMNode> node;
  e_->get_nextSibling(&node);
  if (node == NULL) {
    return ParameterBlock::Null;
  }
  IXMLDOMElement *child = NULL;
  node->QueryInterface(IID_IXMLDOMElement, (void **)&child);
  return ParameterBlock(child);
}

bool ParameterBlock::IsNull() const {
  return e_ == NULL;
}

VariableConfigBlock::VariableConfigBlock() : configPath_(TEXT(""))
{}

VariableConfigBlock::~VariableConfigBlock() 
{}

VariableConfigBlock::VariableConfigBlock(LPCTSTR path) : configPath_(path)
{}

VariableConfigBlock::VariableConfigBlock(const VariableConfigBlock &other) : configPath_(other.configPath_)
{}

VariableConfigBlock &VariableConfigBlock::operator=(const VariableConfigBlock &other)
{
	if (&other != this)
	{
		configPath_.Format(TEXT("%s"), other.configPath_);
	}

	return *this;
}

CString VariableConfigBlock::GetCurrentMode()
{
	if (_tcscmp(configPath_, TEXT("")) == 0)
	{
		return CString("");
	}

	TCHAR szCurConfig[128];
	GetPrivateProfileString(TEXT("Mode"), TEXT("CurrentMode"), TEXT(""), szCurConfig, _countof(szCurConfig), configPath_);
	if (_tcscmp(szCurConfig, TEXT("")) == 0)
	{
		return CString("");
	}
	else
	{
		return CString(szCurConfig);
	}
}

int VariableConfigBlock::GetIntParameter(LPCTSTR name, int defaultValue)
{
	if (_tcscmp(configPath_, TEXT("")) == 0)
	{
		return defaultValue;
	}

	// 首先读取当前配置
	CString strCurMode = GetCurrentMode();
	if (_tcscmp(strCurMode, TEXT("")) == 0)
	{
		return defaultValue;
	}

	// 再在当前配置下读取数据
	return GetPrivateProfileInt(strCurMode, name, defaultValue, configPath_);
}

void VariableConfigBlock::SetIntParemeter(LPCTSTR name,int choiceValue)
{
	if (_tcscmp(configPath_, TEXT("")) == 0)
	{
		return;
	}
	CString strCurMode = GetCurrentMode();
	if (_tcscmp(strCurMode,TEXT(""))==0)
	{
		return;
	}
    //只需将整型的值变为字符型
	CString strTemp;
	strTemp.Format(L"%d", choiceValue);
	::WritePrivateProfileString(strCurMode, name, strTemp, configPath_);
}

CString VariableConfigBlock::GetStringParameter(LPCTSTR name, LPCTSTR defaultValue)
{
	if (_tcscmp(configPath_, TEXT("")) == 0)
	{
		return defaultValue;
	}

	// 首先读取当前配置
	CString strCurMode = GetCurrentMode();
	if (_tcscmp(strCurMode, TEXT("")) == 0)
	{
		return defaultValue;
	}

	// 再在当前配置下读取数据
	TCHAR szValue[MAX_PATH] = { 0 };
	GetPrivateProfileString(strCurMode, name, defaultValue, szValue, _countof(szValue), configPath_);
	if (_tcscmp(szValue, TEXT("")) == 0)
	{
		return defaultValue;
	}
	else
	{
		return CString(szValue);
	}
}

bool VariableConfigBlock::IsNull() const
{
	if (_tcscmp(configPath_, TEXT("")) == 0)
	{
		return true;
	}

	// 检查配置文件是否存在
	return !PathFileExists(configPath_);
}