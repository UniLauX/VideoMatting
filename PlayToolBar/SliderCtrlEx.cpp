// SliderCtrlEx.cpp : implementation file
//

#include "../VideoEditing/stdafx.h"
#include "SliderCtrlEx.h"
#include "../VideoEditing/Resource.h"
#include "memdc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define LINETIMER  13
/////////////////////////////////////////////////////////////////////////////
// CSliderCtrlEx

CSliderCtrlEx::CSliderCtrlEx()
{
	CBitmap bmp;
	bmp.LoadBitmap(IDB_SLIDER_HOVER);
	m_brhHover.CreatePatternBrush(&bmp);
	bmp.DeleteObject();
	bmp.LoadBitmap(IDB_SLIDER_NORMAL);
	m_brhNormal.CreatePatternBrush(&bmp);


	m_clrLineColor0 = RGB(17, 116, 152);
	m_clrLineColor1 = RGB(170, 195, 225);

	m_bHover = FALSE;
	m_bDragging = FALSE;
	m_bAutoLineing = FALSE;

	m_nRangeMax = 0;
	m_nRangeMin = 0;
	m_nPos = 0;
	m_nLineSize = 1;
	m_nThumbWidth = 30;
	m_nThumbHeight = 14;

	m_hHand = NULL;

	m_hHand = ::LoadCursor(NULL, MAKEINTRESOURCE(32649));
}

CSliderCtrlEx::~CSliderCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CSliderCtrlEx, CSliderCtrl)
	//{{AFX_MSG_MAP(CSliderCtrlEx)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSliderCtrlEx message handlers

void CSliderCtrlEx::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CMemDC pDC(&dc); 
	DrawSlider(&pDC);
	// Do not call CSliderCtrl::OnPaint() for painting messages
}

void CSliderCtrlEx::DrawSlider(CDC *pDC)
{
	CRect rectClient;
	GetClientRect(rectClient);
	pDC->FillSolidRect(rectClient, ::GetSysColor(COLOR_3DFACE));

	CRect rectLine;
	GetLineRect(rectLine);
	pDC->Draw3dRect(rectLine, m_clrLineColor0, m_clrLineColor1);
	rectLine.DeflateRect(1, 1, 0, 1);
	pDC->Draw3dRect(rectLine, m_clrLineColor0, m_clrLineColor1);

	CRect rectThumb;
	GetThumbRect(rectThumb);

	CPen  pen(PS_SOLID, 1, RGB(255,255,255));
	CPen  *pOldPen = pDC->SelectObject(&pen);
	CBrush *pOldBrh = NULL;

	if(m_bHover||m_bDragging)
		pOldBrh = pDC->SelectObject(&m_brhHover);
	else
		pOldBrh = pDC->SelectObject(&m_brhNormal);
	pDC->RoundRect(&rectThumb, CPoint(5,5));

	pDC->SelectObject(pOldBrh);
	pDC->SelectObject(pOldPen);
// 	CString str;
// 	str.Format("%d", m_nPos);
// 	pDC->DrawText(str, rectThumb, DT_CENTER);
}

BOOL CSliderCtrlEx::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
//	return CSliderCtrl::OnEraseBkgnd(pDC);
}

void CSliderCtrlEx::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rectThumb;
	GetThumbRect(rectThumb);
	if(m_bDragging && GetCapture() == this)
	{
		CRect rectClient;
		GetClientRect(rectClient);
		m_nPos = point.x*(m_nRangeMax - m_nRangeMin)/rectClient.Width();
		VerifyPos();
		Invalidate(FALSE);
	}
	else if(rectThumb.PtInRect(point) && !m_bHover)
	{
		m_bHover = TRUE;
		Invalidate(FALSE);
	}
	else if(!rectThumb.PtInRect(point) && m_bHover)
	{
		m_bHover = FALSE;
		Invalidate(FALSE);
	}
//	CSliderCtrl::OnMouseMove(nFlags, point);
}

void CSliderCtrlEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rectThumb;
	GetThumbRect(rectThumb);

	if(rectThumb.PtInRect(point))
	{
		m_bDragging = TRUE;
		SetCapture();
	}
	else 
	{
		CRect rectLine;
		GetLineRect(rectLine);
		rectLine.InflateRect(0, 5, 0, 5);

		if(rectLine.PtInRect(point))
		{
			if(point.x < rectThumb.left)
			{
				m_nPos -= m_nLineSize;
			}
			else if(point.x > rectThumb.right)
			{
				m_nPos += m_nLineSize;
			}
//			SetTimer(LINETIMER, 300, NULL);
			m_bAutoLineing = TRUE;
			VerifyPos();
			Invalidate(FALSE);
		}
	}
//	CSliderCtrl::OnLButtonDown(nFlags, point);
}

void CSliderCtrlEx::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_bDragging)
	{
		m_bDragging = FALSE;
		ReleaseCapture();
	}
	else if(m_bAutoLineing)
	{
		m_bAutoLineing = FALSE;
		KillTimer(LINETIMER);
	}
	OnMouseMove(nFlags, point);

//	CSliderCtrl::OnLButtonUp(nFlags, point);
}

void CSliderCtrlEx::GetThumbRect( LPRECT lprc ) const
{
	CRect rectClient;
	GetClientRect(rectClient);
	if(m_nRangeMax - m_nRangeMin == 0)
		lprc->left = 0;
	else
		lprc->left = 
		(rectClient.Width() - m_nThumbWidth)*m_nPos/(m_nRangeMax - m_nRangeMin);

	lprc->top = rectClient.Height()/5;
 	lprc->right = lprc->left + m_nThumbWidth;
 	lprc->bottom = lprc->top + m_nThumbHeight;
}

int	CSliderCtrlEx::GetRangeMax( ) const
{
	return m_nRangeMax;
}

int	CSliderCtrlEx::GetRangeMin( ) const
{
	return m_nRangeMin;
}

void CSliderCtrlEx::GetRange( int& nMin, int& nMax ) const
{
	nMin = m_nRangeMin;
	nMax = m_nRangeMax;
}

void CSliderCtrlEx::SetRangeMin( int nMin, BOOL bRedraw )
{
	m_nRangeMin = nMin;
	if(bRedraw)
		Invalidate(FALSE);
}

void CSliderCtrlEx::SetRangeMax( int nMax, BOOL bRedraw )
{
	m_nRangeMax = nMax;
	if(bRedraw)
		Invalidate(FALSE);
}

void CSliderCtrlEx::SetRange( int nMin, int nMax, BOOL bRedraw )
{
	m_nRangeMax = nMax;
	m_nRangeMin = nMin;
	if(bRedraw)
		Invalidate(FALSE);
}

int	CSliderCtrlEx::GetPos( ) const
{
	return m_nPos;
}

void CSliderCtrlEx::SetPos( int nPos )
{
	m_nPos = nPos;
	Invalidate(FALSE);
}

void CSliderCtrlEx::GetLineRect(LPRECT lprc) const
{
	CRect rectLine;
	GetClientRect(rectLine);
	rectLine.DeflateRect(0, 10, 0, 0);
	rectLine.bottom = rectLine.top + 4;
	*lprc = rectLine;
}

int CSliderCtrlEx::GetLineSize() const
{
	return m_nLineSize;
}

int CSliderCtrlEx::SetLineSize(int nSize)
{
	int nOldSize = m_nLineSize;
	m_nLineSize = nSize;
	return nOldSize;
}

void CSliderCtrlEx::VerifyPos()
{
	if(m_nPos < m_nRangeMin)
		m_nPos = m_nRangeMin;
	else if(m_nPos >= m_nRangeMax)
	{
		m_nPos = m_nRangeMax;
	}
	::SendMessage(
			GetParent()->GetSafeHwnd(), WM_SLIDER_POS_CHANGED,
			GetDlgCtrlID(), m_nPos );
}

void CSliderCtrlEx::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == LINETIMER)
	{
		m_nPos -= m_nLineSize;
		Invalidate(FALSE);
	}	
//	CSliderCtrl::OnTimer(nIDEvent);
}

BOOL CSliderCtrlEx::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	if( m_hHand != NULL )
	{
		::SetCursor(m_hHand);
		return TRUE;
	}
	return FALSE;	
//	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}
