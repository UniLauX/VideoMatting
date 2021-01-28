#if !defined(AFX_CONTROLTIP_H__2A16D7D8_B78E_4765_94F3_95010367E6D8__INCLUDED_)
#define AFX_CONTROLTIP_H__2A16D7D8_B78E_4765_94F3_95010367E6D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// controltip.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CControlTip view

class CControlTip : public CWnd
{
public:
	CControlTip();   
	virtual ~CControlTip();

// Attributes
public:
	BOOL		m_bMirror; 
	COLORREF	m_clrBackground;
	COLORREF	m_clrBorder;
	COLORREF	m_clrFrame;
	COLORREF	m_clrFont;
	void Show(CString);
	void Hide();
protected:
	CPoint		m_ptCursor;
	LPCTSTR		m_szClass;	// Window class

	CString		m_strTip;
// Operations
	BOOL GetWindowRegion(CDC* pDC, HRGN* hRegion, CSize *Size = NULL);
public:
	BOOL Create(CWnd* pParentWnd, BOOL _bMirror = FALSE) ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CControlTip)
	public:
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
protected:
	//{{AFX_MSG(CControlTip)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTROLTIP_H__2A16D7D8_B78E_4765_94F3_95010367E6D8__INCLUDED_)
