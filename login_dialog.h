#pragma once

#include "resource.h"
#include "button.h"
#include "thread.h"
#include "label.h"
#include "bitmap.h"

// LoginDialog

class LoginDialog : public CDialog {
	DECLARE_DYNAMIC(LoginDialog)

public:
	LoginDialog();
	virtual ~LoginDialog();

  virtual BOOL OnInitDialog();
  bool isRemPswd;
protected:
  Button    loginButton_;
  Button    closeButton_;
  CButton   pBtn;
  CButton*  pButton;
  Label     userNameLabel_;
  Label     passwordLabel_;
  CString   userName_;
  CString   password_;
  Bitmap    header_;

  DWORD Login();

  virtual void DoDataExchange(CDataExchange* pDX);
  virtual void OnOK();
  virtual void OnCancel();

  afx_msg void OnPaint();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  
	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedCheckRempswd();
};


