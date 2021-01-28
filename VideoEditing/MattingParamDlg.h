#pragma once
#include "afxwin.h"
#include "resource.h"
#include "VideoEditingView.h"


// CMattingParamDlg dialog

class CMattingParamDlg : public CDialog
{
	DECLARE_DYNAMIC(CMattingParamDlg)

public:
	CMattingParamDlg(CWnd* pParent = NULL);   // standard constructor
	CMattingParamDlg(CString _layername, int _sframe, int _endframe, CWnd* pParent=NULL);
	virtual ~CMattingParamDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MATTING_PARAM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// //show current layer name for matting
	CString m_layerName;
	// //show start frame index
	int m_startFrame;
	// //show end frame index
	int m_endFrame;
	// //indicate matting algorithm
	CComboBox m_algorithmType;
	afx_msg void OnCbnSelchangeComboAlgorithm();
	//indicate matting algorithm
	int m_algorithm;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	// // partition or whole mode
	int m_radio;
	afx_msg void OnBnClickedRadioMatPart();
	afx_msg void OnBnClickedRadioMatWhole();
	CVideoEditingDoc* getDoc();

};
