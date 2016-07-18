// image_view_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "image_view_dialog.h"
#include "graphics.h"
#include "gui_resources.h"
#include "run_cash_monitor.h"
#include "run_cash_monitor_inl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ImageViewDialog dialog

IMPLEMENT_DYNAMIC(ImageViewDialog, CDialog)

ImageViewDialog::ImageViewDialog(RunCashMonitor *monitor, int cashIndex, CWnd* pParent /*=NULL*/)
	: CDialog(ImageViewDialog::IDD, pParent)
  , monitor_(monitor)
  , cashIndex_(cashIndex) {
}

ImageViewDialog::~ImageViewDialog() {
}

void ImageViewDialog::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_STATIC_TITLE, labelTitle_);
  DDX_Control(pDX, IDC_BUTTON_CLOSE, closeButton_);
}

BOOL ImageViewDialog::OnInitDialog() {
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
  layout_.AddDlgItem(IDC_PLACE_IMAGE, AnchorLayout::TOP_LEFT, AnchorLayout::BOTTOM_RIGHT);
  
  ShowWindow(SW_MAXIMIZE);

  SelectCash(cashIndex_);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void ImageViewDialog::DrawImage(CDC *dc, Bitmap *image, const CRect &rect) {
  if (!image->IsValid()) {
    return;
  }
  double scaleX = (double)rect.Width() / image->GetWidth();
  double scaleY = (double)rect.Height() / image->GetHeight();
  double scale = __min(__min(scaleX, scaleY), 1.0);
  int dstWidth = (int)(scale * image->GetWidth());
  int dstHeight = (int)(scale * image->GetHeight());
  int left = rect.left + (rect.Width() - dstWidth) / 2;
  int top = rect.top + (rect.Height() - dstHeight) / 2;
  image->Draw(dc->GetSafeHdc(), CRect(left, top, left + dstWidth, top + dstHeight));
}

int ImageViewDialog::GetSelectedCashIndex() const {
  return cashIndex_;
}

void ImageViewDialog::SelectCash(int index) {
  cashIndex_ = index;

  if (monitor_ != NULL && cashIndex_ != -1) {
    int cashCount = monitor_->Freeze();
    if (cashIndex_ < cashCount) {
      Cash *cash = monitor_->GetCash(cashIndex_);        
      // draw image.
      Bitmap *topImage = cash->GetTopImage();          
      Bitmap *bottomImage = cash->GetBottomImage();
      CString title;
      title.Format(IDS_VIEW_CASH_IMAGE, CA2T(cash->GetSN()));
      labelTitle_.SetWindowText(title);
      ASSERT(topImage != NULL && bottomImage != NULL);
      topImage_ = *topImage;
      bottomImage_ = *bottomImage;
    }
    monitor_->Unfreeze();
  }
  
  Invalidate();
}

BEGIN_MESSAGE_MAP(ImageViewDialog, CDialog)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_BUTTON_CLOSE, &ImageViewDialog::OnBnClickedButtonClose)
  ON_WM_KEYDOWN()
  ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()


// ImageViewDialog message handlers
void ImageViewDialog::OnPaint() {
  BufferredPaintDC dc(this);
  
  CRect rectTitle;
  GetDlgItem(IDC_PLACE_CAPTION)->GetWindowRect(&rectTitle);
  ScreenToClient(&rectTitle);
  dc.FillSolidRect(rectTitle, RGB(54, 133, 214));

  CRect imageRect;
  GetDlgItem(IDC_PLACE_IMAGE)->GetWindowRect(&imageRect);
  ScreenToClient(&imageRect);
  dc.FillSolidRect(imageRect, RGB(0, 0, 0));

  CRect topRect(imageRect.left, imageRect.top, 
      imageRect.right, imageRect.top + imageRect.Height() / 2);
  CRect bottomRect(imageRect.left, topRect.bottom, imageRect.right, imageRect.bottom);
  dc.SetStretchBltMode(HALFTONE);
  DrawImage(&dc, &topImage_, topRect);
  DrawImage(&dc, &bottomImage_, bottomRect);
}

BOOL ImageViewDialog::OnEraseBkgnd(CDC* pDC) {
  return TRUE;
}

void ImageViewDialog::OnBnClickedButtonClose() {
  PostMessage(WM_CLOSE);
}

void ImageViewDialog::OnSize(UINT nType, int cx, int cy) {
  CDialog::OnSize(nType, cx, cy);

  if (::IsWindow(m_hWnd)) {
    layout_.RecalcLayout();
  }
}

void ImageViewDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
  CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL ImageViewDialog::PreTranslateMessage(MSG* pMsg) {
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
          Cash *cash = monitor_->GetCash(cashIndex_);        
          // draw image.
          Bitmap *topImage = cash->GetTopImage();          
          Bitmap *bottomImage = cash->GetBottomImage();
          CString title;
          title.Format(IDS_VIEW_CASH_IMAGE, CA2T(cash->GetSN()));
          labelTitle_.SetWindowText(title);
          ASSERT(topImage != NULL && bottomImage != NULL);
          topImage_ = *topImage;
          bottomImage_ = *bottomImage;
          Invalidate();
        }
        monitor_->Unfreeze();
      }
    }
  }

  return CDialog::PreTranslateMessage(pMsg);
}


void ImageViewDialog::OnRButtonDown(UINT nFlags, CPoint point) {
  CMenu menu;
  menu.LoadMenu(IDR_POPUP_IMAGE_VIEW);
  CMenu *contextMenu = menu.GetSubMenu(0);
  int choice = contextMenu->TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);
  if (choice == ID_SAVE_IMAGE) {
    CFileDialog dlg(FALSE);
    if (dlg.DoModal() == IDOK) {
      CString filePath = dlg.GetPathName();
      topImage_.SaveToFile(filePath + "_top.bmp");
      bottomImage_.SaveToFile(filePath + "_bottom.bmp");
    }
  }

  CDialog::OnRButtonDown(nFlags, point);
}
