#pragma once

#include "page.h"
#include "resource.h"
#include "label.h"
#include "anchor_layout.h"
#include "thread.h"

// PageConnect dialog

class PageConnect : public Page
{
	DECLARE_DYNAMIC(PageConnect)

public:
	PageConnect(CWnd* pParent = NULL);   // standard constructor
	virtual ~PageConnect();

	// Dialog Data
	enum { IDD = IDD_PAGE_CONNECT };

	virtual void OnEnter(int leavePageId);
	virtual bool OnLeave(int enterPageId);

protected:
	Label                 labelTips_;
	AnchorLayout          layout_;
	Thread                connectThread_;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DWORD TryConnect();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
