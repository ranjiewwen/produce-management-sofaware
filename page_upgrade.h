#pragma once

#include "page.h"
#include "resource.h"
#include <vector>
#include "anchor_layout.h"
#include "progress_bar.h"
#include "thread.h"
#include "progress_tracker.h"
#include "check_point_list_box.h"

// PageUpgrade dialog

class PageUpgrade : public Page
{
	DECLARE_DYNAMIC(PageUpgrade)

public:
	PageUpgrade(CWnd* pParent = NULL);   // standard constructor
	virtual ~PageUpgrade();

// Dialog Data
	enum { IDD = IDD_PAGE_UPGRADE };

  virtual void OnEnter(int leavePageId);
  virtual bool OnLeave(int enterPageId);

protected:
  class UpgradeProgressTracker : public ProgressTracker {
  public:
    UpgradeProgressTracker();
    
    void AttachWindow(HWND hWnd);
    virtual void UpdateProgress(int pos);

  private:
    HWND            hWnd_;    
  };

  AnchorLayout            layout_;
  ProgressBar             progressBar_;
  Thread                  upgradeThread_;
  CString                 upgradeDeviceSN_;
  UpgradeProgressTracker  progressTracker_;
  CheckPointListBox       taskListView_;
  CImageList              imageList_;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  DWORD UpgradeAsync();

	virtual BOOL OnInitDialog();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnBnClickedButtonStart();
  afx_msg LRESULT OnUpgradeCompleted(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnUpgradeProgress(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnUpgradeStep(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnUpgradeStepCompleted(WPARAM wParam, LPARAM lParam);

  DECLARE_MESSAGE_MAP()
};
