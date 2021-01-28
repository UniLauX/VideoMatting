// VideoEditing.h : main header file for the VideoEditing application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "toolmanagerapp.h"

// CVideoEditingApp:
// See VideoEditing.cpp for the implementation of this class
//

class CVideoEditingApp : public CWinApp,public CToolManagerApp
{
public:
	CVideoEditingApp();


// Overrides
public:
	virtual BOOL InitInstance();
	bool InitToolManager();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
public:
     virtual BOOL OnIdle(LONG lCount);
};

extern CVideoEditingApp theApp;
extern bool imageOpened;