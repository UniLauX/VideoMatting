// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "VideoEditing.h"

#include "MainFrm.h"
#include "PlayControl.h"
#include "CoolTabCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	//added func for active the toolbar button
	ON_COMMAND_RANGE(ID_TOOLS_BASE,ID_TOOLS_END,OnToolsBaseRange)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TOOLS_BASE,ID_TOOLS_END,OnUpdateToolsBaseRange)

	ON_MESSAGE(WM_COMMAND_UPDATE_TIMELINE, OnUpdateTimeLine)
	ON_COMMAND(ID_VIEW_DOCKBAR, &CMainFrame::OnViewDockbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DOCKBAR, &CMainFrame::OnUpdateViewDockbar)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};
// CMainFrame construction/destruction
ofstream logText;
CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	logText.open("log.txt",ios::out|ios::_Noreplace);
}

CMainFrame::~CMainFrame()
{
	logText.close();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	if (!m_wndCommandBar.Create(_T("my bar"),this,CSize(220,260),true,123))
	{
		TRACE0("Failed to create bar\n");
		return -1;
	}
	//tab control
	m_wndTabCont.Create(TCS_UP|WS_CHILD|WS_VISIBLE,CRect(0,0,100,50),&m_wndCommandBar,125);
	//add pages here
	
	m_wndTabCont.AddPage(RUNTIME_CLASS(SysParam),IDD_PARAM_SYSTEM,"System",1);
	m_wndTabCont.AddPage(RUNTIME_CLASS(CMultiLayerDlg),IDD_MULTILAYER_DLG,"MultiLayer",2);
	m_wndTabCont.AddPage(RUNTIME_CLASS(CClosedFormParam),IDD_PARAM_CLOSEDFORM,"Matting",0);
	m_wndTabCont.AddPage(RUNTIME_CLASS(RotoScopeParam), IDD_DIALOG_ROTOSCOPE,"RotoScope",3);
	
	
	m_wndTabCont.UpdateWindow();

	//style and dock position control
	m_wndCommandBar.SetBarStyle(m_wndCommandBar.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|
		CBRS_SIZE_DYNAMIC);
	m_wndCommandBar.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndCommandBar,AFX_IDW_DOCKBAR_LEFT);
	BOOL bRes = m_wndViewPlayBar.Create(this, IDD_DIALOG_PLAY, WS_VISIBLE|WS_CHILD|CBRS_BOTTOM, 1);
	m_wndViewPlayBar.EnableDocking(CBRS_ORIENT_HORZ);
	DockControlBar(&m_wndViewPlayBar);

	CPlayControl::GetInstance()->SetDuration(200);
	m_wndViewPlayBar.SetRange(200);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style   &=   ~WS_THICKFRAME;//使窗口不能用鼠标改变大小
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return TRUE;
}
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

LRESULT CMainFrame::OnUpdateTimeLine(WPARAM wParam, LPARAM lParam)
{
	m_wndViewPlayBar.SetPos((int)wParam);
	CPlayControl::GetInstance()->setPos((int)wParam);
	return 0;
}

void CMainFrame::OnViewDockbar()
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_wndCommandBar,!m_wndCommandBar.IsVisible(),false);
}

void CMainFrame::OnUpdateViewDockbar(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_wndCommandBar.IsVisible());
}

int CMainFrame::GetPos()
{
	return this->m_wndViewPlayBar.GetPos();
}

void CMainFrame::setRange(int range)
{
	CPlayControl::GetInstance()->SetDuration(range);
	m_wndViewPlayBar.SetRange(range);
}
void CMainFrame::addLayer(CString name,int depth)
{
	CCoolTabCtrl::CPageItem *pt=(CCoolTabCtrl::CPageItem*)this->m_wndTabCont.GetPageItem(1);
	CMultiLayerDlg *layDlg=(CMultiLayerDlg*)pt->m_pWnd;
	layDlg->addLayer(name,depth);
}

void CMainFrame::setCheckboxStatus(bool ischeked)
{
	CCoolTabCtrl::CPageItem *pt=(CCoolTabCtrl::CPageItem*)this->m_wndTabCont.GetPageItem(1);
	CMultiLayerDlg *layDlg=(CMultiLayerDlg*)pt->m_pWnd;
	layDlg->setCheckboxStatus(ischeked);
}
LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message==WM_NCLBUTTONDBLCLK)
	{
		return 0;
	}
	if (message==WM_SYSCOMMAND)
	{		
		if((wParam==SC_MAXIMIZE)||(wParam==SC_MINIMIZE)||(wParam==SC_RESTORE)||(wParam==SC_SIZE))
				((CVideoEditingView*)this->GetActiveView())->calstop();
	}
	if (message==WM_MOVING)
			((CVideoEditingView*)this->GetActiveView())->calstop();
	//if(message==WM_ACTIVATE)
	//	((CVideoEditingView*)this->GetActiveView())->calstop();
	return CFrameWnd::WindowProc(message,wParam,lParam);
}
//added func for drawing polygon
void CMainFrame::OnToolsBaseRange(UINT nID)
{
	IsToolManager = !IsToolManager;
	CToolManagerFrame::OnSwitchActiveTool(nID);
}

void CMainFrame::OnUpdateToolsBaseRange(CCmdUI* pCmdUI)
{
	CToolManagerFrame::OnUpdateActiveToolUI(pCmdUI);
}
