// toolbarex.cpp : implementation file
//

#include "../VideoEditing/stdafx.h"
#include "toolbarex.h"
#include "TCImageList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolBarEx

BEGIN_MESSAGE_MAP(CToolBarEx, CToolBar)
	//{{AFX_MSG_MAP(CToolBarEx)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolBarEx message handlers

void CToolBarEx::LoadBitmaps(DWORD cx, COLORREF clrMask, DWORD idBitmap, DWORD idHotBitmap, DWORD idDisableBitmap)
{
	CTCImageList imagelist, hotimagelist, disimagelist;
	imagelist.CreateTC(idBitmap, clrMask);
	GetToolBarCtrl().SetImageList(&imagelist);

	hotimagelist.CreateTC(idHotBitmap, clrMask);
	GetToolBarCtrl().SetHotImageList(&imagelist);

	disimagelist.CreateTC(idDisableBitmap, clrMask);
	GetToolBarCtrl().SetDisabledImageList(&disimagelist);

	SetSizes(CSize(cx+7, cx+6), CSize(cx, cx));

	imagelist.Detach();
	hotimagelist.Detach();
	disimagelist.Detach();

}