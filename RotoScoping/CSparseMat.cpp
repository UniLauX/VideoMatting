#include "../RotoScoping/stdafx.h"
#include "../RotoScoping/CSparseMat.h"
#include <assert.h>
#include <iostream>
using namespace std;

CSparseMat::CSparseMat(int M,int N){
	m_Col = N;
	m_Row = M;
	CRSFormatData = NULL;
	DenseData = NULL;
	PrevNumOfNonZero = 0;
	Create(M,N);
}

void CSparseMat::Create(int M,int N){
	m_Col = N;
	m_Row = M;
	CRSFormatData = NULL;
	DenseData = NULL;
	PrevNumOfNonZero = 0;
	Rows = new SparseRow[M];
}
CSparseMat::~CSparseMat(){
	printf("Inside ~CSparseMat()\n");
	printf("Num Of Row = %d\n",m_Row);
	for (int i=0;i<m_Row;i++)
	{
		Rows[i].clear();
	}
	delete [] Rows;


	delete CRSFormatData;

	if (DenseData!=NULL)
	{
		for (int i=0;i<m_Col;i++)
		{
			delete [] DenseData[i];
		}
		delete []DenseData;
	}
	

	printf("End of ~CSparseMat()\n");
}
double & CSparseMat::operator()(int r,int c){
	assert(r<m_Row);
	assert(c<m_Col);
	return Rows[r][c];
}

bool CSparseMat::Exist(int r,int c){
	SparseRow::iterator it = Rows[r].find(c);
	if (it==Rows[r].end())
	{
		return false;
	}
	return true;
}


void CSparseMat::PrintMat(){

	SparseRow::iterator it;

	printf("Matrix=\n");
	for (int i=0;i<m_Row;i++)
	{	printf("Row%d: ",i);
	for (it = Rows[i].begin();it!=Rows[i].end();it++)
	{
		printf("R(%3d,%3d):%5.5f  ",i,it->first,it->second);
	}
	printf("\n");
	}

}



void CSparseMat::ToCRSFormat(){
	assert(Rows != NULL);

	int NumOfNonZero = GetNonZeroElemNum();			//深度优化
		
	if (NumOfNonZero > PrevNumOfNonZero)			//深度优化
	{
		PrevNumOfNonZero = NumOfNonZero;
		delete CRSFormatData;						//深度优化    ??对吗??
		CRSFormatData = new CRSData(m_Row,m_Col,NumOfNonZero);
		printf("new\n");
	}


	int	ValIndex = 0;
	RowIterator Rit;
	for (int r=0; r<m_Row; r++)
	{
		CRSFormatData->row_ptr[r] = ValIndex;
		for (Rit = Rows[r].begin(); Rit != Rows[r].end(); Rit++)
		{
			CRSFormatData->val[ValIndex] = (*Rit).second;
			CRSFormatData->col_ind[ValIndex] = (*Rit).first;
			ValIndex++;
		}
	}
	CRSFormatData->row_ptr[m_Row] = ValIndex;
}


int CSparseMat::GetNonZeroElemNum()
{
	RowIterator Rit;
	int NumOfNonZero = 0;
	for (int r=0;r<m_Row;r++)
	{
		NumOfNonZero += Rows[r].size();
	}
	return NumOfNonZero;

}

void CSparseMat::ToDenseMat(){
	printf("To Dense Mat\n");
	

	if (DenseData == NULL)
	{
		DenseData = new double *[m_Col];   //这里比较特殊,是column order.
		for (int i=0;i<m_Col;i++)
		{
			DenseData[i] = new double [m_Row];
			memset(DenseData[i],0,sizeof(double)*m_Row);
		}
	}



	for (int r=0;r<m_Row;r++)
	{
		for (RowIterator Rit = Rows[r].begin();Rit != Rows[r].end(); Rit++)
		{
			DenseData[Rit->first][r] = Rit->second;
		}
	}

}


