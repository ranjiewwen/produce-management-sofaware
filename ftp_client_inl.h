#pragma once

#include "ftp_client.h"

// FtpFile
FtpFile::FtpFile(LPCTSTR name, DWORD attributes)
  : name_(name)
  , attrs_(attributes) {
}

const CString &FtpFile::GetName() const {
  return name_;
}

DWORD FtpFile::GetAttributes() const {
  return attrs_;
}

// FtpClient
template <typename Results>
bool FtpClient::ListFiles(LPCTSTR searchFile, Results &results) {
  WIN32_FIND_DATA wfd;
  HINTERNET hFind = FtpFindFirstFile(hSession_, 
      searchFile, &wfd, 
      INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD, 
      NULL);
  if (hFind == NULL) {
    return false;
  }
  while (1) {
    results.push_back(FtpFile(wfd.cFileName, wfd.dwFileAttributes));

    if (!InternetFindNextFile(hFind, &wfd)) {
      if (GetLastError() == ERROR_NO_MORE_FILES) {
        break;
      }
      InternetCloseHandle(hFind);
      return false;
    }
  }
  InternetCloseHandle(hFind);

  return true;
}

template <typename Results>
bool FtpClient::ListDirectories(LPCTSTR searchFile, Results &results) {
  WIN32_FIND_DATA wfd;
  HINTERNET hFind = FtpFindFirstFile(hSession_, 
      searchFile, &wfd, 
      INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD,
      NULL);
  if (hFind == NULL) {
    return false;
  }
  while (1) {
    if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      results.push_back(FtpFile(wfd.cFileName, wfd.dwFileAttributes));
    }
    if (!InternetFindNextFile(hFind, &wfd)) {
      if (GetLastError() == ERROR_NO_MORE_FILES) {
        break;
      }
      InternetCloseHandle(hFind);
      return false;
    }
  }
  InternetCloseHandle(hFind);

  return true;
}
