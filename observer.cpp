#include "stdafx.h"
#include "observer.h"
#include "observer_inl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Observable
Observable::Observable() {
}

Observable::~Observable() {
}

void Observable::NotifyUpdate(int subject) {
  Observers::iterator i = observers_.begin();
  while (i != observers_.end()) {
    Observers::iterator n = i;
    ++n;
    if ((*i)->GetSubject() == subject) {
      (*i)->OnUpdate(subject);
    }
    i = n;
  }
}

void Observable::AddObserver(LPCTSTR name, HWND hwnd, int subject, UINT messageId, bool post/* = true*/) {
  observers_.push_back(new HWNDObserver(name, hwnd, subject, messageId, post));
}

void Observable::RemoveObserver(LPCTSTR name) {
  for (Observers::iterator i = observers_.begin(); i != observers_.end(); ++i) {
    if (_tcscmp((*i)->GetName(), name) == 0) {
      delete (*i);
      observers_.erase(i);
      break;
    }
  }
}

// Observer
Observer::Observer(LPCTSTR name, int subject)
  : name_(name)
  , subject_(subject) {
}

Observer::~Observer() {
}

// HWNDObserver
HWNDObserver::HWNDObserver(LPCTSTR name, HWND hwnd, int subject, UINT messageId, bool post)
  : Observer(name, subject)
  , hwnd_(hwnd)
  , messageId_(messageId)
  , post_(post) {
}

void HWNDObserver::OnUpdate(int subject) {
  if (post_) {
    PostMessage(hwnd_, messageId_, 0, 0);
  } else {
    SendMessage(hwnd_, messageId_, 0, 0);
  }
}