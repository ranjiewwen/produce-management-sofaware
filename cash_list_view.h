#pragma once
#include <Windows.h>
// CashListView
class RunCashMonitor;

class CashListView : public CWnd
{
	DECLARE_DYNAMIC(CashListView)

public:
	CashListView();
	virtual ~CashListView();

  void SetMonitor(RunCashMonitor *monitor);
  void Update();
  void SelectItem(int index);
  void EnsureItemVisiable(int index);
  int GetSelected() const;  
  virtual void PreSubclassWindow();
  TCHAR   szPath[MAX_PATH];
protected:
  RunCashMonitor    *monitor_;
  int               itemHeight_;
  int               selectedItem_;
  int               topItem_;
  int               itemCount_;
  CString           filePath;
  void SelectItemAndNotify(int index);
  void DrawRow(CDC *dc, int y, int index, CString text[6], int width[6]);
  void SetTopItem(int index);

  DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();  
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
  afx_msg UINT OnGetDlgCode();
  afx_msg void OnKillFocus(CWnd* pNewWnd);
  afx_msg void OnSetFocus(CWnd* pOldWnd);
public:
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

public:
	static char *szdir;
public:
	static  int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData);
};



