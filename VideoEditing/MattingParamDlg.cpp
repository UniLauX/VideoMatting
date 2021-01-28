// MattingParamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MattingParamDlg.h"


// CMattingParamDlg dialog

IMPLEMENT_DYNAMIC(CMattingParamDlg, CDialog)

CMattingParamDlg::CMattingParamDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMattingParamDlg::IDD, pParent)
	, m_layerName(_T(""))
	, m_startFrame(0)
	, m_endFrame(0)
	, m_radio(0)
{
	m_algorithm = CLOSEDMAT;
}


CMattingParamDlg::CMattingParamDlg(CString _layername, int _sframe, int _endframe, CWnd* pParent/* =NULL */)
:CDialog(CMattingParamDlg::IDD, pParent)
, m_layerName(_layername)
,m_startFrame(_sframe)
, m_endFrame(_endframe)
,m_radio(0)
{
	m_algorithm = CLOSEDMAT;
}

CMattingParamDlg::~CMattingParamDlg()
{
}

void CMattingParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_ALGORITHM, m_algorithmType);
	DDX_Text(pDX, IDC_EDIT_MAT_LAYER_NAME, m_layerName);
	DDX_Text(pDX, IDC_EDIT_MAT_SFRAME, m_startFrame);
	DDX_Text(pDX, IDC_EDIT_MAT_EFRAME, m_endFrame);
	DDX_Radio(pDX, IDC_RADIO_MAT_PART, m_radio);
}


BEGIN_MESSAGE_MAP(CMattingParamDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_ALGORITHM, &CMattingParamDlg::OnCbnSelchangeComboAlgorithm)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_RADIO_MAT_PART, &CMattingParamDlg::OnBnClickedRadioMatPart)
	ON_BN_CLICKED(IDC_RADIO_MAT_WHOLE, &CMattingParamDlg::OnBnClickedRadioMatWhole)
END_MESSAGE_MAP()


// CMattingParamDlg message handlers

void CMattingParamDlg::OnCbnSelchangeComboAlgorithm()//监听下拉列表改变
{
	// TODO: Add your control notification handler code here
	int nindex = m_algorithmType.GetCurSel();
	CString strAlgorithm;
	m_algorithmType.GetLBText(nindex, strAlgorithm);
	if (strAlgorithm==CString("Closed Form Matting"))
	{
		m_algorithm = CLOSEDMAT;
	}
}

int CMattingParamDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO:  Add your specialized creation code here
	m_algorithmType.AddString("Closed Form Matting");
	m_algorithmType.SelectString(0, "Closed Form Matting");
	return 0;
}

void CMattingParamDlg::OnBnClickedRadioMatPart()
{
	// TODO: Add your control notification handler code here
	m_radio =0;
}

void CMattingParamDlg::OnBnClickedRadioMatWhole()
{
	// TODO: Add your control notification handler code here
	m_radio = 1;
}
