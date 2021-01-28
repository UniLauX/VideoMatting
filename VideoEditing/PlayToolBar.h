#pragma once

#include "../PlayToolBar/toolbarex.h"
#include "../PlayToolBar/controltip.h"
#include "../PlayToolBar/SliderCtrlEx.h"

// CPlayToolBar

class CPlayToolBar : public CDialogBar
{
	DECLARE_DYNAMIC(CPlayToolBar)

public:
	CPlayToolBar();
	virtual ~CPlayToolBar();


	void SetRange(int nMax);
	void SetPos(int nPos);
	int GetPos();

protected:

	void AdjustLayout();

	CToolBarEx			m_wndPlayToolBar;
	CSliderCtrlEx		m_wndPlaySlider;
	CControlTip			m_wndTip;

	// for horizontal scroll bar
	int m_nHscrollPos;			
	int m_nHscrollMax;			
	int m_nHorzInc;				

	BOOL mNeedResize;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	afx_msg void OnPaint();
public:
	afx_msg void OnNcPaint();

	afx_msg LRESULT OnSliderMoving(WPARAM wParam, LPARAM lParam);

	afx_msg void OnPlayForward();
	afx_msg void OnPlayBackword();
	afx_msg void OnPlayStop();
	afx_msg void OnPlayToStart();
	afx_msg void OnPlayToEnd();
	afx_msg void OnPlayStepBackward();
	afx_msg void OnPlayStepForward();
};


