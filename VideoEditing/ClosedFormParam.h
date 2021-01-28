#pragma once


#include "VideoEditingDoc.h"
#include "afxwin.h"
// CClosedFormParam dialog

class CVideoEditingView;

class CClosedFormParam : public CDialog
{
	//DECLARE_DYNAMIC(CClosedFormParam)

public:
	CClosedFormParam(CWnd* pParent = NULL);   // standard constructor
	DECLARE_DYNCREATE(CClosedFormParam);
	virtual ~CClosedFormParam();

// Dialog Data
	enum { IDD = IDD_PARAM_CLOSEDFORM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL CClosedFormParam::PreTranslateMessage(MSG* pMsg);
	virtual void OnOK(){};
	BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:
	CVideoEditingDoc* getDoc();
	CVideoEditingView* GetView();

private:
	int level;
	int activeLevel;
	int winSize;
	double alphaThreshold;
	double epsilon;
	UINT prePos;
	CScrollBar m_scrollbar;
	int m_scrollbar_value;

public:
	float m_fTriMpAlpha;
	CSliderCtrl m_ctrlTriMpAlpha;

public:
	//afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	CComboBox m_algorithm;
	int m_radio;
	afx_msg void OnBnClickedRadioImpart();
	afx_msg void OnBnClickedRadioImgwhole();
	afx_msg void OnCbnSelchangeComboAlgo();
	afx_msg void OnBnClickedCheckFrameMat();
	CButton m_checkBoxframeMat;
	afx_msg void OnNMCustomdrawSliderTrimapAlpha(NMHDR *pNMHDR, LRESULT *pResult);
};
