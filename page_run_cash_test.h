#pragma once

#include "resource.h"
#include "page.h"
#include "anchor_layout.h"
#include "check_point_list_box.h"

class RunCashMonitor;

class PageRunCashTest : public Page {
	DECLARE_DYNAMIC(PageRunCashTest)

public:
	PageRunCashTest(CWnd* pParent = NULL);   // standard constructor
	virtual ~PageRunCashTest();

// Dialog Data
	enum { IDD = IDD_PAGE_RUN_CASH_TEST };

  virtual BOOL OnInitDialog();

  virtual void OnEnter(int leavePageId);
  virtual bool OnLeave(int enterPageId);

protected:
  AnchorLayout          layout_;
  RunCashMonitor        *monitor_;
  CheckPointListBox     checkPointList_;
  CImageList            imageList_;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  void UpdateCheckState();
  void OnCashInfo(int subject);

  DECLARE_MESSAGE_MAP()

	afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnCashInfo(WPARAM wParam, LPARAM lParam);
  afx_msg void OnBnClickedButtonStep();
};
