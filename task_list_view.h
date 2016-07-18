#pragma once

// TaskListView
class TaskListView : public CListBox {
	DECLARE_DYNAMIC(TaskListView)

public:
  enum TaskStatus {
    STATUS_WAITING,
    STATUS_PROCESSING,
    STATUS_COMPLETED,
    STATUS_ERROR,
  };
	TaskListView();
	virtual ~TaskListView();

  int AddTask(LPCTSTR name, TaskStatus status);
  void UpdateTaskStatus(int index, TaskStatus status);

protected:
  struct TaskItem {
    CString     name;
    TaskStatus  status;
  };
  CPen    dotPen_;

  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()
public:
  afx_msg void DeleteItem(LPDELETEITEMSTRUCT /*lpDeleteItemStruct*/);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  virtual void PreSubclassWindow();
};


