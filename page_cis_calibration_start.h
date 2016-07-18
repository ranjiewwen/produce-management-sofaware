#pragma once

#include "resource.h"
#include "page.h"
#include "label.h"
#include "button.h"
#include "anchor_layout.h"

// PageCISCalibrationStart dialog

class PageCISCalibrationStart : public Page
{
	DECLARE_DYNAMIC(PageCISCalibrationStart)

public:
	PageCISCalibrationStart(CWnd* pParent = NULL);   // standard constructor
	virtual ~PageCISCalibrationStart();

// Dialog Data
	enum { IDD = IDD_PAGE_CIS_CALIBRATION_START };

protected:
  Label         labelTips_;
  Button        startButton_;
  AnchorLayout  layout_;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
  virtual BOOL OnInitDialog();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnBnClickedButtonStart();
};
