#if !defined(AFX_SLIDERCTRLEX_H__7159B843_B2DC_494A_BEAD_42EAF27B0438__INCLUDED_)
#define AFX_SLIDERCTRLEX_H__7159B843_B2DC_494A_BEAD_42EAF27B0438__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SliderCtrlEx.h : header file
//
#define WM_SLIDER_POS_CHANGED WM_USER + 1213
/////////////////////////////////////////////////////////////////////////////
// CSliderCtrlEx window

class CSliderCtrlEx : public CSliderCtrl
{
// Construction
public:
	CSliderCtrlEx();

// Attributes
public:
	COLORREF	m_clrLineColor0;
	COLORREF	m_clrLineColor1;
protected:
	CBrush		m_brhNormal;
	CBrush		m_brhHover;

	BOOL    	m_bHover;
	BOOL		m_bDragging;
	BOOL	    m_bAutoLineing;

	int			m_nRangeMin;
	int			m_nRangeMax;

	int			m_nPos;
	int			m_nLineSize;

	int			m_nThumbWidth;
	int			m_nThumbHeight;	

	HCURSOR		m_hHand;

// Operations
public:
	int			GetRangeMax( ) const;
	int			GetRangeMin( ) const;
	void		GetRange( int& nMin, int& nMax ) const;	
	void		SetRangeMin( int nMin, BOOL bRedraw = FALSE );
	void		SetRangeMax( int nMax, BOOL bRedraw = FALSE );
	void		SetRange( int nMin, int nMax, BOOL bRedraw = FALSE );
	int			GetPos( ) const;
	void		SetPos( int nPos );

	void		GetThumbRect( LPRECT lprc ) const; 
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSliderCtrlEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	void VerifyPos();
	int SetLineSize( int nSize );
	int GetLineSize() const;
	virtual ~CSliderCtrlEx();

protected:
	void DrawSlider(CDC*);
	// Generated message map functions
protected:
	void GetLineRect( LPRECT lprc ) const;
	//{{AFX_MSG(CSliderCtrlEx)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDERCTRLEX_H__7159B843_B2DC_494A_BEAD_42EAF27B0438__INCLUDED_)
