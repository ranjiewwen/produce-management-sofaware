#include "stdafx.h"
#include "about_dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

AboutDialog::AboutDialog() : CDialog(AboutDialog::IDD)
{
}

void AboutDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(AboutDialog, CDialog)
END_MESSAGE_MAP()