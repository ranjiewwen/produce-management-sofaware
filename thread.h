#pragma once

// class CriticalSection
class CriticalSection {
public:
  class ScopedLocker {
  public:
    ScopedLocker(CriticalSection &criSec, bool initialLock = true);
    ~ScopedLocker();
    void Lock();
    void Unlock();

  private:
    CriticalSection &criSec_;
    bool            locked_;
  };
  CriticalSection();
  ~CriticalSection();
  void Enter();
  bool TryEnter();
  void Leave();

private:
  CRITICAL_SECTION    criSec_;
};

// class Runnable
class Runnable {
public:
  Runnable();
  virtual ~Runnable();
  virtual DWORD Run() = 0;
};

// class MethodRunnable<T>
template <typename T>
class MethodRunnable : public Runnable {
public:
  MethodRunnable(T *instance, DWORD (T::*method)());
  virtual ~MethodRunnable();
  virtual DWORD Run();
private:
  T *instance_;
  DWORD (T::*method_)();
};

// class Runnable
class Thread {
public:
  Thread();
  virtual ~Thread();

  bool Start();
  bool Start(Runnable *runnable);
  bool WaitForExit(DWORD milliseconds);
  void Interrupt();
  bool IsInterrupted() const;
  bool IsAlive() const;
  DWORD GetExitCode() const;

protected:
  HANDLE    handle_;
  Runnable  *runnable_;
  DWORD     exitCode_;
  bool          interrupted_;

  static DWORD WINAPI ThreadProc(void *arg);

  virtual void Run();
};

