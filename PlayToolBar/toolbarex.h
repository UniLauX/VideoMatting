#if !defined(AFX_TOOLBAREX_H__CD96AE53_20BD_4EBD_823A_2A51BEA3EBB4__INCLUDED_)
#define AFX_TOOLBAREX_H__CD96AE53_20BD_4EBD_823A_2A51BEA3EBB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// toolbarex.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CToolBarEx window

class CToolBarEx : public CToolBar
{
public: 
	void LoadBitmaps(DWORD id, COLORREF clrMask, 
		DWORD idBitmap, DWORD idHotBitmap, DWORD idDisableBitmap);
	// All commands should be routed via owner window, not via frame:
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler) 
	{ 
		CToolBar::OnUpdateCmdUI ((CFrameWnd*) GetOwner (), bDisableIfNoHndler);
	}

	// Toolbar should't appear on the customization list
	virtual BOOL AllowShowOnList () const		{	return FALSE;	}

protected:
	//{{AFX_MSG(CToolBarEx)

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBAREX_H__CD96AE53_20BD_4EBD_823A_2A51BEA3EBB4__INCLUDED_)
