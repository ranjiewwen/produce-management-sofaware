#pragma once

#include "resource.h"
#include "bitmap.h"
#include "thread.h"
#include "button.h"

// SplashScreen dialog

class SplashScreen : public CDialog {
	DECLARE_DYNAMIC(SplashScreen)

public:
	SplashScreen(CWnd* pParent = NULL);   // standard constructor
	virtual ~SplashScreen();

  void ShowProgress(bool show);
  enum ButtonState{SELECT, RETRY, UNSHOWN};
  void ShowButtons(ButtonState state);

// Dialog Data
	enum { IDD = IDD_SPLASH_SCREEN };

private:
  Bitmap    marquee_;
  Bitmap    background_;
  Bitmap    icon_;
  CString   tips_;
  bool      showProgress_;
  Button    onlineButton_;
  Button    offlineButton_;
  int       progress_;
  Thread    initThread_;
  int       errorCode_;
  bool      canOffline_;

protected:  
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  virtual void OnOK();
  virtual void OnCancel();
  afx_msg void OnPaint();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg LRESULT OnInitEvent(WPARAM wParam, LPARAM lParam);

  DWORD InitAsync();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnBnClickedRetry();
  afx_msg void OnBnClickedOffline();
  afx_msg void OnBnClickedOnline();
};
