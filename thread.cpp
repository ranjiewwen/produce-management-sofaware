#include "stdafx.h"
#include "thread.h"
#include "thread_inl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Runnable
Runnable::Runnable() {
}

Runnable::~Runnable() {
}

// Thread
Thread::Thread()
: handle_(NULL)
, runnable_(NULL)
, exitCode_(0)
, interrupted_(false)  {
}

Thread::~Thread() {
  WaitForExit(INFINITE);
}

bool Thread::Start() {
  if (IsAlive()) {
    SetLastError(ERROR_ALREADY_INITIALIZED);
    return false;
  }
  interrupted_ = false;
  DWORD id = 0;
  handle_ = CreateThread(NULL, 0, ThreadProc, this, 0, &id);
  return handle_ != NULL;
}

bool Thread::Start(Runnable *runnable) {
  if (IsAlive()) {
    SetLastError(ERROR_ALREADY_INITIALIZED);
    return false;
  }
  interrupted_ = false;
  DWORD id = 0;
  handle_ = CreateThread(NULL, 0, ThreadProc, this, CREATE_SUSPENDED, &id);
  if (handle_ == NULL) {
    return false;
  }  
  runnable_ = runnable;  
  ResumeThread(handle_);
  return true;
}

void Thread::Interrupt() {
  interrupted_ = true;
}

bool Thread::WaitForExit(DWORD milliseconds) {
  if (handle_ == NULL) {
    return true;
  }
  int ret = WaitForSingleObject(handle_, milliseconds);
  if (ret == WAIT_OBJECT_0) {
    CloseHandle(handle_);
    handle_ = NULL;
    if (runnable_ != NULL) {
      delete runnable_;
      runnable_ = NULL;
    }
    return true;
  }
  return false;
}

DWORD WINAPI Thread::ThreadProc(void *arg) {
  static_cast<Thread *>(arg)->Run();
  return 0;
}

void Thread::Run() {
  if (runnable_ != NULL) {
    exitCode_ = runnable_->Run();
  }
}
