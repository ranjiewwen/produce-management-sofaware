// page_cis_calibration.cpp : implementation file
//

#include "stdafx.h"
#include "page_cis_calibration.h"
#include "gui_resources.h"
#include "device_proxy_inl.h"
#include "working_parameters.h"
#include "working_parameters_inl.h"
#include "thread_inl.h"
#include "graphics.h"
#include "main_dialog.h"
#include "debug_logger.h"
#include "error.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_CALIBRATION_DONE WM_USER + 100

#define STEP_ADJUST   1
#define STEP_CORRECT  2

// PageCISCalibration dialog
IMPLEMENT_DYNAMIC(PageCISCalibration, Page)

PageCISCalibration::PageCISCalibration(CWnd* pParent /*=NULL*/)
	: Page(PageCISCalibration::IDD, pParent) {
}

PageCISCalibration::~PageCISCalibration() {
}

void PageCISCalibration::DoDataExchange(CDataExchange* pDX) {
	Page::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_LABEL_TOP, labelTop_);
  DDX_Control(pDX, IDC_LABEL_BOTTOM, labelBottom_);
}

void PageCISCalibration::OnEnter(int leavePageId) {
  SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_PLACE_WHITE_STRIP, 0);
  SetHeaderButtonText(IDC_BUTTON_STEP, IDS_BUTTON_START);
  ShowHeaderButton(IDC_BUTTON_STEP, true);

  calibrateResult_ = false;

  DeviceProxy::GetInstance()->LightCIS(CIS_TOP, CIS_COLOR_MASK_WHITE);
  DeviceProxy::GetInstance()->LightCIS(CIS_BOTTOM, CIS_COLOR_MASK_WHITE);
}

bool PageCISCalibration::OnLeave(int enterPageId) {
  if (thread_.IsAlive()) {
    return false;
  }
  
  DeviceProxy::GetInstance()->UnlightCIS(CIS_TOP);
  DeviceProxy::GetInstance()->UnlightCIS(CIS_BOTTOM);

  return true;
}

void PageCISCalibration::StartCalibrate() {
  EnableConnectionCheck(false);

  SetHeaderInfo(HEADER_STYLE_INFO, IDS_CIS_CALIBRATING, 0);

  calibrator_.Init(DeviceProxy::GetInstance(), WorkingParameters::GetInstance());

  // 如果要进行本地保存，则保存CIS校准图像
  
  thread_.Start(new MethodRunnable<PageCISCalibration>(this, &PageCISCalibration::CalibrateAsync));

  Invalidate(FALSE);
}

DWORD PageCISCalibration::CalibrateAsync() {
  bool result = calibrator_.AdjustOffset();
  if (result) {
    result = calibrator_.AdjustExposure();
    if (result) {
      result = calibrator_.AdjustGain();
      if (result) {
        result = calibrator_.Correct();
        if (result) {
          result = calibrator_.CheckUniformity();
          if (result) {
            result = calibrator_.Submit();
          }
        }
      }
    }
  }

  CISImageInfo images[8] = {      
    {"top-black-green", calibrator_.GetImage(CISCalibrator::TOP, COLOR_GREEN, CISCalibrator::BLACK)},
    {"top-black-ir", calibrator_.GetImage(CISCalibrator::TOP, COLOR_IR, CISCalibrator::BLACK)},
    {"bottom-black-green", calibrator_.GetImage(CISCalibrator::BOTTOM, COLOR_GREEN, CISCalibrator::BLACK)},
    {"bottom-black-ir", calibrator_.GetImage(CISCalibrator::BOTTOM, COLOR_IR, CISCalibrator::BLACK)},
    {"top-white-green", calibrator_.GetImage(CISCalibrator::TOP, COLOR_GREEN, CISCalibrator::WHITE)},
    {"top-white-ir", calibrator_.GetImage(CISCalibrator::TOP, COLOR_IR, CISCalibrator::WHITE)},
    {"bottom-white-green", calibrator_.GetImage(CISCalibrator::BOTTOM, COLOR_GREEN, CISCalibrator::WHITE)},
    {"bottom-white-ir", calibrator_.GetImage(CISCalibrator::BOTTOM, COLOR_IR, CISCalibrator::WHITE)},
  };
  const CISErrorInfo *errorInfo = (result ? NULL : calibrator_.GetErrorInfo());
  DebugLogger::GetInstance()->CalibrateCIS(0, 
    errorInfo,
    calibrator_.GetParameters(), images, 8);

  PostMessage(WM_CALIBRATION_DONE, 0, result ? 1 : 0);

  return 0;
}

BEGIN_MESSAGE_MAP(PageCISCalibration, Page)
  ON_WM_SIZE()
  ON_MESSAGE(WM_CALIBRATION_DONE, &PageCISCalibration::OnCalibrationDone)
  ON_BN_CLICKED(IDC_BUTTON_STEP, &PageCISCalibration::OnBnClickedButtonContinue)
END_MESSAGE_MAP()


// PageCISCalibration message handlers

BOOL PageCISCalibration::OnInitDialog() {
  Page::OnInitDialog();

  layout_.Init(m_hWnd);
  layout_.AddDlgItem(IDC_LABEL_TOP, AnchorLayout::TOP_LEFT, AnchorLayout::TOP_RIGHT);
  layout_.AddDlgItem(IDC_TOP_IMAGE, AnchorLayout::TOP_LEFT, AnchorLayout::TOP_RIGHT);
  layout_.AddDlgItem(IDC_LABEL_BOTTOM, AnchorLayout::TOP_LEFT, AnchorLayout::TOP_RIGHT);
  layout_.AddDlgItem(IDC_BOTTOM_IMAGE, AnchorLayout::TOP_LEFT, AnchorLayout::TOP_RIGHT);

  calibrateResult_ = false;

  HeaderButton buttons = { IDC_BUTTON_STEP, IDS_BUTTON_START };
  SetHeaderButtons(&buttons, 1);
  ShowHeader(true);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}


void PageCISCalibration::OnSize(UINT nType, int cx, int cy)
{
  Page::OnSize(nType, cx, cy);

  // TODO: Add your message handler code here
  if (::IsWindow(m_hWnd)) {
    layout_.RecalcLayout();
  }
}

LRESULT PageCISCalibration::OnCalibrationDone(WPARAM wParam, LPARAM lParam) {
  thread_.WaitForExit(INFINITE);

  bool result = (lParam != 0);
  
  if (result) {
    SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_CIS_CALIBRATION_COMPLETED, 0);
    SetHeaderButtonText(IDC_BUTTON_STEP, IDS_BUTTON_CONTINUE);
  } else {
    const CISErrorInfo *errorInfo = calibrator_.GetErrorInfo();
    CString errorMsg;
    if (errorInfo->side != -1) {
      errorMsg.Format(
          errorInfo->side == CISCalibrator::TOP ? IDS_TOP_CIS_ERROR : IDS_BOTTOM_CIS_ERROR, 
          (LPCTSTR)FormatErrorMessage(errorInfo->result));
    } else {
      errorMsg = FormatErrorMessage(errorInfo->result);
    }
    SetHeaderInfo(HEADER_STYLE_ERROR, errorMsg, NULL);
    SetHeaderButtonText(IDC_BUTTON_STEP, IDS_BUTTON_RETRY);
  }
  ShowHeaderButton(IDC_BUTTON_STEP, true);

  DeviceProxy::GetInstance()->LightCIS(CIS_TOP, CIS_COLOR_MASK_WHITE);
  DeviceProxy::GetInstance()->LightCIS(CIS_BOTTOM, CIS_COLOR_MASK_WHITE);

  calibrateResult_ = result;

  EnableConnectionCheck(true);

  Invalidate(FALSE);

  return 0;
}

void PageCISCalibration::OnBnClickedButtonContinue() {  
  if (thread_.IsAlive()) {
    return;
  }

  ShowHeaderButton(IDC_BUTTON_STEP, false);

  DeviceProxy::GetInstance()->UnlightCIS(CIS_TOP);
  DeviceProxy::GetInstance()->UnlightCIS(CIS_BOTTOM);

  if (calibrateResult_) {
    for (int side = 0; side < 2; side++) {
      for (int c = 0; c < COLOR_COUNT; c++) {
        CString filePath;
        filePath.Format(_T("d:\\gray_%d_%d.bmp"), side, c);
        calibrator_.GetImage(side, c, 1)->SaveToFile(filePath); 
      }
    }
    // 检查白条是否拿出
    if (calibrator_.CheckStripExists()) {
      SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_TAKE_OUT_WHITE_STRIP, 0);
      ShowHeaderButton(IDC_BUTTON_STEP, true);
      DeviceProxy::GetInstance()->LightCIS(CIS_TOP, CIS_COLOR_MASK_WHITE);
      DeviceProxy::GetInstance()->LightCIS(CIS_BOTTOM, CIS_COLOR_MASK_WHITE);
      return;
    }
    GotoStep(STEP_RUN_CASH_TEST);    
  } else {
    StartCalibrate();
  }
}

void PageCISCalibration::DrawClient(CDC &dc, const CRect &rect) {
  CRect rects[2];
  GetDlgItem(IDC_TOP_IMAGE)->GetWindowRect(&rects[1]);
  ScreenToClient(&rects[1]);
  GetDlgItem(IDC_BOTTOM_IMAGE)->GetWindowRect(&rects[0]);
  ScreenToClient(&rects[0]);
  
  if (thread_.IsAlive()) {
    dc.FillSolidRect(rects[0], RGB(0, 0, 0));
    dc.FillSolidRect(rects[1], RGB(0, 0, 0));
    dc.Draw3dRect(rects[0], RGB(182,183,185), RGB(182,183,185));
    dc.Draw3dRect(rects[1], RGB(182,183,185), RGB(182,183,185));
    return;
  }

  for (int side = 0; side < 2; side++) {
    dc.FillSolidRect(rects[side], RGB(0, 0, 0));
    CRect imageRect(rects[side]);
    imageRect.DeflateRect(1, 1, 1, 1);    
    CRect halfImageRect(imageRect);
    halfImageRect.bottom = halfImageRect.top + (imageRect.Height() - 2) / 2;
    DrawImage(dc, side, 0, halfImageRect);
    int y0 = halfImageRect.bottom;    
    halfImageRect.OffsetRect(0, imageRect.bottom - halfImageRect.bottom);
    int y1 = halfImageRect.top;
    DrawImage(dc, side, 1, halfImageRect);      
    dc.FillSolidRect(imageRect.left, y0, imageRect.Width(), y1 - y0, RGB(255, 255, 255));    
    int dstWidth = imageRect.Width() / COLOR_COUNT;
    for (int c = 0; c < COLOR_COUNT; c++) {
      for (int i = 0; i < calibrator_.GetDustCount(side, c); i++) {
        int pos = calibrator_.GetDustPosition(side, c, i);
        int x = imageRect.left + c * dstWidth + pos * dstWidth / CIS_IMAGE_WIDTH;
        for (int y = y0; y < y1; y++) {
          dc.SetPixel(x, y, RGB(255, 0, 0));
        }
      }
    }
    dc.Draw3dRect(rects[side], RGB(182,183,185), RGB(182,183,185));
  }
}

void PageCISCalibration::DrawImage(CDC &dc, int side, int white, const CRect &rect) {
  dc.SetStretchBltMode(HALFTONE);

  int dstWidth = rect.Width() / COLOR_COUNT;
  int dstHeight = rect.Height();
  int x = rect.left;
  int y = rect.top;
  for (int color = 0; color < COLOR_COUNT; color++) {
    Bitmap *bitmap = calibrator_.GetImage(side, color, white);
    if (bitmap->IsValid()) {
      bitmap->Draw(dc.GetSafeHdc(), CRect(x, y, x + dstWidth, y + dstHeight));
    } else {
      dc.FillSolidRect(CRect(x, y, x + dstWidth, y + dstHeight), RGB(0, 0, 0));
    }
    x += dstWidth;
  }
}
