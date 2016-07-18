#pragma once

#include "resource.h"

// DataRangeChoiceDlg 对话框

class DataRangeChoiceDlg : public CDialog
{
	DECLARE_DYNAMIC(DataRangeChoiceDlg)

public:
	DataRangeChoiceDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DataRangeChoiceDlg();

// 对话框数据
	enum { IDD = IDD_DATA_RANGLE_CHOICE };
public:
	void  InitRadioboxStatus();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_radiobtngrp1;
	afx_msg void OnBnClickedGroup1Radio();
	int m_radiobtngrp2;
	afx_msg void OnBnClickedGroup2Radio();
	afx_msg void OnBnClickedOk();
};
