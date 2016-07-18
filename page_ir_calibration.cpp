// page_ir_calibration.cpp : implementation file
//

#include "stdafx.h"
#include "page_ir_calibration.h"
#include "gui_resources.h"
#include "thread_inl.h"
#include "device_proxy.h"
#include "device_proxy_inl.h"
#include "working_parameters.h"
#include "working_parameters_inl.h"
#include "graphics.h"
#include "main_dialog.h"
#include "debug_logger.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_ADJUST_DONE WM_USER + 100

IMPLEMENT_DYNAMIC(PageIRCalibration, Page)

PageIRCalibration::PageIRCalibration(CWnd* pParent /*=NULL*/)
	: Page(PageIRCalibration::IDD, pParent) {
}

PageIRCalibration::~PageIRCalibration() {
}

void PageIRCalibration::DoDataExchange(CDataExchange* pDX) {
  Page::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_STATIC_RESULT, resultView_);
}

void PageIRCalibration::OnEnter(int leavePageId) {
  adjustSuccessed_ = true;
  nextStep_ = STEP_FIRST;
  
  SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_PUT_IR_STICK, 0);
  SetHeaderButtonText(IDC_BUTTON_STEP, IDS_BUTTON_START);

  resultView_.SetStates(NULL);
}

bool PageIRCalibration::OnLeave(int enterPageId) {
  if (thread_.IsAlive()) {
    return false;
  }
  
  return true;
}

bool PageIRCalibration::Adjust() {
  if (!calibration_.AdjustWithPaper()) {
    DebugLogger::GetInstance()->CalibrateInfrared(1, calibration_.GetStates());

    return false;
  }

  if (!DeviceProxy::GetInstance()->StartMotor(500)) {
    return false;
  }

  if (!calibration_.VerifyNoPaper()) {
    DebugLogger::GetInstance()->CalibrateInfrared(1, calibration_.GetStates());

    return false;
  }
  
  DebugLogger::GetInstance()->CalibrateInfrared(0, calibration_.GetStates());
  
  if (!calibration_.Submit()) {
    return false;
  }

  return true;
}

DWORD PageIRCalibration::AdjustAsync() {
  adjustSuccessed_ = Adjust();
  
  if (!adjustSuccessed_) {
    adjustError_ = GetLastError();
  }

  ::PostMessage(m_hWnd, WM_ADJUST_DONE, 0, 0);

  return 0;
}

void PageIRCalibration::StartAdjust() {
  SetHeaderInfo(HEADER_STYLE_INFO, IDS_IR_CALIBRATING, 0);
  
  calibration_.Init(DeviceProxy::GetInstance(), WorkingParameters::GetInstance());

  thread_.Start(new MethodRunnable<PageIRCalibration>(this, &PageIRCalibration::AdjustAsync));
}

void PageIRCalibration::DrawClient(CDC &dc, const CRect &rect) {
  static POINT offsets[6] = {
    {31, 158},
    {55, 158},
    {168, 158},
    {417, 158},
    {528, 158},
    {552, 158}
  };
  CRect imageRect;
  GetDlgItem(IDC_STATIC_FIGURE)->GetWindowRect(&imageRect);
  ScreenToClient(&imageRect);
  sketchMap_.Draw(dc.GetSafeHdc(), imageRect.left, imageRect.top);

  if (thread_.IsAlive()) {
    return;
  }

  if (nextStep_ == STEP_NEXT || !adjustSuccessed_) {
    const IRState *states = calibration_.GetStates();
    for (int i = 0; i < 6; i++) {
      if (states[5 - i].status[0] == IR_STATUS_ERROR || states[5 - i].status[1] == IR_STATUS_ERROR) {
        redCircle_.Draw(dc.GetSafeHdc(), imageRect.left + offsets[i].x, imageRect.top + offsets[i].y);
      } else {
        greenCircle_.Draw(dc.GetSafeHdc(), imageRect.left + offsets[i].x, imageRect.top + offsets[i].y);
      }
    }
  }
}

BEGIN_MESSAGE_MAP(PageIRCalibration, Page)
  ON_WM_SIZE()
  ON_MESSAGE(WM_ADJUST_DONE, &PageIRCalibration::OnAdjustDone)
  ON_BN_CLICKED(IDC_BUTTON_STEP, &PageIRCalibration::OnBnClickedButtonStep)
END_MESSAGE_MAP()

// PageIRCalibration message handlers
BOOL PageIRCalibration::OnInitDialog() {
  Page::OnInitDialog();

  redCircle_.LoadFromResource(IDB_RED_CIRCLE);
  greenCircle_.LoadFromResource(IDB_GREEN_CIRCLE);
  sketchMap_.LoadFromResource(IDB_SKETCH_MAP);

  layout_.Init(m_hWnd);
  layout_.AddDlgItem(IDC_STATIC_FIGURE, AnchorLayout::CENTER, AnchorLayout::CENTER);
  layout_.AddDlgItem(IDC_STATIC_RESULT, AnchorLayout::CENTER, AnchorLayout::CENTER);

  HeaderButton buttons = { IDC_BUTTON_STEP, IDS_BUTTON_START };
  SetHeaderButtons(&buttons, 1);
  ShowHeader(true);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}


void PageIRCalibration::OnSize(UINT nType, int cx, int cy) {
  Page::OnSize(nType, cx, cy);

  if (::IsWindow(m_hWnd)) {
    layout_.RecalcLayout();
  }
}

LRESULT PageIRCalibration::OnAdjustDone(WPARAM, LPARAM) {
  thread_.WaitForExit(INFINITE);

  if (adjustSuccessed_) {
    nextStep_ = STEP_NEXT;
  } else {
    nextStep_ = STEP_FIRST;
  }  

  ShowAdjustResult();

  EnableConnectionCheck(true);

  Invalidate(FALSE);

  return 0;
}

void PageIRCalibration::OnBnClickedButtonStep() {
  resultView_.SetStates(NULL);

  if (nextStep_ == STEP_FIRST) {
    EnableConnectionCheck(false);

    StartAdjust();
  } else if (nextStep_ == STEP_NEXT) {
    GotoStep(STEP_MH_DETECT);
  }

  Invalidate(FALSE);
}

void PageIRCalibration::ShowAdjustResult() {
  if (adjustSuccessed_) {
    SetHeaderInfo(HEADER_STYLE_OK, IDS_IR_CALIBRATING_SUCCESSED, 0);
    SetHeaderButtonText(IDC_BUTTON_STEP, IDS_BUTTON_CONTINUE);
  } else {
    SetHeaderInfo(HEADER_STYLE_ERROR, IDS_IR_CALIBRATING_FAILED, 0);
    SetHeaderButtonText(IDC_BUTTON_STEP, IDS_BUTTON_RETRY);
  }

  resultView_.SetStates(calibration_.GetStates());
}

// IRStateListView
BEGIN_MESSAGE_MAP(IRStateListView, CStatic)
  ON_WM_PAINT()
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

IRStateListView::IRStateListView() : states_(NULL) {
}

IRStateListView::~IRStateListView() {
}

void IRStateListView::OnPaint() {
  BufferredPaintDC dc(this);

  CFont *oldFont = dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL));
  dc.SetBkMode(TRANSPARENT);
  CRect clientRect;
  GetClientRect(&clientRect);
  dc.FillSolidRect(clientRect, RGB(255, 255, 255));
  int cellWidth = clientRect.Width() / IRCalibration::IR_COUNT;
  int cellHeight = clientRect.Height() / 5;
  CString text;
  for (int i = 0; i < IRCalibration::IR_COUNT; i++) {
    int index = IRCalibration::IR_COUNT - 1 - i;

    CRect cellRect(0, 0, cellWidth, cellHeight);
    // IR
    cellRect.OffsetRect(clientRect.left + i * cellWidth, 0);    
    text.Format(_T("IR%d"), i + 1);
    DrawCell(dc, cellRect, text, RGB(247, 247, 247), RGB(0, 0, 0));
    // IR value
    cellRect.OffsetRect(0, cellHeight);
    if (states_ == NULL) {
      text.Empty();      
    } else {
      text.Format(_T("%d"), states_[index].emissionValue);
    }
    DrawCell(dc, cellRect, text, RGB(255, 255, 255), RGB(0, 0, 0));
    // DIR
    cellRect.OffsetRect(0, cellHeight);
    text.Format(_T("DIR%d"), i + 1);
    DrawCell(dc, cellRect, text, RGB(247, 247, 247), RGB(0, 0, 0));
    // DIR value with paper
    cellRect.OffsetRect(0, cellHeight);
    if (states_ == NULL || states_[index].status[0] == IR_STATUS_UNKNOWN) {
      text.Empty();
    } else {
      text.Format(_T("%d"), states_[index].collectionValue[0]);
    }
    DrawCell(dc, cellRect, text, RGB(255, 255, 255),
      (states_ != NULL && states_[index].status[0] == IR_STATUS_ERROR) ? RGB(255, 0, 0) : RGB(0, 0, 0));
    // DIR value no paper
    cellRect.OffsetRect(0, cellHeight);
    if (states_ == NULL || states_[index].status[1] == IR_STATUS_UNKNOWN) {
      text.Empty();
    } else {
      text.Format(_T("%d"), states_[index].collectionValue[1]);
    }
    DrawCell(dc, cellRect, text, RGB(255, 255, 255),
      (states_ != NULL && states_[index].status[1] == IR_STATUS_ERROR) ? RGB(255, 0, 0) : RGB(0, 0, 0));
  }
  dc.SelectObject(oldFont);
}

void IRStateListView::DrawCell(CDC &dc, const CRect &rect, const CString &text, COLORREF bgColor, COLORREF foreColor) {
  dc.FillSolidRect(rect, bgColor);
  CRect textRect = rect;
  dc.SetTextColor(foreColor);
  dc.DrawText(text, textRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
  dc.Draw3dRect(rect, RGB(227, 227, 227), RGB(227, 227, 227));
}

BOOL IRStateListView::OnEraseBkgnd(CDC *pDC) {
  return TRUE;
}

void IRStateListView::SetStates(const IRState *states) {
  states_ = states;

  if (::IsWindow(m_hWnd)) {
    Invalidate(FALSE);
  }
}