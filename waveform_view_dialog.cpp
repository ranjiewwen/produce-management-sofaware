// waveform_view_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "waveform_view_dialog.h"
#include "graphics.h"
#include "gui_resources.h"
#include "run_cash_monitor.h"
#include "run_cash_monitor_inl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// WaveformViewDialog dialog

IMPLEMENT_DYNAMIC(WaveformViewDialog, CDialog)

WaveformViewDialog::WaveformViewDialog(RunCashMonitor *monitor, int cashIndex, int channel, CWnd* pParent /*=NULL*/)
	: CDialog(WaveformViewDialog::IDD, pParent)
  , monitor_(monitor)
  , cashIndex_(cashIndex)
  , channel_(channel) {
}

WaveformViewDialog::~WaveformViewDialog() {
}

void WaveformViewDialog::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_STATIC_TITLE, labelTitle_);
  DDX_Control(pDX, IDC_BUTTON_CLOSE, closeButton_);
}

BOOL WaveformViewDialog::OnInitDialog() {
  CDialog::OnInitDialog();

  labelTitle_.SetFont(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL), FALSE);
  labelTitle_.SetBkColor(RGB(54, 133, 214));
  labelTitle_.SetTextColor(RGB(255, 255, 255));

  ButtonStyle buttonStyle;
  
  // close button style
  buttonStyle.bgColor = RGB(54, 133, 214);
  buttonStyle.textColor = RGB(255, 255, 255);
  buttonStyle.bitmap.LoadFromResource(IDB_BUTTON_CLOSE);
  buttonStyle.bitmap.SetWithAlpha();
  buttonStyle.bitmap.PreMultiplyAlpha();
  closeButton_.SetStyle(BUTTON_STATE_NORMAL, buttonStyle);
  buttonStyle.bgColor = RGB(255, 64, 64);
  closeButton_.SetStyle(BUTTON_STATE_OVER, buttonStyle);
  buttonStyle.bgColor = RGB(186, 0, 0);
  closeButton_.SetStyle(BUTTON_STATE_DOWN, buttonStyle);

  layout_.Init(m_hWnd);
  layout_.AddDlgItem(IDC_PLACE_CAPTION, AnchorLayout::TOP_LEFT, AnchorLayout::TOP_RIGHT);
  layout_.AddDlgItem(IDC_BUTTON_CLOSE, AnchorLayout::TOP_RIGHT, AnchorLayout::TOP_RIGHT);
  layout_.AddDlgItem(IDC_PLACE_WAVEFORM, AnchorLayout::TOP_LEFT, AnchorLayout::BOTTOM_RIGHT);
  
  ShowWindow(SW_MAXIMIZE);

  SelectCash(cashIndex_);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

int WaveformViewDialog::GetSelectedCashIndex() const {
  return cashIndex_;
}

void WaveformViewDialog::SelectCash(int index) {
  cashIndex_ = index;

  Invalidate();
}

BEGIN_MESSAGE_MAP(WaveformViewDialog, CDialog)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_BUTTON_CLOSE, &WaveformViewDialog::OnBnClickedButtonClose)
  ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// WaveformViewDialog message handlers
void WaveformViewDialog::OnPaint() {
  BufferredPaintDC dc(this);
  
  CRect rectTitle;
  GetDlgItem(IDC_PLACE_CAPTION)->GetWindowRect(&rectTitle);
  ScreenToClient(&rectTitle);
  dc.FillSolidRect(rectTitle, RGB(54, 133, 214));

  CRect waveformRect;
  GetDlgItem(IDC_PLACE_WAVEFORM)->GetWindowRect(&waveformRect);
  ScreenToClient(&waveformRect);
  dc.FillSolidRect(waveformRect, RGB(0, 0, 0));

  CFont *oldFont = dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL));
  dc.SetTextColor(RGB(255, 255, 255));
  dc.SetBkMode(TRANSPARENT);
  CSize textSize = dc.GetTextExtent(_T("88888"));
  waveformRect.left += textSize.cx + 8;

  if (monitor_ != NULL && cashIndex_ != -1) {
    int cashCount = monitor_->Freeze();
    if (cashIndex_ < cashCount) {
      Cash *cash = monitor_->GetCash(cashIndex_);
      CPen penYellow(PS_SOLID, 1, RGB(255, 255, 0));
      CPen penGreen(PS_SOLID, 1, RGB(0, 255, 0));
      int count = cash->GetADCSampleCount(g_ADCChannels[channel_].id);      
      if (count > 0) {
        CString title;
        title.Format(_T("%s - %s"), (LPCTSTR)CA2T(cash->GetSN()), g_ADCChannels[channel_].name);
        labelTitle_.SetWindowText(title);

        const short *codes = cash->GetADCSampleCodes(g_ADCChannels[channel_].id);
        const short *values = cash->GetADCSampleValues(g_ADCChannels[channel_].id);
        
        // draw scaler
        __int64 sum = 0;
        short minValue = SHORT_MAX, maxValue = SHORT_MIN;
        for (int i = 0; i < count; i++) {
          if (values[i] < minValue) {
            minValue = values[i];
          }
          if (values[i] > maxValue) {
            maxValue = values[i];
          }
          sum += values[i];
        }
        CPen whitePen(PS_SOLID, 1, RGB(255, 255, 255));
        CPen *oldPen = dc.SelectObject(&whitePen);
        dc.MoveTo(waveformRect.left, waveformRect.top);
        dc.LineTo(waveformRect.left, waveformRect.bottom);
        
        int height = waveformRect.Height();
        int range = g_ADCChannels[channel_].maxValue - g_ADCChannels[channel_].minValue;
        // min
        CString label;
        int x = waveformRect.left - textSize.cx - 8;
        int y = waveformRect.bottom - (minValue - g_ADCChannels[channel_].minValue) * height / range;
        dc.MoveTo(waveformRect.left - 4, y);
        dc.LineTo(waveformRect.right, y);
        label.Format(_T("%d"), minValue);
        dc.TextOut(x, y - textSize.cy / 2, label);
        // center
        short median = (short)(sum / count);
        y = waveformRect.bottom - (median - g_ADCChannels[channel_].minValue) * height / range;
        dc.MoveTo(waveformRect.left - 4, y);
        dc.LineTo(waveformRect.right, y);
        label.Format(_T("%d"), median);
        dc.TextOut(x, y - textSize.cy / 2, label);
        // max
        y = waveformRect.bottom - (maxValue - g_ADCChannels[channel_].minValue) * height / range;
        dc.MoveTo(waveformRect.left - 4, y);
        dc.LineTo(waveformRect.right, y);
        label.Format(_T("%d"), maxValue);
        dc.TextOut(x, y - textSize.cy / 2, label);

        dc.SelectObject(oldPen);

        ::DrawWaveform(&dc, waveformRect, codes, codes[0], codes[count - 1], count, &penYellow);
        ::DrawWaveform(&dc, waveformRect, values, 
            g_ADCChannels[channel_].minValue, 
            g_ADCChannels[channel_].maxValue, count, &penGreen);
      }
    }
    monitor_->Unfreeze();
  }

  dc.SelectObject(oldFont);
}

BOOL WaveformViewDialog::OnEraseBkgnd(CDC* pDC) {
  return TRUE;
}

void WaveformViewDialog::OnBnClickedButtonClose() {
  PostMessage(WM_CLOSE);
}

void WaveformViewDialog::OnSize(UINT nType, int cx, int cy) {
  CDialog::OnSize(nType, cx, cy);

  if (::IsWindow(m_hWnd)) {
    layout_.RecalcLayout();
  }
}

void WaveformViewDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
  CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL WaveformViewDialog::PreTranslateMessage(MSG* pMsg) {
  if (pMsg->message == WM_KEYDOWN) {
    UINT key = pMsg->wParam;
    if (key == VK_DOWN || key == VK_UP || key == VK_LEFT || key == VK_RIGHT || key == VK_NEXT || key == VK_PRIOR || key == VK_SPACE) {
      if (monitor_ != NULL && cashIndex_ != -1) {
        int cashCount = monitor_->Freeze();
        int cashIndex = cashIndex_;
        if (key == VK_DOWN || key == VK_RIGHT || key == VK_NEXT || key == VK_SPACE) {
          cashIndex++;
        } else if (key == VK_UP || key == VK_LEFT || key == VK_PRIOR) {
          cashIndex--;
        }
        if (cashIndex < 0) {
          cashIndex = 0;
        }
        if (cashIndex >= cashCount) {
          cashIndex = cashCount - 1;
        }
        if (cashIndex != cashIndex_) {
          cashIndex_ = cashIndex;

          Invalidate();
        }
        monitor_->Unfreeze();
      }
    }
  }

  return CDialog::PreTranslateMessage(pMsg);
}
