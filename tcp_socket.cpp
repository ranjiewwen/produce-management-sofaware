#include "stdafx.h"
#include "tcp_socket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TcpSocket::TcpSocket()
  : socket_(INVALID_SOCKET) {
}

TcpSocket::~TcpSocket() {
  Close();
}

bool TcpSocket::Connect(LPCTSTR address, int port, int milliseconds/* = 2000*/) {
  socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_ == INVALID_SOCKET) {
    SetLastError(WSAGetLastError());
    return false;
  }

  unsigned long ul = 1;
  ioctlsocket(socket_, FIONBIO, &ul);

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.S_un.S_addr = inet_addr(/*CT2A(address)*/"172.16.100.174");
  addr.sin_port = htons(port);
  if (connect(socket_, (const sockaddr *)&addr, sizeof(addr)) != 0) {
    timeval tm;
    tm.tv_sec = 0;
    tm.tv_usec = milliseconds * 1000;
    fd_set set;
    FD_ZERO(&set);
    FD_SET(socket_, &set);
    bool success = false;
    if (select(socket_ + 1, NULL, &set, NULL, &tm) > 0) {
      int error = -1, len = sizeof(int);
      getsockopt(socket_, SOL_SOCKET, SO_ERROR, (char *)&error, &len);
      success = (error == 0);
    }
    if (!success) {
      Close();
      SetLastError(ERROR_TIMEOUT);
      return false;
    }
  }

  ul = 0;
  ioctlsocket(socket_, FIONBIO, &ul);

  return true;
}

bool TcpSocket::Listen(LPCTSTR address, int port) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.S_un.S_addr = inet_addr(CT2A(address));
  addr.sin_port = htons(port);

  return Listen((const sockaddr *)&addr, sizeof(addr));
}

bool TcpSocket::Listen(const sockaddr *addr, int addrLength) {
  socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_ == INVALID_SOCKET) {
    SetLastError(WSAGetLastError());
    return false;
  }

  if (bind(socket_, addr, addrLength) == SOCKET_ERROR) {
    SetLastError(WSAGetLastError());
    return false;
  }

  if (listen(socket_, 1) == SOCKET_ERROR) {
    SetLastError(WSAGetLastError());
    return false;
  }

  return true;
}

SOCKET TcpSocket::Accept(int waitMilliseconds) {
  timeval tm;
  tm.tv_sec = 0;
  tm.tv_usec = waitMilliseconds * 1000;
  fd_set set;
  FD_ZERO(&set);
  FD_SET(socket_, &set);
  int ret = select(socket_ + 1, &set, NULL, NULL, &tm);
  if (ret > 0) {
    sockaddr_in addr;
    int addrLen = sizeof(addr);
    SOCKET s = accept(socket_, (sockaddr *)&addr, &addrLen);
    if (s == INVALID_SOCKET) {
      SetLastError(WSAGetLastError());
    }
    return s;
  } else {
    SetLastError(ret == 0 ? WSAETIMEDOUT : WSAGetLastError());
    return INVALID_SOCKET;
  }
}

void TcpSocket::Attach(SOCKET s) {
  Close();

  socket_ = s;
}

bool TcpSocket::SetTimeout(int milliseconds) {
  DWORD timeout = milliseconds;

  if (setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == SOCKET_ERROR ||
      setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
    return false;
  }
  return true;
}

bool TcpSocket::SetSendBufferSize(int size) {
  if (setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, (const char *)&size, sizeof(size)) == SOCKET_ERROR) {
    return false;
  }
  return true;
}

bool TcpSocket::SetRecvBufferSize(int size) {
  if (setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, (const char *)&size, sizeof(size)) == SOCKET_ERROR) {
    return false;
  }
  return true;
}

int TcpSocket::Receive(void *buffer, int size) {  
  return recv(socket_, (char *)buffer, size, 0);
}

bool TcpSocket::ReceiveFully(void *buffer, int size) {
  int received = 0;
  while (received < size) {
    int ret = recv(socket_, (char *)buffer + received, size - received, 0);
    if (ret == -1) {
      SetLastError(WSAGetLastError());
      return false;
    }
    if (ret == 0) {
      SetLastError(ERROR_NO_MORE_ITEMS);
      return false;
    }
    received += ret;
  }
  return true;
}

bool TcpSocket::Send(const void *data, int size) {
  int ret = send(socket_, (const char *)data, size, 0);

  return ret == size;
}

void TcpSocket::Close() {
  if (socket_ != INVALID_SOCKET) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
  }
}

bool TcpSocket::IsOpened() const {
  return socket_ != INVALID_SOCKET;
}

bool TcpSocket::GetAddress(sockaddr *addr, int *addrLen) {
  if (getsockname(socket_, addr, addrLen) == SOCKET_ERROR) {
    SetLastError(WSAGetLastError());
    return false;
  }
  return true;
}