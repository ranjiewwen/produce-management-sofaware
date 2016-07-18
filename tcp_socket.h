#pragma once

class TcpSocket {
public:
  TcpSocket();
  virtual ~TcpSocket();
  
  bool Connect(LPCTSTR address, int port, int milliseconds = 2000);
  bool Listen(LPCTSTR address, int port);
  bool Listen(const sockaddr *addr, int addrLength);
  SOCKET Accept(int waitMilliseconds);
  bool SetTimeout(int milliseconds);
  bool SetSendBufferSize(int size);
  bool SetRecvBufferSize(int size);
  int Receive(void *buffer, int size);
  bool ReceiveFully(void *buffer, int size);
  bool Send(const void *data, int size);
  void Close();
  bool IsOpened() const;
  void Attach(SOCKET s);
  bool GetAddress(sockaddr *addr, int *addrLen);

private:
  SOCKET  socket_;
};