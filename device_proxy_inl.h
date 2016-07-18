#pragma once

#include "device_proxy.h"

inline int DataPacket::GetLength() const {
  return length_;
}
inline char *DataPacket::GetData() {
  return data_;
}
inline int DataPacket::GetType() const {
  return type_;
}
inline int DataPacket::GetCount() const {
  return count_;
}