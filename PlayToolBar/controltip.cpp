// controltip.cpp : implementation file
//

#include "../VideoEditing/stdafx.h"
#include "controltip.h"
#include "memdc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CX_ROUNDED				12		// Tip horizontal roundness
#define CY_ROUNDED				8		// Tip vertical roundness
#define CX_LEADER				25		// Width of tip lead
#define CY_LEADER				25		// Height of tip lead

/////////////////////////////////////////////////////////////////////////////
// CControlTip

CControlTip::CControlTip()
{
	// Register the class
	m_szClass		= AfxRegisterWndClass(0);
	m_clrBackground = GetSysColor(COLOR_INFOBK);
	m_clrFrame		= RGB(0xBB, 0xF3, 0x11);// GetSysColor(COLOR_WINDOWTEXT);
	m_clrBorder		= GetSysColor(COLOR_SCROLLBAR);
	m_clrFont		= GetSysColor(COLOR_INFOTEXT);

	m_bMirror = FALSE;
}

CControlTip::~CControlTip()
{
}


BEGIN_MESSAGE_MAP(CControlTip, CWnd)
	//{{AFX_MSG_MAP(CControlTip)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CControlTip message handlers

void CControlTip::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rc;
	GetClientRect(rc);
	CMemDC memDC(&dc);
	
	CRgn	*pRegion;
	HRGN	hRegion;
	CSize	windowSize;
	GetWindowRegion(&dc, &hRegion, &windowSize);
	pRegion = CRgn::FromHandle(hRegion);

	CBrush brh(m_clrBackground);
	CBrush brh1(m_clrBorder);
	CBrush brh2(m_clrFrame);
	memDC.FillRgn(pRegion, &brh);
	memDC.FrameRgn(pRegion, &brh1, 3, 3);
	memDC.FrameRgn(pRegion, &brh2, 1, 1);
	brh.DeleteObject();
	brh1.DeleteObject();
	brh2.DeleteObject();

	if(m_bMirror)
	{
		rc.top += (CY_LEADER - CY_ROUNDED);
		rc.bottom += (CY_LEADER - CY_ROUNDED);
	}
	rc.DeflateRect(CX_ROUNDED, CY_ROUNDED, 0, 0);
	
	memDC.SetBkMode(TRANSPARENT);
	memDC.SetTextColor(m_clrFont);
	CFont *pSysFont = dc.SelectObject(GetFont());
	memDC.DrawText(m_strTip, rc, DT_LEFT|DT_TOP);

	memDC.SelectObject(pSysFont);
	::DeleteObject(hRegion);
	// Do not call CWnd::OnPaint() for painting messages
}

BOOL CControlTip::Create(CWnd* pParentWnd, BOOL _bMirror) 
{
	// TODO: Add your specialized code here and/or call the base class
	// Must have a parent
	ASSERT(pParentWnd != NULL);
	
	bool bSuccess = CreateEx(NULL, m_szClass, NULL, WS_POPUP, 0, 0, 0, 0, pParentWnd->GetSafeHwnd(), NULL, NULL);
	m_bMirror = _bMirror;
	return bSuccess;
}

void CControlTip::Show(CString strTip)
{
	m_strTip = strTip;
	CDC *pDC = GetDC();
	HRGN	hRegion;
	CSize	WindowSize;

	GetWindowRegion(pDC, &hRegion, &WindowSize);
	ReleaseDC(pDC);
	
	SetWindowRgn(hRegion, TRUE);

	::GetCursorPos(&m_ptCursor);

	if(m_bMirror)
	{
		SetWindowPos(&wndTop, m_ptCursor.x - WindowSize.cx + CX_ROUNDED, 
			m_ptCursor.y - CY_ROUNDED, WindowSize.cx, WindowSize.cy, 
			SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowPos(&wndTop, m_ptCursor.x - WindowSize.cx + CX_ROUNDED, 
			m_ptCursor.y - WindowSize.cy + CY_ROUNDED, WindowSize.cx, WindowSize.cy, 
			SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
	Invalidate(FALSE);
}

void CControlTip::Hide()
{
	ShowWindow(SW_HIDE);
}

BOOL CControlTip::GetWindowRegion(CDC* pDC, HRGN* hRegion, CSize *Size /* = NULL */)
{
	CRect	rcWnd;
	POINT	ptLeader[3];
	CRgn	LeaderRegion;
	CRgn	CaptionRegion;
	
	ASSERT(pDC != NULL);
	ASSERT(hRegion != NULL);

	// Calculate the are for the tip text
	CFont	*pSysFont = (CFont *)pDC->SelectObject(GetFont());
	pDC->DrawText(m_strTip, &rcWnd, DT_CALCRECT);
	pDC->SelectObject(pSysFont);

	// Adjust for the rounded corners
	rcWnd.InflateRect(CX_ROUNDED, CY_ROUNDED);


	// Calculate the leader triangle coordinates
	if(m_bMirror)
	{
		ptLeader[0].x	= rcWnd.Width() - CX_ROUNDED;
		ptLeader[0].y	= CY_LEADER + CY_ROUNDED;

		ptLeader[1].x	= ptLeader[0].x;
		ptLeader[1].y	= ptLeader[0].y - CY_LEADER;

		ptLeader[2].x	= ptLeader[0].x - CX_LEADER;
		ptLeader[2].y	= CY_LEADER + CY_ROUNDED;
	}
	else
	{
		ptLeader[0].x	= rcWnd.Width() - CX_ROUNDED;
		ptLeader[0].y	= rcWnd.Height() - CY_ROUNDED;

		ptLeader[1].x	= ptLeader[0].x;
		ptLeader[1].y	= ptLeader[0].y + CY_LEADER;

		ptLeader[2].x	= ptLeader[0].x - CX_LEADER;
		ptLeader[2].y	= rcWnd.Height() - CY_ROUNDED;
	}

	// Create the caption region
	if(m_bMirror)
	{
		CaptionRegion.CreateRoundRectRgn(0, CY_LEADER - CY_ROUNDED, rcWnd.Width(), 
			rcWnd.Height() + CY_LEADER - CY_ROUNDED, CX_ROUNDED, CY_ROUNDED);
	}
	else
	{
		CaptionRegion.CreateRoundRectRgn(0, 0, rcWnd.Width(), rcWnd.Height(), CX_ROUNDED, CY_ROUNDED);
	}
	// Create the leader region
	LeaderRegion.CreatePolygonRgn(ptLeader, 3, ALTERNATE);
	// Create window region
	*hRegion =  ::CreateRectRgn(0, 0, rcWnd.Width(), rcWnd.Height() + CY_LEADER);
	
	// Combine the regions
	CombineRgn(*hRegion, CaptionRegion.operator HRGN(), LeaderRegion.operator HRGN(), RGN_OR);

	// Set the window size
	if (Size != NULL)
	{
		Size->cx	= rcWnd.Width();
		Size->cy	= rcWnd.Height() + CY_LEADER;
	}

	return TRUE;
}

BOOL CControlTip::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return FALSE;
//	return CWnd::OnEraseBkgnd(pDC);
}
