#pragma once


// CParaDlg 对话框

class CParaDlg : public CDialog
{
	DECLARE_DYNAMIC(CParaDlg)

public:
	CParaDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CParaDlg();

// 对话框数据
	enum { IDD = IDD_PRA_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int level;
	int activeLevel;
	int winSize;
	double thrAlpha;
	double epsilon;
};
