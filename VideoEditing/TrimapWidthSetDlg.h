#pragma once

#include "resource.h"
// CTrimapWidthSetDlg dialog

class CTrimapWidthSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CTrimapWidthSetDlg)

public:
	CTrimapWidthSetDlg(CWnd* pParent = NULL);   // standard constructor
	CTrimapWidthSetDlg( const CString& title,CWnd* pParent = NULL);
	virtual ~CTrimapWidthSetDlg();
	int GetTrimapWidth();
// Dialog Data
	enum { IDD = IDD_DIALOG_TRIMAPWIDTH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
private:
	// // must be integer, between 1 to 10
	int m_iTrimapWidth;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
