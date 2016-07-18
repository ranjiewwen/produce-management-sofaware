// page_magnetic_detect.cpp : implementation file
//

#include "stdafx.h"
#include "page_magnetic_detect.h"
#include "gui_resources.h"
#include "run_cash_monitor.h"
#include "run_cash_monitor_inl.h"
#include "working_parameters.h"
#include "working_parameters_inl.h"
#include "main_dialog.h"
#include "device_proxy.h"
#include "device_proxy_inl.h"
#include "observer_inl.h"
#include "debug_logger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_CASH_DONE  WM_USER + 101
#define STEP_CHECK_WHITE_PAPER            1
#define STEP_CHECK_RMB100V05_SIDE1        2
#define STEP_CHECK_RMB100V05_SIDE2        3

// PageMagneticDetect dialog

IMPLEMENT_DYNAMIC(PageMagneticDetect, Page)

PageMagneticDetect::PageMagneticDetect(CWnd* pParent /*=NULL*/)
	: Page(PageMagneticDetect::IDD, pParent)
  , monitor_(NULL)
  , currentStep_(STEP_CHECK_WHITE_PAPER) {
}

PageMagneticDetect::~PageMagneticDetect() {
}

void PageMagneticDetect::DoDataExchange(CDataExchange* pDX) {
  Page::DoDataExchange(pDX);
}

BOOL PageMagneticDetect::OnInitDialog() {
  Page::OnInitDialog();

  // TODO:  Add extra initialization here
  hasError_ = false;
  completed_ = false;
  prevRMB100v5Side_ = -1;

  sketchMap_.LoadFromResource(IDB_SKETCH_MAP);
  redRect_.LoadFromResource(IDB_RED_RECT);
  greenRect_.LoadFromResource(IDB_GREEN_RECT);

  HeaderButton buttons = { IDC_BUTTON_CONTINUE, IDS_BUTTON_CONTINUE };
  SetHeaderButtons(&buttons, 1);
  ShowHeader(true);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void PageMagneticDetect::OnEnter(int leavePageId) {  
  hasError_ = false;
  completed_ = false;
  currentStep_ = STEP_CHECK_WHITE_PAPER;
  prevRMB100v5Side_ = -1;

  ShowHeaderButton(IDC_BUTTON_CONTINUE, false);
  SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_RUN_WHITE_PAPER, 0);
  
  sensorTesting_.Init(WorkingParameters::GetInstance());
  monitor_ = RunCashMonitor::GetInstance();
  monitor_->AddObserver(_T("PageMagneticDetect::OnCashInfo"), 
      RunCashMonitor::SUBJECT_CASH_INFO, 
      &PageMagneticDetect::OnCashInfo, this);  
}

bool PageMagneticDetect::OnLeave(int enterPageId) {
  monitor_->RemoveObserver(_T("PageMagneticDetect::OnCashInfo"));
  monitor_->Release();
  monitor_ = NULL;

  return true;
}

BEGIN_MESSAGE_MAP(PageMagneticDetect, Page)
  ON_WM_SIZE()
  ON_MESSAGE(WM_CASH_DONE, &PageMagneticDetect::OnCashDone)
  ON_BN_CLICKED(IDC_BUTTON_CONTINUE, &PageMagneticDetect::OnBnClickedButtonContinue)
END_MESSAGE_MAP()


// PageMagneticDetect message handlers
void PageMagneticDetect::OnSize(UINT nType, int cx, int cy) {
  Page::OnSize(nType, cx, cy);
}

void PageMagneticDetect::OnCashInfo(int subject) {
  if (currentStep_ != STEP_CHECK_WHITE_PAPER && 
      currentStep_ != STEP_CHECK_RMB100V05_SIDE1 &&
      currentStep_ != STEP_CHECK_RMB100V05_SIDE2) {
    return;
  }

  int cashCount = monitor_->Freeze();
  if (cashCount > 0) {
    Cash *cash = monitor_->GetCash(cashCount - 1);
    if (cash->Valid(Cash::VALID_ADC)) {
      MagneticSignal signals[MAGNETIC_HEADER_COUNT];
      signals[MAGNETIC_HEADER_HD].codes = cash->GetADCSampleCodes(ADC_CHANNEL_HD);
      signals[MAGNETIC_HEADER_HD].values = cash->GetADCSampleValues(ADC_CHANNEL_HD);
      signals[MAGNETIC_HEADER_HD].length = cash->GetADCSampleCount(ADC_CHANNEL_HD);
      signals[MAGNETIC_HEADER_B].codes = cash->GetADCSampleCodes(ADC_CHANNEL_BM);
      signals[MAGNETIC_HEADER_B].values = cash->GetADCSampleValues(ADC_CHANNEL_BM);
      signals[MAGNETIC_HEADER_B].length = cash->GetADCSampleCount(ADC_CHANNEL_BM);
      signals[MAGNETIC_HEADER_L].codes = cash->GetADCSampleCodes(ADC_CHANNEL_LM);
      signals[MAGNETIC_HEADER_L].values = cash->GetADCSampleValues(ADC_CHANNEL_LM);
      signals[MAGNETIC_HEADER_L].length = cash->GetADCSampleCount(ADC_CHANNEL_LM);
      signals[MAGNETIC_HEADER_R].codes = cash->GetADCSampleCodes(ADC_CHANNEL_RM);
      signals[MAGNETIC_HEADER_R].values = cash->GetADCSampleValues(ADC_CHANNEL_RM);
      signals[MAGNETIC_HEADER_R].length = cash->GetADCSampleCount(ADC_CHANNEL_RM);
      signals[MAGNETIC_HEADER_LS].codes = cash->GetADCSampleCodes(ADC_CHANNEL_LSM);
      signals[MAGNETIC_HEADER_LS].values = cash->GetADCSampleValues(ADC_CHANNEL_LSM);
      signals[MAGNETIC_HEADER_LS].length = cash->GetADCSampleCount(ADC_CHANNEL_LSM);
      signals[MAGNETIC_HEADER_RS].codes = cash->GetADCSampleCodes(ADC_CHANNEL_RSM);
      signals[MAGNETIC_HEADER_RS].values = cash->GetADCSampleValues(ADC_CHANNEL_RSM);
      signals[MAGNETIC_HEADER_RS].length = cash->GetADCSampleCount(ADC_CHANNEL_RSM);
      if (currentStep_ == STEP_CHECK_WHITE_PAPER) {
        hasError_ = !sensorTesting_.TestWhitePaperMagnetic(signals, results_);
      } else if (currentStep_ == STEP_CHECK_RMB100V05_SIDE1) {
        hasError_ = !sensorTesting_.TestRMB100V05Magnetic(signals, results_, &prevRMB100v5Side_);
      } else {
        ASSERT(currentStep_ == STEP_CHECK_RMB100V05_SIDE2);
        int side;
        hasError_ = !sensorTesting_.TestRMB100V05Magnetic(signals, results_, &side);
        if (!hasError_ && side != prevRMB100v5Side_) {
          completed_ = true;
        }
      }

      static const int WHITE_PAPER = 0;
      static const int RMB100V05 = 1;
      DebugLogger::GetInstance()->DetectMagneticSignal(hasError_ ? 1 : 0, 
          (currentStep_ == STEP_CHECK_WHITE_PAPER) ? WHITE_PAPER : RMB100V05, 
          signals);

	  if ((currentStep_ == STEP_CHECK_WHITE_PAPER) && (!hasError_))
	  {
		  // 如果检白纸成功，则发送胶带纸学习指令
		  int length = 0;
		  const void * data = cash->GetADCOriginalData(length);
		  if (DeviceProxy::GetInstance()->TapeStudy(data, length))
		  {
			  AfxMessageBox(_T("学习成功！"));
		  }
		  else
		  {
			  AfxMessageBox(_T("学习失败！"));
		  }
	  }
    }
  }
  monitor_->Unfreeze();
  
  if (hasError_) 
  {
    currentStep_ = 0;
  } else 
  {
    if (currentStep_ != STEP_CHECK_RMB100V05_SIDE2 || completed_)
	{
      currentStep_++;
    }
  }

  ::PostMessage(m_hWnd, WM_CASH_DONE, 0, 0);
}

void PageMagneticDetect::DrawClient(CDC &dc, const CRect &rect) {
	// 根据反馈，左边磁和左中磁报错位置反了，右边磁和右中磁报错位置反了，因此将这两组分别调换顺序 lux
  static RECT rects[] = {
    {187, 110, 226, 50}, // HD
    {187, 110, 226, 50}, // B
    //{84, 148, 32, 35}, // L
    //{482, 148, 32, 35}, // R
    //{193, 164, 32, 35}, // LS
    //{376, 164, 32, 35} // RS
	{ 193, 164, 32, 35 }, // L
	{ 376, 164, 32, 35 }, // R
	{ 84, 148, 32, 35 }, // LS
	{ 482, 148, 32, 35 } // RS
  };

  int x = rect.left + (rect.Width() - sketchMap_.GetWidth()) / 2;
  int y = rect.top + (rect.Height() - sketchMap_.GetHeight()) / 2;

  sketchMap_.Draw(dc.GetSafeHdc(), x, y);

  if (hasError_ || completed_) {
    dc.SetStretchBltMode(HALFTONE);
    for (int i = 0; i < 6; i++) {
      Bitmap *mask = results_[i] ? &redRect_ : &greenRect_;
      mask->Draw(dc.GetSafeHdc(), CRect(x + rects[i].left, 
          y + rects[i].top, 
          x + rects[i].left + rects[i].right,
          y + rects[i].top + rects[i].bottom));
    }
  }
}

void PageMagneticDetect::OnBnClickedButtonContinue() {
  if (hasError_) {
    hasError_ = false;
    currentStep_ = STEP_CHECK_WHITE_PAPER;
  
    SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_RUN_WHITE_PAPER, 0);
    ShowHeaderButton(IDC_BUTTON_CONTINUE, false);
  } else {    
    GotoStep(STEP_CIS_CALIBRATION);
  }
}

LRESULT PageMagneticDetect::OnCashDone(WPARAM, LPARAM) {
  if (hasError_) {
    SetHeaderInfo(HEADER_STYLE_ERROR, IDS_PROMPT_MAGNETIC_TEST_FAILED, 0);
    SetHeaderButtonText(IDC_BUTTON_CONTINUE, IDS_BUTTON_RETRY);
    ShowHeaderButton(IDC_BUTTON_CONTINUE, true);    
  } else {
    if (currentStep_ == STEP_CHECK_RMB100V05_SIDE1) {
      SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_RUN_RMB100V05, 0);
    } else if (currentStep_ == STEP_CHECK_RMB100V05_SIDE2) {
      SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_RUN_RMB100V05_OTHER_SIDE, 0);
    } else {
      ASSERT(completed_);
      // how to deal error return??
      DeviceProxy *device = DeviceProxy::GetInstance();
      device->UpdateDebugState(DEBUG_MASTER_SIGNAL_DETECT_DONE, DEBUG_MASTER_SIGNAL_DETECT_DONE);
      SetHeaderInfo(HEADER_STYLE_OK, IDS_PROMPT_MAGNETIC_DETECT_COMPLETED, 0);
      SetHeaderButtonText(IDC_BUTTON_CONTINUE, IDS_BUTTON_CONTINUE);
      ShowHeaderButton(IDC_BUTTON_CONTINUE, true);
    }
  }
  
  Invalidate(FALSE);

  return 0;
}