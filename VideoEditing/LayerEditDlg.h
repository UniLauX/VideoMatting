#pragma once
#include "resource.h"
#include "afxwin.h"

// CLayerEditDlg dialog

class CLayerEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CLayerEditDlg)


public:
	CLayerEditDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLayerEditDlg();
	CString GetLayerName();
	int GetLayerDepth();
	int GetLayerType();
	void SetLayerNameDepth(CString name,int depth);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
// Dialog Data
	enum { IDD = IDD_DIALOG_EDITLAYER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CString LayerName;
	int layerDepth;
	int layerType;

public:
	CComboBox m_LayerTypeMFC;
	afx_msg void OnCbnSelchangeComboLayertype();
};
