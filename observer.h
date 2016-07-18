#pragma once

#include <list>

class Observable;

class Observer {
public:
  Observer(LPCTSTR name, int subject);
  virtual ~Observer();
  
  virtual void OnUpdate(int subject) = 0;
  
  LPCTSTR GetName() const;
  int GetSubject() const;

private:
  CString   name_;
  int       subject_;
};

template <typename T>
class MethodObserver : public Observer {
public:
  MethodObserver(LPCTSTR name, int subject, void (T::*method)(int), T *instance);

  virtual void OnUpdate(int subject);

private:
  void (T::*method_)(int);
  T *instance_;  
};

// class HWNDObserver
class HWNDObserver : public Observer {
public:
  HWNDObserver(LPCTSTR name, HWND hwnd, int subject, UINT messageId, bool post);

  virtual void OnUpdate(int subject);

private:
  HWND    hwnd_;
  UINT    messageId_;
  bool    post_;
};

// class Observable
class Observable {
public:
  Observable();
  virtual ~Observable();

  void NotifyUpdate(int subject);

  template<typename T>
  void AddObserver(LPCTSTR name, int subject, void (T::*method)(int), T *instance);
  void AddObserver(LPCTSTR name, HWND hwnd, int subject, UINT messageId, bool post = true);

  void RemoveObserver(LPCTSTR name);

private:
  typedef std::list<Observer *> Observers;

  Observers   observers_;
};