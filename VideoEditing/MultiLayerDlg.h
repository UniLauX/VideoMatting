#pragma once
#include "VideoEditingDoc.h"
#include "VideoEditingView.h"
#include "afxcmn.h"
#include "afxwin.h"



// CMultiLayerDlg dialog

class CMultiLayerDlg : public CDialog
{
	//DECLARE_DYNAMIC(CMultiLayerDlg)
		DECLARE_DYNCREATE(CMultiLayerDlg)

public:
	CMultiLayerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMultiLayerDlg();

// Dialog Data
	enum { IDD = IDD_MULTILAYER_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CListCtrl m_Layerlist;
public:
	virtual BOOL OnInitDialog();
	void addLayer(CString name,int depth);
	void setCheckboxStatus(bool ischecked);
protected:
	virtual void OnOK();
private:

	CVideoEditingView* getView();

public:

	
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonEdit();
	CSliderCtrl depthCtrl;
	CButton check_SelectPoint;
	int check_status;
	afx_msg void OnBnClickedCheckEnableDepth();
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
};