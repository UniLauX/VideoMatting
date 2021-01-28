#pragma once

#include "VideoEditingDoc.h"
#include "afxwin.h"
#include "resource.h"

class CVideoEditingView;
// RotoScopeParam dialog

class RotoScopeParam : public CDialog
{
	//DECLARE_DYNAMIC(RotoScopeParam);
	

public:
	RotoScopeParam(CWnd* pParent = NULL);   // standard constructor
	DECLARE_DYNCREATE(RotoScopeParam);
	virtual ~RotoScopeParam();

// Dialog Data
	enum { IDD = IDD_DIALOG_ROTOSCOPE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL RotoScopeParam::PreTranslateMessage(MSG* pMsg);
	virtual void OnOK(){};
	BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	CVideoEditingView* getView();
	CVideoEditingDoc* getDoc();

private:
	//parameter
	int kup;
	int klow;
	int startFrameIndex;
	int endFrameIndex;
	int pyramidLevel;


	//save parameter
	int ls;
	int lb;
	float bmin;
	float bmax;
	int startIndex;
	int endIndex;

	//assist
	int curruntFrameIndex;
	int totalFrameNum;



public:

	afx_msg void OnEnChangeKlow();
	afx_msg void OnEnChangeKu();
	afx_msg void OnEnChangeStartindex();
	afx_msg void OnEnChangeEndindex();
	afx_msg void OnEnChangeEditPyrLevel();
	afx_msg void OnBnClickedSaveMask();
	afx_msg void OnEnChangeSaveStart();
	afx_msg void OnEnChangeSaveEnd();
	afx_msg void OnEnChangeEditBmin();
	afx_msg void OnEnChangeEditBmax();
	afx_msg void OnEnChangeEditLb();
	afx_msg void OnEnChangeEditLs();
	afx_msg void OnBnClickedCheckPreview();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButtonObjectdir();
	afx_msg void OnBnClickedButtonImport();
	afx_msg void OnBnClickedButtonExport();
};
