#pragma once

#include "observer.h"

// Observer
inline LPCTSTR Observer::GetName() const {
  return name_;
}

inline int Observer::GetSubject() const {
  return subject_;
}

// MethodObserver
template <typename T>
MethodObserver<T>::MethodObserver(LPCTSTR name, int subject, void (T::*method)(int), T *instance)
  : Observer(name, subject)
  , method_(method)
  , instance_(instance) {
}

template <typename T>
void MethodObserver<T>::OnUpdate(int subject) {
  (instance_->*method_)(subject);
}

// Observable
template<typename T>
void Observable::AddObserver(LPCTSTR name, int subject, void (T::*method)(int), T *instance) {
  observers_.push_back(new MethodObserver<T>(name, subject, method, instance));
}