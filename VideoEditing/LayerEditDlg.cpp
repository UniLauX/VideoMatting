// LayerEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VideoEditing.h"
#include "LayerEditDlg.h"
#include "../MultiLayer/LayerObject.h"

// CLayerEditDlg dialog

IMPLEMENT_DYNAMIC(CLayerEditDlg, CDialog)
static const char* type[]={"static plane","static rigid", "plane motion", "rigid motion", "nonrigid motion"};

CLayerEditDlg::CLayerEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLayerEditDlg::IDD, pParent)
	, LayerName(_T(""))
	, layerDepth(0)
	,layerType(-1)
{
}

CLayerEditDlg::~CLayerEditDlg()
{
}

void CLayerEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX,IDC_NAME_EDIT,LayerName);
	//DDX_Text(pDX,IDC_DEPTH_EDIT,layerDepth);
	DDX_Control(pDX, IDC_COMBO_LAYERTYPE, m_LayerTypeMFC);
}

int CLayerEditDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct)==-1)
	{
		return -1;
	}
	//m_LayerTypeMFC.AddString("static plane");//0
	//m_LayerTypeMFC.SelectString(0, "static plane");
	//m_LayerTypeMFC.AddString("static rigid");//1
	//m_LayerTypeMFC.AddString("plane motion");//2
	//m_LayerTypeMFC.AddString("rigid motion");//3
	//m_LayerTypeMFC.AddString("nonrigid motion");//4
	for (int i=0; i<5; ++i)
	{
		m_LayerTypeMFC.AddString(type[i]);
	}
	return 0;
}
CString CLayerEditDlg::GetLayerName()
{
	return LayerName;
}

int CLayerEditDlg::GetLayerDepth()
{
	return layerDepth;
}

int CLayerEditDlg::GetLayerType()
{
	return layerType;
}
void CLayerEditDlg::SetLayerNameDepth(CString name,int depth)
{
	this->LayerName=name;
	this->layerDepth=depth;
}
BEGIN_MESSAGE_MAP(CLayerEditDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_LAYERTYPE, &CLayerEditDlg::OnCbnSelchangeComboLayertype)
END_MESSAGE_MAP()


// CLayerEditDlg message handlers

void CLayerEditDlg::OnCbnSelchangeComboLayertype()
{
	// TODO: Add your control notification handler code here
	int nindex = m_LayerTypeMFC.GetCurSel();
	CString strType;
	m_LayerTypeMFC.GetLBText(nindex, strType);
	if (strType==CString("static plane"))
		layerType = STATIC_PLANE;
	else if (strType==CString("static rigid"))
		layerType = STATIC_RIGID;
	else if(strType==CString("plane motion"))
		layerType = PLANE_MOTION;
	else if(strType==CString("rigid motion"))
		layerType = RIGID_MOTION;
	else if(strType==CString("nonrigid motion"))
		layerType = NONRIGID_MOTION;
	printf("%d %s\n",layerType, type[layerType] );
}
