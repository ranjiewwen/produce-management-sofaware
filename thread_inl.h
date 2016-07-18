#pragma once

#include "thread.h"

// class MethodRunnable<T>
template <typename T>
MethodRunnable<T>::MethodRunnable(T *instance, DWORD(T::*method)())
: instance_(instance)
, method_(method) {
}

template <typename T>
MethodRunnable<T>::~MethodRunnable() {
}

template <typename T>
DWORD MethodRunnable<T>::Run() {
  return (instance_->*method_)();
}

// class Thread
inline bool Thread::IsAlive() const {
  return handle_ != NULL;
}

inline DWORD Thread::GetExitCode() const {
  return exitCode_;
}

inline bool Thread::IsInterrupted() const {
  return interrupted_;
}

// CriticalSection
inline CriticalSection::CriticalSection() {
  InitializeCriticalSection(&criSec_);
}

inline CriticalSection::~CriticalSection() {
  DeleteCriticalSection(&criSec_);
}

inline void CriticalSection::Enter() {
  EnterCriticalSection(&criSec_);
}

inline bool CriticalSection::TryEnter() {
  return !!TryEnterCriticalSection(&criSec_);
}

inline void CriticalSection::Leave() {
  LeaveCriticalSection(&criSec_);
}

// class CriticalSection::ScopedLocker
inline CriticalSection::ScopedLocker::ScopedLocker(CriticalSection &criSec, bool initialLock/* = true*/)
  : criSec_(criSec)
  , locked_(false) {
  if (initialLock) {
    Lock();
  }
}

inline CriticalSection::ScopedLocker::~ScopedLocker() {
  Unlock();
}

inline void CriticalSection::ScopedLocker::Lock() {
  if (!locked_) {
    criSec_.Enter();
    locked_ = true;
  }
}

inline void CriticalSection::ScopedLocker::Unlock() {
  if (locked_) {
    criSec_.Leave();
    locked_ = false;
  }
}