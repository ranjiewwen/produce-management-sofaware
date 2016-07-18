#include "stdafx.h"
#include "ftp_client.h"
#include "error.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

FtpClient::FtpClient()
  : hInternet_(NULL)
  , hSession_(NULL) {

}

FtpClient::~FtpClient() {
  Close();
}

bool FtpClient::Initial()
{
	hInternet_ = InternetOpen(_T("ccmdbg"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternet_ == NULL) {
		SetLastError(ERROR_NETWORK_UNAVAILABLE);
		return false;
	}

	return true;
}

bool FtpClient::Connect(LPCTSTR serverName, int serverPort, LPCTSTR userName, LPCTSTR password) {
  Close();

  hInternet_ = InternetOpen(_T("ccmdbg"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  if (hInternet_ == NULL) {
    SetLastError(ERROR_NETWORK_UNAVAILABLE);
    return false;
  }
  
  hSession_ = InternetConnect(hInternet_, 
      serverName, serverPort, 
      userName, password, 
      INTERNET_SERVICE_FTP,
      INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_PASSIVE, 
      NULL);
  if (hSession_ == NULL) {
    LastErrorHolder errorHolder;
    switch (errorHolder.GetError()) {
    case 12014:
      errorHolder.SetError(ERROR_SERVER_DENIED);
      break;
    default:
      errorHolder.SetError(ERROR_SERVER_UNAVAILABLE);
      break;
    }
    InternetCloseHandle(hInternet_);
    hInternet_ = NULL;
    return false;
  }

  // 设置超时时间 lux
  DWORD dwTimeout = 3600000;
  if (!InternetSetOption(hSession_, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeout, sizeof(dwTimeout)))
  {
	  LastErrorHolder errorHolder;
	  InternetCloseHandle(hSession_);
	  hSession_ = NULL;
	  InternetCloseHandle(hInternet_);
	  hInternet_ = NULL;
	  return false;
  }

  return true;
}

bool FtpClient::GetFile(LPCTSTR remoteFile, LPCTSTR newFile) {
  if (!FtpGetFile(hSession_, 
        remoteFile, newFile, 
        FALSE, FILE_ATTRIBUTE_NORMAL, 
        FTP_TRANSFER_TYPE_BINARY, NULL)) {
    LastErrorHolder errorHolder;
    switch (errorHolder.GetError()) {
    case 12003:
      errorHolder.SetError(ERROR_SERVER_CONFIG);
      break;
    default:
      errorHolder.SetError(ERROR_SERVER_UNAVAILABLE);
      break;
    }
    return false;
  }
  return true;
}

CString FtpClient::GetFile(LPCTSTR remoteFile) {
  CString tempFilePath;
  LPTSTR tempFilePathBuffer = tempFilePath.GetBuffer(MAX_PATH);
  GetTempPath(MAX_PATH, tempFilePathBuffer);
  GetTempFileName(tempFilePathBuffer, _T("~ccmdbg_"), 0, tempFilePathBuffer);
  tempFilePath.ReleaseBuffer();
  if (!GetFile(remoteFile, tempFilePath)) {
    LastErrorHolder errorHolder;
    DeleteFile(tempFilePath);
    return _T("");
  }
  return tempFilePath;
}

bool FtpClient::PutFile(LPCTSTR localFile, LPCTSTR newRemoteFile) {
  return !!FtpPutFile(hSession_, localFile, newRemoteFile, FTP_TRANSFER_TYPE_BINARY, NULL);
}

void FtpClient::Close() {
  if (hSession_ != NULL) {
    InternetCloseHandle(hSession_);
    hSession_ = NULL;
  }
  if (hInternet_ != NULL) {
    InternetCloseHandle(hInternet_);
    hInternet_ = NULL;
  }
}