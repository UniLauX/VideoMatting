#ifndef CSPARSEMAT_H
#define CSPARSEMAT_H


#include <map>			// Map are sorted internally from lower to higher key value
#include <hash_map>


struct CRSData{
	double * val;
	int * col_ind;
	int * row_ptr;
	int NumOfElem;
	int m_Row;
	int m_Col;

	CRSData(int nRow, int nCol, int nNumOfElem)
	{
		NumOfElem = nNumOfElem;
		m_Row = nRow;
		m_Col = nCol;

		val = new double[NumOfElem];
		col_ind = new int[NumOfElem];
		row_ptr = new int[m_Row+1];
	}

	~CRSData()
	{
		delete []val;
		delete []col_ind;
		delete []row_ptr;
	}
	void PrintCRS(){
		printf("val:");
		for (int i=0;i<NumOfElem;i++)
		{
			printf("%.2f ",val[i]);
		}

		printf("\ncol_ind:");
		for (int i=0;i<NumOfElem;i++)
		{
			printf("%2d  ",col_ind[i]);
		}

		printf("\nrow_ptr:");
		for (int i=0;i<m_Row+1;i++)
		{
			printf("%2d  ",row_ptr[i]);
		}
		printf("\n");

	}
};

struct CCSData{
	double * val;
	int * row_ind;
	int * col_ptr;

	int m_Row;
	int m_Col;
	int NumOfElem;
	CCSData(int nRow, int nCol, int nNumOfElem)
	{
		NumOfElem = nNumOfElem;
		m_Row = nRow;
		m_Col = nCol;

		val = new double[NumOfElem];
		row_ind = new int[NumOfElem];
		col_ptr = new int[m_Col];
	}

	~CCSData()
	{
		delete []val;
		delete []row_ind;
		delete []col_ptr;
	}

};

// The implementation of CSparseMat should be independent of any other class.
class CSparseMat{
public:
	typedef stdext::hash_map<int,double> SparseRow;



private:
	int m_Col;
	int m_Row;
	SparseRow * Rows;
	//SparseCol * Cols;
public:
	CSparseMat(){};
	CSparseMat(int M,int N);
	~CSparseMat();
	void Create(int M,int N);

public:	
	double & operator()(int r,int c);
	bool Exist(int r,int c);

public:  //Change File Format
	void ToCRSFormat();
	void ToDenseMat();
	int GetNonZeroElemNum();

	int PrevNumOfNonZero;//深度优化
	CRSData * CRSFormatData;
	double ** DenseData;
public:  //utils
	void PrintMat();
	int GetCol()const{return m_Col;}
	int GetRow()const{return m_Row;}
	SparseRow GetRowData(int RowIndex)const { return Rows[RowIndex];}
	typedef SparseRow::iterator RowIterator;

	friend void JTJCRS(CSparseMat & JacT, double * Jac, double * JTJ);  //深度优化
	friend void JTJCRSBlock(CSparseMat & JacT, double * Jac, double * JTJ,double ** JacCOLorder, int NumOfFrame, int NumOfControlPoint);  //深度优化
};


#endif