#pragma once


// CParaDlg �Ի���

class CParaDlg : public CDialog
{
	DECLARE_DYNAMIC(CParaDlg)

public:
	CParaDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CParaDlg();

// �Ի�������
	enum { IDD = IDD_PRA_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	int level;
	int activeLevel;
	int winSize;
	double thrAlpha;
	double epsilon;
};
