#pragma once

#include "page.h"
#include "resource.h"
#include "anchor_layout.h"
#include "label.h"
#include "button.h"
#include "sensor_testing.h"

// PageMagneticDetect dialog
class RunCashMonitor;

class PageMagneticDetect : public Page {
	DECLARE_DYNAMIC(PageMagneticDetect)

public:
	PageMagneticDetect(CWnd* pParent = NULL);   // standard constructor
	virtual ~PageMagneticDetect();

// Dialog Data
	enum { IDD = IDD_PAGE_MAGNETIC_DETECT };

  virtual BOOL OnInitDialog();

  virtual void OnEnter(int leavePageId);
  virtual bool OnLeave(int enterPageId);

protected:
  RunCashMonitor    *monitor_;
  SensorTesting     sensorTesting_;
  int               currentStep_;
  int               results_[MAGNETIC_HEADER_COUNT];
  bool              hasError_;
  int               prevRMB100v5Side_;
  bool              completed_;
  Bitmap            sketchMap_;
  Bitmap            redRect_;
  Bitmap            greenRect_;  

  virtual void DrawClient(CDC &dc, const CRect &rect);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  
  void OnCashInfo(int subject);

  DECLARE_MESSAGE_MAP()

	afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnCashDone(WPARAM wParam, LPARAM lParam);
  afx_msg void OnBnClickedButtonContinue();
};
