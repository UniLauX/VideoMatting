// PlayToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PlayToolBar.h"
#include "PlayControl.h"
#include <iostream>

using namespace std;
// CPlayToolBar

IMPLEMENT_DYNAMIC(CPlayToolBar, CDialogBar)

CPlayToolBar::CPlayToolBar()
{
	mNeedResize=TRUE;

	m_nHscrollMax = 100;
	m_nHscrollPos = 0;
	m_nHorzInc = 1;
}

CPlayToolBar::~CPlayToolBar()
{
}


BEGIN_MESSAGE_MAP(CPlayToolBar, CDialogBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_NCPAINT()
	ON_MESSAGE(WM_SLIDER_POS_CHANGED, OnSliderMoving)

	ON_COMMAND(ID_PLAY_FORWARD,OnPlayForward)
	ON_COMMAND(ID_PLAY_BACKWARD,OnPlayBackword)
	ON_COMMAND(ID_PLAY_STOP,OnPlayStop)
	ON_COMMAND(ID_PLAY_TOSTART,OnPlayToStart)
	ON_COMMAND(ID_PLAY_TOEND,OnPlayToEnd)
	ON_COMMAND(ID_PLAY_STEPLAST,OnPlayStepBackward)
	ON_COMMAND(ID_PLAY_STEPNEXT,OnPlayStepForward)

END_MESSAGE_MAP()



// CPlayToolBar message handlers



int CPlayToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	if (!m_wndPlayToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM
		| CBRS_TOOLTIPS | CBRS_SIZE_DYNAMIC) ||
		!m_wndPlayToolBar.LoadToolBar(IDR_TOOLBAR_PLAY))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndPlayToolBar.LoadBitmaps(16, COLORREF(RGB(190,  190, 190)),
		IDB_PLAYBAR_MENU, IDB_PLAYBAR_MENU,IDB_PLAYBAR_MENU_DISABLE);

	m_wndPlayToolBar.SetOwner (this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	// 	if (!m_wndHorzBar.Create(SBS_HORZ|WS_CHILD|WS_VISIBLE,
	//                                        rectDummy, this,
	//                                        23423))
	//     {
	//        TRACE0("Failed to create scrollbar\n");
	//        return FALSE;
	//     }
	// 	m_wndHorzBar.SetScrollRange(0, 100);

	if(!m_wndPlaySlider.Create(WS_CHILD|WS_VISIBLE, rectDummy, this, 23423))
	{
		TRACE0("Failed to create sliderbar\n");
		return FALSE;
	}
	m_wndPlaySlider.SetRange(0, 50);

	// TOOL TIP
	// Parent window, Mirror show
	m_wndTip.Create(this);

	return 0;
}

void CPlayToolBar::OnSize(UINT nType, int cx, int cy)
{
	CDialogBar::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	CRect rect;
	if(AfxGetMainWnd()){
	AfxGetMainWnd()->GetClientRect(rect);
	SetWindowPos(NULL,0,0,rect.Width(),cy,SWP_NOMOVE);
	}

	AdjustLayout();

	mNeedResize=TRUE;

	Invalidate();
}

void CPlayToolBar::AdjustLayout()
{
	CRect rectClient;
	GetClientRect (rectClient);

	CSize sizeTlb = m_wndPlayToolBar.CalcFixedLayout (FALSE, TRUE);
	m_wndPlayToolBar.SetWindowPos (NULL, rectClient.left, rectClient.bottom - sizeTlb.cy +2, 
		sizeTlb.cx, sizeTlb.cy+2,
		SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndPlaySlider.SetWindowPos (NULL, sizeTlb.cx,  rectClient.bottom-22, 
		rectClient.Width() - sizeTlb.cx,
		20,
		SWP_NOACTIVATE | SWP_NOZORDER);
}
void CPlayToolBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogBar::OnPaint() for painting messages

	if(mNeedResize)
	{
		CWnd* pWnd = AfxGetMainWnd();
		if(pWnd)
		{
			//			pWnd->SetPlayBarHeight(20);
			mNeedResize=FALSE;
		}
	}
	else
	{
		CDialogBar::OnPaint();
	}


	CRect rectWnd;
	GetWindowRect (rectWnd);
	ScreenToClient (rectWnd);

	rectWnd.InflateRect(1,1);
	dc.FillSolidRect(rectWnd, ::GetSysColor (COLOR_3DFACE));	

}

void CPlayToolBar::SetRange(int nMax)
{	
	m_nHscrollMax = nMax;
	m_nHscrollPos = 0;

	//	m_wndHorzBar.SetScrollRange(0, m_nHscrollMax-1);
	m_wndPlaySlider.SetRange(0, m_nHscrollMax-1);
}

void CPlayToolBar::SetPos(int nPos)
{
	if(nPos >= 0 && nPos < m_nHscrollMax)
		//		m_wndHorzBar.SetScrollPos(nPos);
		m_wndPlaySlider.SetPos(nPos);
	

}

int CPlayToolBar::GetPos()
{
	return m_wndPlaySlider.GetPos();
}
void CPlayToolBar::OnNcPaint() 
{
	// TODO: Add your message handler code here
	CPaintDC dc(this); // device context for painting

	CRect rectWnd;
	GetWindowRect (rectWnd);
	ScreenToClient (rectWnd);

	rectWnd.InflateRect(1,1);
	dc.FillSolidRect(rectWnd, ::GetSysColor (COLOR_3DFACE));		
	// Do not call CBCGPDockingControlBar::OnNcPaint() for painting messages
}

LRESULT CPlayToolBar::OnSliderMoving(WPARAM wParam, LPARAM lParam)
{
	//cout<<"wParam:"<<wParam<<endl;
	//cout<<"lParam:"<<lParam<<endl;
	CPlayControl::GetInstance()->Seek((int)lParam);

	m_wndPlaySlider.RedrawWindow();
	return 0;
}

void CPlayToolBar::OnPlayForward()
{
	CPlayControl::GetInstance()->PlayForward();
}

void CPlayToolBar::OnPlayBackword()
{
	CPlayControl::GetInstance()->PlayBackward();
}

void CPlayToolBar::OnPlayStop()
{
	CPlayControl::GetInstance()->Stop();
}

void CPlayToolBar::OnPlayToStart()
{
	CPlayControl::GetInstance()->GoBegin();
}

void CPlayToolBar::OnPlayToEnd()
{
	CPlayControl::GetInstance()->GoEnd();
}

void CPlayToolBar::OnPlayStepBackward()
{
	CPlayControl::GetInstance()->StepIt(false);
}

void CPlayToolBar::OnPlayStepForward()
{
	CPlayControl::GetInstance()->StepIt(true);
}