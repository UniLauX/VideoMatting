#pragma once

#define WM_COMMAND_RENDER	WM_USER + 1000
#define WM_COMMAND_UPDATE_TIMELINE	WM_USER + 1001

class CPlayControl
{
public:
	CPlayControl(void);
public:
	~CPlayControl(void);

public:
	bool	Seek(int pos);
	int 	GetPos();
	bool	Begin(){return m_iPos==0;};
	bool	End(){return m_iPos==m_iDuration-1;};
	void	GoBegin();
	void	GoEnd();
	void	PlayForward();
	void	PlayBackward();
	void	Stop();
	void	Pause();
	int	GetDuration();
	void	SetDuration(int length);
	bool	GetDir(){return m_direction;}
	bool	SetDir(bool m_dir){m_direction = m_dir;}
	void	SetStep(int iStep){m_iStep = iStep;}
	int	GetStep(){return m_iStep;}
	void	StepIt();
	void	StepIt(bool m_dir);
	bool	Render();

	void setPos(int pos);
	
	void Attach(HWND hWnd){m_hWnd = hWnd;}
	HWND GetHWnd(){return m_hWnd;}
	UINT_PTR TimerID(){return m_iTimerID;};

	static CPlayControl* GetInstance();

	static CPlayControl* m_Instance;

protected:
	int m_iLastPos;
	int m_iPos;
	int m_iDuration;
	int m_iStep;
	bool   m_bLoop;
	bool   m_direction;
	UINT_PTR m_iTimerID;
	HWND	m_hWnd;	
};
