// RotoScopeParam.cpp : implementation file
//

#include "stdafx.h"
#include "RotoScopeParam.h"
#include "VideoEditing.h"
#include "VideoEditingView.h"

// RotoScopeParam dialog

int index = 0;

//IMPLEMENT_DYNAMIC(RotoScopeParam, CDialog)
IMPLEMENT_DYNCREATE(RotoScopeParam, CDialog)

RotoScopeParam::RotoScopeParam(CWnd* pParent /*=NULL*/)
	: CDialog(RotoScopeParam::IDD, pParent)
	, kup(5)
	, klow(-5)
	, startFrameIndex(0)
	, endFrameIndex(9)
	, curruntFrameIndex(0)
	, pyramidLevel(1)
	, startIndex(0)
	, endIndex(0)
	, bmin(0.3)
	, bmax(0.7)
	, lb(3)
	, ls(7)
{

	
}

RotoScopeParam::~RotoScopeParam()
{
}

void RotoScopeParam::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_KU, kup);
	DDX_Text(pDX, IDC_KL, klow);
	DDX_Text(pDX, IDC_STARTINDEX, startFrameIndex);
	DDX_Text(pDX, IDC_ENDINDEX, endFrameIndex);
	DDX_Text(pDX, IDC_EDIT_FRAMEINDEX, curruntFrameIndex);
	DDX_Text(pDX, IDC_EDIT_FRAMETOTAL, totalFrameNum);
	DDX_Text(pDX, IDC_EDIT_PYR_LEVEL, pyramidLevel);
	DDX_Text(pDX, IDC_SAVE_START, startIndex);
	DDX_Text(pDX, IDC_SAVE_END, endIndex);
	DDX_Text(pDX, IDC_EDIT_BMIN, bmin);
	DDX_Text(pDX, IDC_EDIT_BMAX, bmax);
	DDX_Text(pDX, IDC_EDIT_LB, lb);
	DDX_Text(pDX, IDC_EDIT_LS, ls);
}


BEGIN_MESSAGE_MAP(RotoScopeParam, CDialog)
	ON_EN_CHANGE(IDC_KL, &RotoScopeParam::OnEnChangeKlow)
	ON_EN_CHANGE(IDC_KU, &RotoScopeParam::OnEnChangeKu)
	ON_EN_CHANGE(IDC_STARTINDEX, &RotoScopeParam::OnEnChangeStartindex)
	ON_EN_CHANGE(IDC_ENDINDEX, &RotoScopeParam::OnEnChangeEndindex)
	ON_EN_CHANGE(IDC_EDIT_PYR_LEVEL, &RotoScopeParam::OnEnChangeEditPyrLevel)
	ON_BN_CLICKED(IDC_ROTOSCOPE_SAVEMASK, &RotoScopeParam::OnBnClickedSaveMask)
	ON_EN_CHANGE(IDC_SAVE_START, &RotoScopeParam::OnEnChangeSaveStart)
	ON_EN_CHANGE(IDC_SAVE_END, &RotoScopeParam::OnEnChangeSaveEnd)
	ON_EN_CHANGE(IDC_EDIT_BMIN, &RotoScopeParam::OnEnChangeEditBmin)
	ON_EN_CHANGE(IDC_EDIT_BMAX, &RotoScopeParam::OnEnChangeEditBmax)
	ON_EN_CHANGE(IDC_EDIT_LB, &RotoScopeParam::OnEnChangeEditLb)
	ON_EN_CHANGE(IDC_EDIT_LS, &RotoScopeParam::OnEnChangeEditLs)
	ON_BN_CLICKED(IDC_CHECk_PREVIEW, &RotoScopeParam::OnBnClickedCheckPreview)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &RotoScopeParam::OnBnClickedButtonImport)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, &RotoScopeParam::OnBnClickedButtonExport)
END_MESSAGE_MAP()


CVideoEditingDoc* RotoScopeParam::getDoc()
{
	CFrameWnd* pFrame=(CFrameWnd*)(AfxGetApp()->m_pMainWnd);
	return (CVideoEditingDoc*)pFrame->GetActiveDocument();
}

CVideoEditingView* RotoScopeParam::getView()
{
	CFrameWnd* pFrame=(CFrameWnd*)(AfxGetApp()->m_pMainWnd);
	return dynamic_cast<CVideoEditingView*>(pFrame->GetActiveView());
}


// RotoScopeParam message handlers



afx_msg void RotoScopeParam::OnEnChangeKlow()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	klow = min(0, klow);
	UpdateData(FALSE);
	CVideoEditingDoc * doc = getDoc();
	doc->setRotoScopeParameter(kup, klow, startFrameIndex, endFrameIndex, pyramidLevel);
}

void RotoScopeParam::OnEnChangeKu()
{
	//UpdateData(TRUE);
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	kup = max(0, kup);
	UpdateData(FALSE);
	CVideoEditingDoc * doc = getDoc();
	doc->setRotoScopeParameter(kup, klow, startFrameIndex, endFrameIndex, pyramidLevel);
}

void RotoScopeParam::OnEnChangeStartindex()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	CVideoEditingDoc * doc = getDoc();
	doc->setRotoScopeParameter(kup, klow, startFrameIndex, endFrameIndex, pyramidLevel);
}

void RotoScopeParam::OnEnChangeEndindex()
{
	UpdateData(TRUE);
	CVideoEditingDoc * doc = getDoc();
	doc->setRotoScopeParameter(kup, klow, startFrameIndex, endFrameIndex, pyramidLevel);
}

void RotoScopeParam::OnEnChangeEditPyrLevel()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	CVideoEditingDoc * doc = getDoc();
	doc->setRotoScopeParameter(kup, klow, startFrameIndex, endFrameIndex, pyramidLevel);
}


BOOL RotoScopeParam::OnInitDialog()
{
	CDialog::OnInitDialog();
	UpdateData(FALSE);
	this->SetScrollRange(SB_VERT, 0, 100, TRUE);
	this->SetScrollPos(SB_VERT,0);

	return true;
}

BOOL RotoScopeParam::PreTranslateMessage(MSG* pMsg)
{
	//printf("%d \n", index++);
	CVideoEditingView * pView = getView();
	curruntFrameIndex = pView->imageBuffer.GetFramePos() - pView->getStartIndex();
	totalFrameNum = pView->fileRange - 1;
	UpdateData(FALSE);
	
	return CDialog::PreTranslateMessage(pMsg);
	
}



void RotoScopeParam::OnBnClickedSaveMask()
{
	// TODO: Add your control notification handler code here
	CVideoEditingView * pView = getView();
	pView->saveMaskImage(startIndex, endIndex, lb, ls, bmin, bmax);
	//cout << lb << " " << ls << " " << bmin << " " << bmax << endl;
	
}

void RotoScopeParam::OnEnChangeSaveStart()
{
	
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	//cout << startIndex << endl;
}

void RotoScopeParam::OnEnChangeSaveEnd()
{
	
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	//cout << endIndex << endl;
}

void RotoScopeParam::OnEnChangeEditBmin()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
}

void RotoScopeParam::OnEnChangeEditBmax()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
}

void RotoScopeParam::OnEnChangeEditLb()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
}

void RotoScopeParam::OnEnChangeEditLs()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
}

void RotoScopeParam::OnBnClickedCheckPreview()
{
	// TODO: Add your control notification handler code here
}

void RotoScopeParam::OnBnClickedButtonImport()
{
	// TODO: Add your control notification handler code here
	CVideoEditingView * pView = getView();
	pView->RotoScopingCurveImport();
}

void RotoScopeParam::OnBnClickedButtonExport()
{
	// TODO: Add your control notification handler code here
	CVideoEditingView * pView = getView();
	pView->RotoScopingCurveExport();
}
