#pragma once

#include "Resource.h"

class AboutDialog : public CDialog
{
public:
  AboutDialog();

  // 对话框数据
  enum { IDD = IDD_ABOUTBOX };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

  // 实现
protected:
  DECLARE_MESSAGE_MAP()
};