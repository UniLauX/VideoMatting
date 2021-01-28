#pragma once
#include "VideoEditingDoc.h"
#include "VideoEditingView.h"
#include "afxwin.h"
#include "HistoryEdit.h"
#include "../VideoCut/LocalClassifier.h"
// SysParam dialog

class SysParam : public CDialog
{
	//DECLARE_DYNAMIC(SysParam)
	DECLARE_DYNCREATE(SysParam)

public:
	SysParam(CWnd* pParent = NULL);   // standard constructor
	virtual ~SysParam();
	void setResultPath(CString rp);
	void setAlphaPath(CString ap);

// Dialog Data
	enum { IDD = IDD_PARAM_SYSTEM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL SysParam::PreTranslateMessage(MSG* pMsg);
	virtual void OnOK(){};
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	ClassifierParam_t param;
private:
	CVideoEditingView* getView();
	CVideoEditingDoc* getDoc();
	int videoWinSize;
	int updateWinSize;
	int iterateTime;
	double cosThreshold;
	double disThreshold;
	float lineDiff;
	int pointRadius;
	CSliderCtrl boundCtr;
	CSliderCtrl cutoffCtr;
	CSliderCtrl eminCtr;
	CSliderCtrl foreThresCtr;
	CSliderCtrl backThresCtr;
	CSliderCtrl costhresCtr;
	CSliderCtrl disthresCtr;
public:
	//CHistoryEdit editOut;
	afx_msg void OnEnChangeEditPolygonthreshold();
	afx_msg void OnEnChangeEditPolygonlinediff();
	afx_msg void OnEnChangeEditPolygonpointradius();
	afx_msg void OnEnChangeEditVideowinsize();
	afx_msg void OnEnChangeEditUpdatewinsize();
	afx_msg void OnEnChangeEditIteratetime();
	afx_msg void OnBnClickedButtonResetPara();
	afx_msg void OnNMCustomdrawSliderBoundsize(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderCutoff(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderEpsilon(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderForethres(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderBackthres(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderPolygoncosthreshold(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderPolygondisthreshold(NMHDR *pNMHDR, LRESULT *pResult);
};
