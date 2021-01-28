#include "StdAfx.h"
#include "PlayControl.h"

CPlayControl*	CPlayControl::m_Instance = new CPlayControl;

void _stdcall FramePlay(HWND Window, UINT Message, UINT idEvent,
						DWORD dwTime)
{
	CPlayControl* playCtrl = CPlayControl::GetInstance();
	if(playCtrl->GetDir()){//Forward
		if(!playCtrl->End()){
			playCtrl->StepIt();
		}
		else
			KillTimer(Window,playCtrl->TimerID());
	}
	else{//Backward
		if(!playCtrl->Begin()){
			playCtrl->StepIt();
		}
		else
			KillTimer(Window,playCtrl->TimerID());
	}
	
}


CPlayControl::CPlayControl(void)
{
	m_iPos = 0;
	m_iStep = 1;
	m_iDuration = 100;
	m_bLoop = false;
}

CPlayControl::~CPlayControl(void)
{
}

CPlayControl* CPlayControl::GetInstance()
{
	return m_Instance;
}

void	CPlayControl::GoBegin()
{
	Seek(0);
}

void	CPlayControl::GoEnd()
{
	if(m_iDuration>0)
		Seek(m_iDuration-1);
}

bool	CPlayControl::Seek(int pos)
{
	m_iPos = pos;

	return Render();
}

int 	CPlayControl::GetPos()
{
	return m_iPos;
}

void	CPlayControl::PlayForward()
{
	m_direction = true;
	m_iTimerID = SetTimer(m_hWnd,1,40,FramePlay);

}

void	CPlayControl::PlayBackward()
{
	m_direction = false;
	m_iTimerID = SetTimer(m_hWnd,1,40,FramePlay);
}

void	CPlayControl::Stop()
{
	KillTimer(m_hWnd,m_iTimerID);
}

void	CPlayControl::Pause()
{
	KillTimer(m_hWnd,m_iTimerID);
}

void	CPlayControl::StepIt()
{
	m_iLastPos = m_iPos;
	m_iPos += m_direction?m_iStep:-m_iStep;
	if(m_iPos<0||m_iPos>=m_iDuration)
		m_iPos = m_iLastPos;
	Render();
}

void	CPlayControl::StepIt(bool m_dir)
{
	m_direction = m_dir;
	
	StepIt();
}

int	CPlayControl::GetDuration()
{
	return m_iDuration;
}

void	CPlayControl::SetDuration(int length)
{
	m_iDuration = length;
}

bool	CPlayControl::Render()
{
	AfxGetMainWnd()->SendMessage(WM_COMMAND_UPDATE_TIMELINE,m_iPos);

	SendMessage(m_hWnd,WM_COMMAND_RENDER,m_iPos,0);
	
	//printf("%d\n",m_iPos);

	return true;
}

void CPlayControl::setPos(int pos)
{
	m_iPos=pos;
}