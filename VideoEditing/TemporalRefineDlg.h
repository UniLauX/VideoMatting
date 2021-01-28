#pragma once
#include "resource.h"
#include "afxwin.h"

// CTemporalRefineDlg dialog

class CTemporalRefineDlg : public CDialog
{
	DECLARE_DYNAMIC(CTemporalRefineDlg)

public:
	CTemporalRefineDlg(CWnd* pParent = NULL);   // standard constructor
	CTemporalRefineDlg(CString _layername, int _sf, int _ef, CWnd* pParent=NULL);
	virtual ~CTemporalRefineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_TEMPORAL_REFINE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// //start refine frame number
	int m_startFrame;
	// //end refine frame number
	int m_endFrame;

	CButton m_check_optical_sel;
	bool m_use_global_flow;
	afx_msg void OnBnClickedCheckOpticalGlobal();
	CString m_layerName;
};
