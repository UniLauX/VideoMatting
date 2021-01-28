#pragma once
#include "resource.h"

// CTemporalMattingDlg dialog

class CTemporalMattingDlg : public CDialog
{
	DECLARE_DYNAMIC(CTemporalMattingDlg)

public:
	CTemporalMattingDlg(CWnd* pParent = NULL);   // standard constructor
	CTemporalMattingDlg(CString layername, int sf, int ef, int step, float sw, float tw, CWnd* pParent=NULL);
	virtual ~CTemporalMattingDlg();

// Dialog Data
	enum { IDD = IDD_TEMPORAL_MATTING_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_layerName;
	int m_startFrame;
	int m_endFrame;
	int m_step;
	float m_spatialWeight;
	float m_temporalWeight;
	BOOL m_bAnchorFirst;
	BOOL m_bAnchorLast;
	afx_msg void OnBnClickedCheckAnchorFirst();
	afx_msg void OnBnClickedCheckAnchorLast();
};
