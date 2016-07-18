#pragma once

#include "run_cash_monitor.h"

// Cash
inline int Cash::Id() const {
  return id_;
}

inline short Cash::GetADCSampleCount(int channel) {
  ASSERT(channel >= 0 && channel < ADC_CHANNEL_COUNT);
  return adcChannels_[channel].count;
}

inline const short *Cash::GetADCSampleCodes(int channel) {
  ASSERT(channel >= 0 && channel < ADC_CHANNEL_COUNT);
  return adcChannels_[channel].codes;
}

inline const short *Cash::GetADCSampleValues(int channel) {
  ASSERT(channel >= 0 && channel < ADC_CHANNEL_COUNT);
  return adcChannels_[channel].values;
}

inline const void *Cash::GetADCOriginalData(int &length)
{
	length = adcDataLength_;
	return adcData_;
}

inline Bitmap *Cash::GetSNImage() {
  return &snImage_;
}

inline Bitmap *Cash::GetTopImage() {
  return &topImage_;
}

inline Bitmap *Cash::GetBottomImage() {
  return &bottomImage_;
}

inline bool Cash::Valid(int flags) const {
  return (valid_ & flags) == flags;
}

inline int Cash::GetDirection() const {
  return direction_;
}

inline int Cash::GetDenomination() const {
  return denomination_;
}

inline int Cash::GetVersion() const {
  return version_;
}

inline int Cash::GetError() const {
  return error_;
}

inline const char *Cash::GetSN() const {
  return sn_;
}