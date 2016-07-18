#pragma once
#include "Resource.h"

// CInputNameKey 对话框

class CInputNameKey : public CDialog
{
	DECLARE_DYNAMIC(CInputNameKey)

public:
	CInputNameKey(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInputNameKey();
	virtual void OnOK();

// 对话框数据
	enum { IDD = IDD_INPUT_NAME_KEY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
