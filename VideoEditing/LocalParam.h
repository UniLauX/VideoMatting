#pragma once
#include "afxcmn.h"
#include "../VideoCut/LocalClassifier.h"

// CLocalParam dialog


class CLocalParam : public CDialog
{
	DECLARE_DYNAMIC(CLocalParam)

public:
	CLocalParam(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLocalParam();

// Dialog Data
	enum { IDD = IDD_PARAM_CLASSIFIER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:

    CSliderCtrl boundCtr;
    CSliderCtrl cutoffCtr;
    CSliderCtrl eminCtr;
    CSliderCtrl foreThresCtr;
    CSliderCtrl backThresCtr;

public:

    ClassifierParam_t param;

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMCustomdrawSlider3(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMCustomdrawSlider4(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMCustomdrawSlider5(NMHDR *pNMHDR, LRESULT *pResult);
};
