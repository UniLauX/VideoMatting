// TrimapWidthSetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TrimapWidthSetDlg.h"


// CTrimapWidthSetDlg dialog

IMPLEMENT_DYNAMIC(CTrimapWidthSetDlg, CDialog)

CTrimapWidthSetDlg::CTrimapWidthSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTrimapWidthSetDlg::IDD, pParent)
	, m_iTrimapWidth(2)
{

}

CTrimapWidthSetDlg::CTrimapWidthSetDlg(const CString& title ,CWnd* pParent /* = NULL */ )
:CDialog(CTrimapWidthSetDlg::IDD, pParent)
, m_iTrimapWidth(2)
{
}
CTrimapWidthSetDlg::~CTrimapWidthSetDlg()
{
}
void CTrimapWidthSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX,IDC_EDIT_WIDTH, m_iTrimapWidth);
}

int CTrimapWidthSetDlg::GetTrimapWidth()
{
	return m_iTrimapWidth;
}


BEGIN_MESSAGE_MAP(CTrimapWidthSetDlg, CDialog)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CTrimapWidthSetDlg message handlers

int CTrimapWidthSetDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	this->SetWindowText("Trimap Width");
	return 0;
}
