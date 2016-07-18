#pragma once

#include "progress_tracker.h"

inline void ProgressTracker::UpdateProgress(int current, int range) {
  UpdateProgress(::MulDiv(current, maxPos_ - minPos_, range) + minPos_);
}
inline void ProgressTracker::SetRange(int minPos, int maxPos) {
  minPos_ = minPos;
  maxPos_ = maxPos;
}
inline int ProgressTracker::GetMinPos() const {
  return minPos_;
}
inline int ProgressTracker::GetMaxPos() const {
  return maxPos_;
}
inline bool ProgressTracker::IsCancelled() const {
  return cancelled_;
}
inline void ProgressTracker::Cancel() {
  cancelled_ = true;
}

