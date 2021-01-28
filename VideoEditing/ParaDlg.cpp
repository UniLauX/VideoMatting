// ParaDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VideoEditing.h"
#include "ParaDlg.h"


// CParaDlg 对话框

IMPLEMENT_DYNAMIC(CParaDlg, CDialog)

CParaDlg::CParaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CParaDlg::IDD, pParent)
	, level(0)
	, activeLevel(0)
	, winSize(0)
	, thrAlpha(0)
	, epsilon(0)
{

}

CParaDlg::~CParaDlg()
{
}

void CParaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, level);
	DDX_Text(pDX, IDC_EDIT2, activeLevel);
	DDX_Text(pDX, IDC_EDIT3, winSize);
	DDX_Text(pDX, IDC_EDIT4, thrAlpha);
	DDX_Text(pDX, IDC_EDIT5, epsilon);
}


BEGIN_MESSAGE_MAP(CParaDlg, CDialog)
END_MESSAGE_MAP()


// CParaDlg 消息处理程序
