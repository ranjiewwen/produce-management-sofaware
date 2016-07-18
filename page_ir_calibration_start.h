#pragma once

#include "page.h"
#include "resource.h"
#include "label.h"
#include "button.h"
#include "anchor_layout.h"

// PageIRCalibrationStart dialog

class PageIRCalibrationStart : public Page
{
	DECLARE_DYNAMIC(PageIRCalibrationStart)

public:
	PageIRCalibrationStart(CWnd* pParent = NULL);   // standard constructor
	virtual ~PageIRCalibrationStart();

// Dialog Data
  enum { IDD = IDD_PAGE_IR_CALIBRATION_START };

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
