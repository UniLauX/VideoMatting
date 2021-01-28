// SysParam.cpp : implementation file
//

#include "stdafx.h"
#include "VideoEditing.h"
#include "SysParam.h"
#include "../VideoEditing/PolygonPaintTool.h"
using namespace VideoActiveTool;
// SysParam dialog

//IMPLEMENT_DYNAMIC(SysParam, CDialog)
IMPLEMENT_DYNCREATE(SysParam, CDialog)
 
SysParam::SysParam(CWnd* pParent /*=NULL*/)
	: CDialog(SysParam::IDD, pParent)
	, videoWinSize(80)
	, updateWinSize(25)
	, iterateTime(2)
	, cosThreshold(0.8f)
	, disThreshold(8)
	, lineDiff(0.5f)
	, pointRadius(2.0f)
{
}

SysParam::~SysParam()
{
}

void SysParam::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_VideoWinSize, videoWinSize);
	DDX_Text(pDX, IDC_EDIT_UpdateWinSize, updateWinSize);
	DDX_Text(pDX, IDC_EDIT_ITERATETIME, iterateTime);
	/*DDX_Text(pDX, IDC_EDIT_PolygonThreshold, threshold);*/
	DDX_Text(pDX, IDC_EDIT_CosThreshold, cosThreshold);
	DDX_Text(pDX, IDC_EDIT_DisThreshold, disThreshold);
	DDX_Text(pDX, IDC_EDIT_PolygonLineDiff, lineDiff);
	DDX_Text(pDX, IDC_EDIT_PolygonLineDiff, lineDiff);
	DDX_Text(pDX, IDC_EDIT_PolygonPointRadius, pointRadius);

	DDX_Control(pDX, IDC_SLIDER_BoundSize, boundCtr);
	DDX_Control(pDX, IDC_SLIDER_Cutoff, cutoffCtr);
	DDX_Control(pDX, IDC_SLIDER_Epsilon, eminCtr);
	DDX_Control(pDX, IDC_SLIDER_ForeThres, foreThresCtr);
	DDX_Control(pDX, IDC_SLIDER_BackThres, backThresCtr);
	DDX_Control(pDX, IDC_SLIDER_PolygonCosThreshold, costhresCtr);
	DDX_Control(pDX, IDC_SLIDER_PolygonDisThreshold, disthresCtr);
	
	DDX_Text(pDX, IDC_EDIT_BoundSize, param.boundDis);
	DDX_Text(pDX, IDC_EDIT_Cutoff, param.fcutoff);
	DDX_Text(pDX, IDC_EDIT_Epsilon, param.eMin);
	DDX_Text(pDX, IDC_EDIT_ForeThres, param.foreThres);
	DDX_Text(pDX, IDC_EDIT_BackThres, param.backThres);
	//DDX_Control(pDX, IDC_EDIT_OUT, editOut);
}

BOOL SysParam::OnInitDialog()
{
	printf("InitDialog\n");
	CDialog::OnInitDialog();
	boundCtr.SetRange(0, 100);
	boundCtr.SetPos(param.boundDis);
	cutoffCtr.SetRange(0, 100);
	cutoffCtr.SetPos(int(param.fcutoff*100));
	eminCtr.SetRange(0, 10);
	eminCtr.SetPos(int(param.eMin));
	foreThresCtr.SetRange(0, 100);
	foreThresCtr.SetPos(int(param.foreThres*100));
	backThresCtr.SetRange(0, 100);
	backThresCtr.SetPos(int(param.backThres*100));
	costhresCtr.SetRange( 0, 100 );
	costhresCtr.SetPos(int(cosThreshold*100));
	disthresCtr.SetRange( 0, 50 );
	disthresCtr.SetPos(int(disThreshold));
	UpdateData(FALSE);
	return true;

}

BEGIN_MESSAGE_MAP(SysParam, CDialog)
	/*ON_EN_CHANGE(IDC_EDIT_PolygonThreshold, &SysParam::OnEnChangeEditPolygonthreshold)*/
	ON_EN_CHANGE(IDC_EDIT_PolygonLineDiff, &SysParam::OnEnChangeEditPolygonlinediff)
	ON_EN_CHANGE(IDC_EDIT_PolygonPointRadius, &SysParam::OnEnChangeEditPolygonpointradius)
	ON_EN_CHANGE(IDC_EDIT_VideoWinSize, &SysParam::OnEnChangeEditVideowinsize)
	ON_EN_CHANGE(IDC_EDIT_UpdateWinSize, &SysParam::OnEnChangeEditUpdatewinsize)
	ON_EN_CHANGE(IDC_EDIT_ITERATETIME, &SysParam::OnEnChangeEditIteratetime)
	ON_BN_CLICKED(IDC_BUTTON_ResetPara, &SysParam::OnBnClickedButtonResetPara)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BoundSize, &SysParam::OnNMCustomdrawSliderBoundsize)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_Cutoff, &SysParam::OnNMCustomdrawSliderCutoff)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_Epsilon, &SysParam::OnNMCustomdrawSliderEpsilon)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_ForeThres, &SysParam::OnNMCustomdrawSliderForethres)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BackThres, &SysParam::OnNMCustomdrawSliderBackthres)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_PolygonCosThreshold, &SysParam::OnNMCustomdrawSliderPolygoncosthreshold)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_PolygonDisThreshold, &SysParam::OnNMCustomdrawSliderPolygondisthreshold)
END_MESSAGE_MAP()


// SysParam message handlers

CVideoEditingView* SysParam::getView()
{
	CFrameWnd* pFrame=(CFrameWnd*)(AfxGetApp()->m_pMainWnd);
	return (CVideoEditingView*)pFrame->GetActiveView();
}

CVideoEditingDoc* SysParam::getDoc()
{
	CFrameWnd* pFrame=(CFrameWnd*)(AfxGetApp()->m_pMainWnd);
	return (CVideoEditingDoc*)pFrame->GetActiveDocument();
}
BOOL SysParam::PreTranslateMessage(MSG* pMsg)
{

	return CDialog::PreTranslateMessage(pMsg);
}

//void SysParam::OnEnChangeEditPolygonthreshold()
//{
//	// TODO:  If this is a RICHEDIT control, the control will not
//	// send this notification unless you override the CDialog::OnInitDialog()
//	// function and call CRichEditCtrl().SetEventMask()
//	// with the ENM_CHANGE flag ORed into the mask.
//
//	// TODO:  Add your control notification handler code here
//	UpdateData(TRUE);
//	CPolygonPaintTool::m_fPolygonThreshold = threshold;
//	//UpdateData(FALSE);
//}

void SysParam::OnEnChangeEditPolygonlinediff()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	CPolygonPaintTool::m_fLineDiff = lineDiff;


	UpdateData(FALSE);
}

void SysParam::OnEnChangeEditPolygonpointradius()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	CPolygonPaintTool::m_iPointRadius = pointRadius;
	//UpdateData(FALSE);
}



void SysParam::OnEnChangeEditVideowinsize()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	CVideoEditingDoc* doc=getDoc();
	doc->SetVideoWinSize(videoWinSize);
	//UpdateData(FALSE);
}

void SysParam::OnEnChangeEditUpdatewinsize()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	CVideoEditingDoc* doc=getDoc();
	doc->SetUpdateWinSize(updateWinSize);
	//UpdateData(FALSE);
}

void SysParam::OnEnChangeEditIteratetime()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	CVideoEditingDoc* doc=getDoc();
	doc->SetIterateTime(iterateTime);
	//UpdateData(FALSE);
}

void SysParam::OnBnClickedButtonResetPara()
{
	// TODO: Add your control notification handler code here
	//UpdateData(TRUE);
	CVideoEditingDoc* doc=getDoc();
	param.boundDis=6;
	param.fcutoff=0.85;
	param.eMin=2;
	param.foreThres=0.75;
	param.backThres=0.25;
	cosThreshold = 0.8;
	disThreshold = 8;

	doc->SetLocalClassifierParam(param);
	boundCtr.SetPos(param.boundDis);
	cutoffCtr.SetPos(int(param.fcutoff*100));
	eminCtr.SetPos(int(param.eMin));
	foreThresCtr.SetPos(int(param.foreThres*100));
	backThresCtr.SetPos(int(param.backThres*100));
	costhresCtr.SetPos(int(cosThreshold*100));
	disthresCtr.SetPos(int(disThreshold));
	//CVideoEditingDoc* doc=getDoc();
	lineDiff = 0.5;
	pointRadius = 2;
	CPolygonPaintTool::m_fCosThreshold = cosThreshold;
	CPolygonPaintTool::m_fCosThreshold = disThreshold;
	CPolygonPaintTool::m_fLineDiff = lineDiff;
	CPolygonPaintTool::m_iPointRadius = pointRadius;

	videoWinSize=80;
	updateWinSize=25;
	iterateTime=3;
	doc->SetVideoWinSize(80);
	doc->SetUpdateWinSize(25);
	doc->SetIterateTime(3);

	UpdateData(false);
}

void SysParam::OnNMCustomdrawSliderBoundsize(NMHDR *pNMHDR, LRESULT *pResult)
{
	//printf("OnNMCustomdrawSliderBoundsize\n");
	//UpdateData(TRUE);
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	param.boundDis=boundCtr.GetPos();
	CVideoEditingDoc* doc=getDoc();
	doc->SetLocalClassifierParam(param);
	*pResult = 0;
	UpdateData(FALSE);
}

void SysParam::OnNMCustomdrawSliderCutoff(NMHDR *pNMHDR, LRESULT *pResult)
{
	//printf("OnNMCustomdrawSliderCutoff\n");
	//UpdateData(TRUE);
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	param.fcutoff=cutoffCtr.GetPos()/100.0;
	CVideoEditingDoc* doc=getDoc();
	doc->SetLocalClassifierParam(param);
	*pResult = 0;
	UpdateData(FALSE);
}

void SysParam::OnNMCustomdrawSliderEpsilon(NMHDR *pNMHDR, LRESULT *pResult)
{
	//printf("OnNMCustomdrawSliderEpsilon\n");
	//UpdateData(TRUE);
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	param.eMin=eminCtr.GetPos();
	CVideoEditingDoc* doc=getDoc();
	doc->SetLocalClassifierParam(param);
	*pResult = 0;
	UpdateData(FALSE);
}

void SysParam::OnNMCustomdrawSliderForethres(NMHDR *pNMHDR, LRESULT *pResult)
{
	//printf("OnNMCustomdrawSliderForethres\n");
	//UpdateData(TRUE);
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	param.foreThres=foreThresCtr.GetPos()/100.0;
	CVideoEditingDoc* doc=getDoc();
	doc->SetLocalClassifierParam(param);
	*pResult = 0;
	UpdateData(FALSE);
}

void SysParam::OnNMCustomdrawSliderBackthres(NMHDR *pNMHDR, LRESULT *pResult)
{
	//printf("OnNMCustomdrawSliderBackthres\n");
	//UpdateData(TRUE);
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	param.backThres=backThresCtr.GetPos()/100.0;
	CVideoEditingDoc* doc=getDoc();
	doc->SetLocalClassifierParam(param);
	*pResult = 0;
	UpdateData(FALSE);
}

void SysParam::OnNMCustomdrawSliderPolygoncosthreshold(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	cosThreshold = costhresCtr.GetPos()/100.0;
	CPolygonPaintTool::m_fCosThreshold = cosThreshold;
	*pResult = 0;
	UpdateData(FALSE);
}

void SysParam::OnNMCustomdrawSliderPolygondisthreshold(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	disThreshold = (double)disthresCtr.GetPos();
	CPolygonPaintTool::m_fDisThreshold = disThreshold;
	*pResult = 0;
	UpdateData(FALSE);
}

