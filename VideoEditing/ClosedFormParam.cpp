// ClosedFormParam.cpp : implementation file
//

#include "stdafx.h"
#include "VideoEditing.h"
#include "ClosedFormParam.h"
#include "VideoEditingView.h"


// CClosedFormParam dialog

//IMPLEMENT_DYNAMIC(CClosedFormParam, CDialog)
IMPLEMENT_DYNCREATE(CClosedFormParam, CDialog)

CClosedFormParam::CClosedFormParam(CWnd* pParent /*=NULL*/)
	: CDialog(CClosedFormParam::IDD, pParent)
	, level(1)
	, activeLevel(1)
	, winSize(1)
	, alphaThreshold(0.02)
	, epsilon(0.00000001)
	, m_scrollbar_value(0)
	,prePos(0)
	, m_radio(0)
	, m_fTriMpAlpha(0.1)
{
	//getDoc()->GetCFParameter(level,activeLevel,winSize,alphaThreshold,epsilon);
}

CClosedFormParam::~CClosedFormParam()
{
}

void CClosedFormParam::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CF_LEVEL, level);
	DDX_Text(pDX, IDC_EDIT_CF_ACTIVELEVEL, activeLevel);
	DDX_Text(pDX, IDC_EDIT_CF_WINSIZE, winSize);
	DDX_Text(pDX, IDC_EDIT_CF_THRESHOLD, alphaThreshold);
	DDX_Text(pDX, IDC_EDIT_CF_EPSILON, epsilon);
	DDX_Text(pDX, IDC_EDIT_TRIMAPALPHA, m_fTriMpAlpha);

	DDX_Control(pDX, IDC_COMBO_ALGO, m_algorithm);
	DDX_Radio(pDX, IDC_RADIO_IMPART, m_radio);
	DDX_Control(pDX, IDC_CHECK_FRAME_MAT, m_checkBoxframeMat);
	DDX_Control(pDX, IDC_SLIDER_TRIMAPALPHA, m_ctrlTriMpAlpha);
}
BOOL CClosedFormParam::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message==WM_KEYDOWN)
	{
		if (pMsg->wParam==VK_RETURN)
		{
			//MessageBox("Enter");
			UpdateData(TRUE);
			CVideoEditingDoc* doc=getDoc();
			doc->SetParameter(level,activeLevel,winSize,alphaThreshold,epsilon);
			
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

CVideoEditingDoc* CClosedFormParam::getDoc()
{
	CFrameWnd* pFrame=(CFrameWnd*)(AfxGetApp()->m_pMainWnd);
	return (CVideoEditingDoc*)pFrame->GetActiveDocument();
}

CVideoEditingView* CClosedFormParam::GetView()
{
	CFrameWnd* pFrame=(CFrameWnd*)(AfxGetApp()->m_pMainWnd);
	return dynamic_cast<CVideoEditingView*>(pFrame->GetActiveView());
}

BOOL CClosedFormParam::OnInitDialog()
{
	UpdateData(false);
	this->SetScrollRange(SB_VERT,0,100,TRUE);
	this->SetScrollPos(SB_VERT,0);
	m_algorithm.AddString("Closed Form Algo");
	m_algorithm.SelectString(0, "Closed Form Algo");

	m_checkBoxframeMat.SetCheck(false);
	GetDlgItem(IDC_RADIO_IMPART)->EnableWindow(m_checkBoxframeMat.GetCheck());
	GetDlgItem(IDC_RADIO_IMGWHOLE)->EnableWindow(m_checkBoxframeMat.GetCheck());
	GetDlgItem(IDC_COMBO_ALGO)->EnableWindow(m_checkBoxframeMat.GetCheck());

	m_ctrlTriMpAlpha.SetRange(0, 100);
	m_ctrlTriMpAlpha.SetPos(10);

	return true;
}
BEGIN_MESSAGE_MAP(CClosedFormParam, CDialog)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_RADIO_IMPART, &CClosedFormParam::OnBnClickedRadioImpart)
	ON_BN_CLICKED(IDC_RADIO_IMGWHOLE, &CClosedFormParam::OnBnClickedRadioImgwhole)
	ON_CBN_SELCHANGE(IDC_COMBO_ALGO, &CClosedFormParam::OnCbnSelchangeComboAlgo)
	ON_BN_CLICKED(IDC_CHECK_FRAME_MAT, &CClosedFormParam::OnBnClickedCheckFrameMat)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TRIMAPALPHA, &CClosedFormParam::OnNMCustomdrawSliderTrimapAlpha)
END_MESSAGE_MAP()

void CClosedFormParam::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
		int pos=this->GetScrollPos(SB_VERT);
		switch (nSBCode)
		{
			case  SB_LINEUP: pos-=1;break;
			case  SB_LINEDOWN: pos+=1;break;
			case  SB_PAGEDOWN:pos+=10;break;
			case  SB_PAGEUP: pos-=10;break;
			case  SB_THUMBTRACK: pos=nPos;break;
			default:break;
		}
		ScrollWindow(0,2*(prePos-pos),NULL,NULL);
		this->SetScrollPos(SB_VERT,pos,true);
		prePos=pos;
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}
void CClosedFormParam::OnBnClickedRadioImpart()//image partition
{
	// TODO: Add your control notification handler code here
	m_radio = 0;
	getDoc()->mattingMode = MAT_PARTIMG;
	printf("radio: %d \n", m_radio);
}

void CClosedFormParam::OnBnClickedRadioImgwhole()
{
	// TODO: Add your control notification handler code here
	m_radio = 1;
	getDoc()->mattingMode = MAT_WHOLEIMG;
	printf("radio: %d \n", m_radio);
}

void CClosedFormParam::OnCbnSelchangeComboAlgo()
{
	// TODO: Add your control notification handler code here
	int nindex = m_algorithm.GetCurSel();
	CString strAlgorithm;
	m_algorithm.GetLBText(nindex, strAlgorithm);
	if (strAlgorithm==CString("Closed Form Algo"))
		getDoc()->mattingAlgo = CLOSEDMAT;
}

void CClosedFormParam::OnBnClickedCheckFrameMat()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_RADIO_IMPART)->EnableWindow(m_checkBoxframeMat.GetCheck());
	GetDlgItem(IDC_RADIO_IMGWHOLE)->EnableWindow(m_checkBoxframeMat.GetCheck());
	GetDlgItem(IDC_COMBO_ALGO)->EnableWindow(m_checkBoxframeMat.GetCheck());
	if (m_checkBoxframeMat.GetCheck())
	{
		getDoc()->mattingType = MAT_FRAME;
	}
	else
		getDoc()->mattingType = MAT_SEQ;
}

void CClosedFormParam::OnNMCustomdrawSliderTrimapAlpha(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_fTriMpAlpha = float(m_ctrlTriMpAlpha.GetPos()) / m_ctrlTriMpAlpha.GetRangeMax();
	UpdateData(FALSE);

	CVideoEditingView* pView = GetView();
	if(pView)
	{
		pView->m_fTriMpAlpha = m_fTriMpAlpha;
		if(pView->m_TriMpVwMode == CVideoEditingView::TRIMAP)
			pView->RecompositeTrimap();
		pView->Invalidate(FALSE);
	}

	*pResult = 0;
}
