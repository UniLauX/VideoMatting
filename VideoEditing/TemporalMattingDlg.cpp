// TemporalMattingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TemporalMattingDlg.h"


// CTemporalMattingDlg dialog

IMPLEMENT_DYNAMIC(CTemporalMattingDlg, CDialog)

CTemporalMattingDlg::CTemporalMattingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTemporalMattingDlg::IDD, pParent)
	, m_startFrame(0)
	, m_endFrame(0)
	, m_step(0)
	, m_spatialWeight(0)
	, m_temporalWeight(0)
	, m_bAnchorFirst(FALSE)
	, m_bAnchorLast(FALSE)
{
}

CTemporalMattingDlg::CTemporalMattingDlg(CString layername, int sf, int ef, int step, float sw, float tw, CWnd* pParent)
										 :CDialog(CTemporalMattingDlg::IDD, pParent)
										 ,m_layerName(layername)
										 ,m_startFrame(sf)
										 ,m_endFrame(ef)
										 ,m_step(step)
										 ,m_spatialWeight(sw)
										 ,m_temporalWeight(tw)
{

}

CTemporalMattingDlg::~CTemporalMattingDlg()
{
}

void CTemporalMattingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TM_LAYERNAME, m_layerName);
	DDX_Text(pDX, IDC_EDIT_TM_SFRAME, m_startFrame);
	DDX_Text(pDX, IDC_EDIT_TM_EFRAME, m_endFrame);
	DDX_Text(pDX, IDC_EDIT_TM_STEP, m_step);
	DDX_Text(pDX, IDC_EDIT_TM_SWEIGHT, m_spatialWeight);
	DDX_Text(pDX, IDC_EDIT_TM_TEMWEIGHT, m_temporalWeight);
	DDX_Check(pDX, IDC_CHECK_ANCHOR_FIRST, m_bAnchorFirst);
	DDX_Check(pDX, IDC_CHECK_ANCHOR_LAST, m_bAnchorLast);
}


BEGIN_MESSAGE_MAP(CTemporalMattingDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_ANCHOR_FIRST, &CTemporalMattingDlg::OnBnClickedCheckAnchorFirst)
	ON_BN_CLICKED(IDC_CHECK_ANCHOR_LAST, &CTemporalMattingDlg::OnBnClickedCheckAnchorLast)
END_MESSAGE_MAP()


// CTemporalMattingDlg message handlers

void CTemporalMattingDlg::OnBnClickedCheckAnchorFirst()
{
	// TODO: Add your control notification handler code here
	UpdateData();
}

void CTemporalMattingDlg::OnBnClickedCheckAnchorLast()
{
	// TODO: Add your control notification handler code here
	UpdateData();
}
