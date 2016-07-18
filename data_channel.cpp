#include "stdafx.h"
#include "data_channel.h"
#include "data_channel_inl.h"
#include "error.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// DataPacket
DataPacket::DataPacket()
  : type_(-1)
  , count_(0)
  , length_(0)
  , data_(NULL)
  , capacity_(0) {
}

DataPacket::~DataPacket() {
  if (data_ != NULL) {
    delete[] data_;
  }
}

void DataPacket::Create(int type, int count, int length) {
  if (length > capacity_) {
    if (data_ != NULL) {
      delete[] data_;
    }
    data_ = new char[length];
    capacity_ = length;
  }
  type_ = type;
  count_ = count;
  length_ = length;
}

// DataChannel
DataChannel::DataChannel() {
}

DataChannel::~DataChannel() {
}

bool DataChannel::Open(LPCTSTR address, int port) {
  if (socket_.IsOpened()) {
    SetLastError(ERROR_ALREADY_INITIALIZED);
    return false;
  }

  return socket_.Listen(address, port);
}

void DataChannel::Close() {
  socket_.Close();
}

struct DataPacketHeader {
  unsigned char signatures[2];
  unsigned short type;
  unsigned int count;
  unsigned int length;
};

bool DataChannel::ReadPacket(DataPacket *packet) {
  DataPacketHeader hd;
  if (!socket_.ReceiveFully(&hd, sizeof(hd))) {
    return false;
  }
  if (hd.signatures[0] != 'D' || hd.signatures[1] != 'T') {
    SetLastError(ERROR_DEVICE_RESULT_ERROR);
    return false;
  }
  packet->Create(hd.type, hd.count, hd.length);
  if (hd.length > 0) {
    if (!socket_.ReceiveFully(packet->GetData(), hd.length)) {
      return false;
    }
  }
  return true;
}