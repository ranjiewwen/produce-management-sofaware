#pragma once

#include "page.h"
#include "resource.h"
#include "label.h"
#include "anchor_layout.h"
#include "thread.h"
#include "cis_calibrator.h"

// PageCISCalibration dialog
class PageCISCalibration : public Page
{
	DECLARE_DYNAMIC(PageCISCalibration)

public:
	PageCISCalibration(CWnd* pParent = NULL);   // standard constructor
	virtual ~PageCISCalibration();

// Dialog Data
	enum { IDD = IDD_PAGE_CIS_CALIBRATION };

  virtual void OnEnter(int leavePageId);
  virtual bool OnLeave(int enterPageId);

protected:
  Label               labelTop_;
  Label               labelBottom_;
  AnchorLayout        layout_;
  Thread              thread_;
  CISCalibrator       calibrator_;
  bool                calibrateResult_;

  void StartCalibrate();
  DWORD CalibrateAsync();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
  virtual BOOL OnInitDialog();
  virtual void DrawClient(CDC &dc, const CRect &rect);
  void DrawImage(CDC &dc, int side, int white, const CRect &rect);

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnCalibrationDone(WPARAM wParam, LPARAM lParam);
  afx_msg void OnBnClickedButtonContinue();

  DECLARE_MESSAGE_MAP()
};
