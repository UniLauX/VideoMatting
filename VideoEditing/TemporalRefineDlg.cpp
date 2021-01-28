// TemporalRefineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TemporalRefineDlg.h"


// CTemporalRefineDlg dialog

IMPLEMENT_DYNAMIC(CTemporalRefineDlg, CDialog)

CTemporalRefineDlg::CTemporalRefineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTemporalRefineDlg::IDD, pParent)
	, m_startFrame(0)
	, m_endFrame(0)
{
}

CTemporalRefineDlg::CTemporalRefineDlg(CString _layerName, int sf, int ef, CWnd* pParent)
:CDialog(CTemporalRefineDlg::IDD, pParent)
,m_layerName(_layerName)
,m_startFrame(sf)
,m_endFrame(ef)
,m_use_global_flow(false)
{

}

CTemporalRefineDlg::~CTemporalRefineDlg()
{
}

void CTemporalRefineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_OPTICAL_GLOBAL, m_check_optical_sel);
	DDX_Text(pDX, IDC_EDIT_START_FRAME_REFINE, m_startFrame);
	DDX_Text(pDX, IDC_EDIT_END_FRAME_REFINE, m_endFrame);
	DDX_Text(pDX, IDC_EDIT_LAYER_NAME, m_layerName);
}


BEGIN_MESSAGE_MAP(CTemporalRefineDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_OPTICAL_GLOBAL, &CTemporalRefineDlg::OnBnClickedCheckOpticalGlobal)
END_MESSAGE_MAP()


// CTemporalRefineDlg message handlers

void CTemporalRefineDlg::OnBnClickedCheckOpticalGlobal()
{
	// TODO: Add your control notification handler code here
	if(m_check_optical_sel.GetCheck())
	{
		m_use_global_flow = true;
	}
	else
		m_use_global_flow = false;
}
