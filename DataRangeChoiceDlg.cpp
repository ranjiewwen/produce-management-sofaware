// DataRangeChoiceDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DataRangeChoiceDlg.h"
#include "afxdialogex.h"
#include "working_parameters.h"


// DataRangeChoiceDlg 对话框

IMPLEMENT_DYNAMIC(DataRangeChoiceDlg, CDialog)

DataRangeChoiceDlg::DataRangeChoiceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(DataRangeChoiceDlg::IDD, pParent)
{
	InitRadioboxStatus();
}

DataRangeChoiceDlg::~DataRangeChoiceDlg()
{
}

void DataRangeChoiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_radiobtngrp1);
	DDX_Radio(pDX, IDC_RADIO3, m_radiobtngrp2);
}


BEGIN_MESSAGE_MAP(DataRangeChoiceDlg, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, &DataRangeChoiceDlg::OnBnClickedGroup1Radio)
	ON_BN_CLICKED(IDC_RADIO2, &DataRangeChoiceDlg::OnBnClickedGroup1Radio)

	ON_BN_CLICKED(IDC_RADIO3, &DataRangeChoiceDlg::OnBnClickedGroup2Radio)
	ON_BN_CLICKED(IDC_RADIO4, &DataRangeChoiceDlg::OnBnClickedGroup2Radio)
	ON_BN_CLICKED(IDC_RADIO5, &DataRangeChoiceDlg::OnBnClickedGroup2Radio)
	ON_BN_CLICKED(IDOK, &DataRangeChoiceDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// DataRangeChoiceDlg 消息处理程序

void DataRangeChoiceDlg::InitRadioboxStatus()
{
	// 从可变配置文件中读取点钞机的数据上传等级
	VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();   //之前在splash_screen初始化了mode.ini的路径
	int waveLevel = config->GetIntParameter(_T("WaveDataLevel"), 1);  //取得相应模式下的数据
	if ((waveLevel < 0) || (waveLevel > 1))
	{
		waveLevel = 1;
	}
	m_radiobtngrp1 = waveLevel;
	int imageLevel = config->GetIntParameter(_T("ImageDataLevel"), 2);
	if ((imageLevel < 0) || (imageLevel > 2))
	{
		imageLevel = 2;
	}
	m_radiobtngrp2 = imageLevel;
}

void DataRangeChoiceDlg::OnBnClickedGroup1Radio()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);  //作用是 将radio的状态值更新给关联的变量
	switch (m_radiobtngrp1)
	{
	case 0:
	//	MessageBox(L"radio1被选中，写进ini文件");
		break;
	case 1:
	//	MessageBox(L"radio2被选中，写进ini文件");
		break;
	default:
		break;
	}
}

void DataRangeChoiceDlg::OnBnClickedGroup2Radio()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);  //作用是 将radio的状态值更新给关联的变量
	switch (m_radiobtngrp2)
	{
	case 0:
	//	MessageBox(L"radio3被选中，写进ini文件");
		break;
	case 1:
	//	MessageBox(L"radio4被选中，写进ini文件");
		break;
	case 2:
	//	MessageBox(L"radio5被选中，写进ini文件");
		break;
	default:
		break;
	}
}

void DataRangeChoiceDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码

	// 从可变配置文件中读取点钞机的数据上传等级
	VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();   //之前在splash_screen初始化了mode.ini的路径
	
	if ((m_radiobtngrp1 < 0) || (m_radiobtngrp1 > 1))
	{
		m_radiobtngrp1 = 1;
	}
	config->SetIntParemeter(_T("WaveDataLevel"), m_radiobtngrp1);  //取得相应模式下的数据
	if ((m_radiobtngrp2 < 0) || (m_radiobtngrp2 > 2))
	{
		m_radiobtngrp2 = 1;
	}
	config->SetIntParemeter(_T("ImageDataLevel"), m_radiobtngrp2);
	CDialog::OnOK();
}
