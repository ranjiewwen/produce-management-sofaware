#pragma once

// class FtpFile
class FtpFile {
public:
  FtpFile(LPCTSTR name, DWORD attributes);

  const CString &GetName() const;
  DWORD GetAttributes() const;

private:
  CString   name_;
  DWORD     attrs_;
};

// class FtpClient
class FtpClient {
public:
  FtpClient();
  virtual ~FtpClient();

  bool Initial();
  bool Connect(LPCTSTR serverName, int serverPort, LPCTSTR userName, LPCTSTR password);
  template <typename Results>
  bool ListFiles(LPCTSTR searchFile, Results &results);
  template <typename Results>
  bool ListDirectories(LPCTSTR searchFile, Results &results);
  bool GetFile(LPCTSTR remoteFile, LPCTSTR newFile);
  CString GetFile(LPCTSTR remoteFile);
  bool PutFile(LPCTSTR localFile, LPCTSTR newRemoteFile);
  void Close();

private:
  HINTERNET hInternet_;
  HINTERNET hSession_;
};