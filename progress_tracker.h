#pragma once

class ProgressTracker {
public:
  ProgressTracker();
  virtual ~ProgressTracker();

  virtual void UpdateProgress(int pos);

  void UpdateProgress(int current, int range);
  void SetRange(int minPos, int maxPos);
  int GetMinPos() const;
  int GetMaxPos() const;
  bool IsCancelled() const;
  void Cancel();

protected:
  bool    cancelled_;
  int     minPos_, maxPos_;
};