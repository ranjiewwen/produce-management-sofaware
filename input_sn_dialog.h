#pragma once

#include "resource.h"
#include "label.h"
#include "button.h"

class SerialNumberEdit : public CEdit {
private:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};

// InputSNDialog dialog
class InputSNDialog : public CDialog {
	DECLARE_DYNAMIC(InputSNDialog)

public:
	InputSNDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~InputSNDialog();

// Dialog Data
	enum { IDD = IDD_INPUT_SN };

  virtual void OnOK();
  virtual void OnCancel();

private:
  SerialNumberEdit    snEdit_;
  Button              ok_;
  Label               tips_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  virtual BOOL OnInitDialog();
  afx_msg void OnBnClickedButtonOk();
  afx_msg void OnPaint();
  afx_msg LRESULT OnDeviceDisconnected(WPARAM, LPARAM);
  afx_msg void OnDestroy();
};
