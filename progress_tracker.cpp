#include "stdafx.h"
#include "progress_tracker.h"
#include "progress_tracker_inl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ProgressTracker::ProgressTracker()
  : cancelled_(false) {
}

ProgressTracker::~ProgressTracker() {
}

void ProgressTracker::UpdateProgress(int minPos) {
}