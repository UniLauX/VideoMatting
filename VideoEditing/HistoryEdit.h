
#ifndef _HistoryEdit_h_
#define _HistoryEdit_h_



class CHistoryEdit : public CEdit
{

public:
	CHistoryEdit();

// Attributes
public:

// Operations
public:
  void  AppendString (CString str);
  BOOL  IsSelectable() { return m_bSelectable; }
  void  AllowSelection (BOOL bAllowSelect) { m_bSelectable = bAllowSelect; }


public:
	virtual ~CHistoryEdit();

	
protected:
	//{{AFX_MSG(CHistoryEdit)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
  BOOL  m_bSelectable;                          // flag: user can select text in control
};



#endif

