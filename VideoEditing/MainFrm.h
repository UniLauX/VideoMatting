// MainFrm.h : interface of the CMainFrame class
//
#pragma once

#include "PlayToolBar.h"
#include "ClosedFormParam.h"
#include "RotoScopeParam.h"
#include "CoolTabCtrl.h"
#include "SysParam.h"
#include "MultiLayerDlg.h"
#include "Z:\Framework\ToolManager\include\ToolManagerFrame.h"

class CMainFrame : public CFrameWnd,public CToolManagerFrame
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
CCoolBar m_wndCommandBar;
// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CPlayToolBar	m_wndViewPlayBar;

	CMultiLayerDlg layerDlg;

public:
	CCoolTabCtrl  m_wndTabCont;//tab control
	

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

public:
	LRESULT OnUpdateTimeLine(WPARAM wParam, LPARAM lParam);
	afx_msg void OnViewDockbar();
	afx_msg void OnUpdateViewDockbar(CCmdUI *pCmdUI);

	int GetPos();
	void setRange(int range);
	void addLayer(CString name ,int depth);
	void setCheckboxStatus(bool ischeked);
	void OnToolsBaseRange(UINT nID);
	void OnUpdateToolsBaseRange(CCmdUI* pCmdUI);
};


