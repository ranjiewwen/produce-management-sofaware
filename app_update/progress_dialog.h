
// progress_dialog.h : 头文件
//

#pragma once

#include "../thread.h"
#include "../bitmap.h"

// ProgressDialog 对话框
class ProgressDialog : public CDialog
{
// 构造
public:
	ProgressDialog(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_APP_UPDATE_DIALOG };

  void SetUpgradeInfo(LPCTSTR upgradeFilePath, LPCTSTR appDir, LPCTSTR version);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

  Bitmap    loaddingBitmap_;
  int       cxLoadding_, cyLoadding_;
  int       currentFrame_;
  int       frameCount_;
  Thread    upgradeThread_;

  DWORD UpgradeAsync();

// 实现
protected:
	HICON m_hIcon;
  CString upgradeFilePath_;
  CString appDir_;
  CString appVersion_;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
  afx_msg LRESULT OnUpgradeDone(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
  virtual void OnOK();
  virtual void OnCancel();
public:
  afx_msg void OnTimer(UINT_PTR nIDEvent);
};
