#pragma once

#include "tcp_socket.h"

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

// class DataChannel
class DataChannel {
public:
  DataChannel();
  virtual ~DataChannel();

  bool Open(LPCTSTR address, int port);
  void Close();
  bool ReadPacket(DataPacket *packet);

private:
  TcpSocket   socket_;
};