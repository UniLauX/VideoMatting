#if !defined(AFX_CONNECTEDCOMPONENTLABELER_H__E4CEAEC2_ABC6_4A4E_AE25_68AC93377186__INCLUDED_)
#define AFX_CONNECTEDCOMPONENTLABELER_H__E4CEAEC2_ABC6_4A4E_AE25_68AC93377186__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>
#include <atltypes.h>
#include <atlstr.h>
using namespace std;

class KConnectedComponentLabeler
{
private:
	class KNode  
	{
	public:
		KNode();
		virtual ~KNode();
		KNode*	ngNext;
		KNode*	sgNext;
		int data;
	};

public:
	class KBox  
	{
	public:
		KBox();
		virtual ~KBox();
		CPoint topLeft;
		CPoint bottomRight;
		int	   ID;
	};


private:
	class KLinkedList
	{
	public:
		void printTable();

		KNode * header;
		int  regionCount;

		void Search(int data, KNode* &p);

		void InsertData(int data);
		void InsertData(int addGroup,int searchGroup);

		KLinkedList();
		~KLinkedList();
	};
public:
	void Merge();

	int*	GetOutput();

	int		m_ObjectNumber;
	int*	m_MaskArray;

	int		m_nAreaThreshold;
	int		m_height;
	int		m_width;

	vector<KBox> m_Components;

	void	Binarize();
	void	Clear();

	void	InitConfig( int AreaThreshold );
	void	Process();
	void CheckConnectivity(unsigned char* l, int minarea=0);
	void FillHoles(int minarea);
	void	SetMask(BYTE* mask, int width, int height);

	void	saveBMP(CString str, int width , int height, int *maskArray, char mode);
	void	OnCalculateHex(int* array, int a );

	KConnectedComponentLabeler();
	virtual ~KConnectedComponentLabeler();
};

#endif // !defined(AFX_CONNECTEDCOMPONENTLABELER_H__E4CEAEC2_ABC6_4A4E_AE25_68AC93377186__INCLUDED_)