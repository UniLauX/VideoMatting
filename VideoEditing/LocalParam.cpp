// LocalParam.cpp : implementation file
//

#include "stdafx.h"
#include "VideoEditing.h"
#include "LocalParam.h"
#include <iostream>

using namespace std;

// CLocalParam dialog

IMPLEMENT_DYNAMIC(CLocalParam, CDialog)

CLocalParam::CLocalParam(CWnd* pParent /*=NULL*/)
	: CDialog(CLocalParam::IDD, pParent)
{

}

CLocalParam::~CLocalParam()
{
}

void CLocalParam::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SLIDER1, boundCtr);
    DDX_Control(pDX, IDC_SLIDER2, cutoffCtr);
    DDX_Control(pDX, IDC_SLIDER3, eminCtr);
    DDX_Control(pDX, IDC_SLIDER4, foreThresCtr);
    DDX_Control(pDX, IDC_SLIDER5, backThresCtr);
}


BEGIN_MESSAGE_MAP(CLocalParam, CDialog)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CLocalParam::OnNMCustomdrawSlider1)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, &CLocalParam::OnNMCustomdrawSlider2)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, &CLocalParam::OnNMCustomdrawSlider3)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, &CLocalParam::OnNMCustomdrawSlider4)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER5, &CLocalParam::OnNMCustomdrawSlider5)
END_MESSAGE_MAP()


// CLocalParam message handlers

BOOL CLocalParam::OnInitDialog()
{
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

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CLocalParam::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    param.boundDis=boundCtr.GetPos();
    *pResult = 0;
}

void CLocalParam::OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    param.fcutoff=cutoffCtr.GetPos()/100.0;
    *pResult = 0;
}

void CLocalParam::OnNMCustomdrawSlider3(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    param.eMin=eminCtr.GetPos();
    *pResult = 0;
}

void CLocalParam::OnNMCustomdrawSlider4(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    param.foreThres=foreThresCtr.GetPos()/100.0;
    *pResult = 0;
}

void CLocalParam::OnNMCustomdrawSlider5(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    param.backThres=backThresCtr.GetPos()/100.0;
    *pResult = 0;
}
