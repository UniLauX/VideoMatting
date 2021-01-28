

#include "stdafx.h"
#include "HistoryEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CHistoryEdit::CHistoryEdit()
{
  m_bSelectable = FALSE;
}

CHistoryEdit::~CHistoryEdit()
{
}

BEGIN_MESSAGE_MAP(CHistoryEdit, CEdit)
	//{{AFX_MSG_MAP(CHistoryEdit)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CHistoryEdit::AppendString  (CString str)

{
CString   strBuffer;   

  // Append string
  GetWindowText (strBuffer);
  if (!strBuffer.IsEmpty())
     strBuffer += "\r\n";
  strBuffer += str;
  SetWindowText (strBuffer);

  // Scroll the edit control
  LineScroll (GetLineCount(), 0);
}


void CHistoryEdit::OnSetFocus(CWnd* pOldWnd) 
{
  
  if (m_bSelectable)
     CEdit::OnSetFocus (pOldWnd);
  else
     pOldWnd->SetFocus();
}

