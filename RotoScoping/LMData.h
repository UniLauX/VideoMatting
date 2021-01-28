#ifndef LMDATA_H
#define  LMDATA_H
#include "Convolve.h"
#include <iostream>
struct LMData{

	bool **FixMask;	
	double **x;				// �����ϵĵ��x����	��һ��ϵ��Ϊ�����ţ��ڶ���ϵ��Ϊ����֡
	double **y;				// �����ϵĵ��y����	��һ��ϵ��Ϊ�����ţ��ڶ���ϵ��Ϊ����֡
	double **NormalX;		// �����ϵĵ�ķ�������x����	��һ��ϵ��Ϊ�����ţ��ڶ���ϵ��Ϊ����֡
	double **NormalY;		// �����ϵĵ�ķ�������x����	��һ��ϵ��Ϊ�����ţ��ڶ���ϵ��Ϊ����֡
	
	unsigned char ** R;		// ���Rͨ��ͼ	Have NumOfFrame terms, each term is an image
	unsigned char ** G;		// ���Gͨ��ͼ	Have NumOfFrame terms, each term is an image
	unsigned char ** B;		// ���Bͨ��ͼ	Have NumOfFrame terms, each term is an image
	double **Gradient;		// ����ݶ�ͼ	Have NumOfFrame terms, each term is an image
	double *M;				// Have NumOfCurvePoint terms


	double **xCp;
	double **yCp;
	double **xAp;
	double **yAp;

	int xLength;
	int NumOfFrame;
	int NumOfCurvePointPerFrame;
	int NumOfControlPoint;
	int NumOfParam;
	int Width;
	int Height;
	int IntermediateSamplePerCurve;

	double StdEI;				//�޸�
	double StdEG;
	double StdEC;
	double StdEL;
	double StdEV;

	int nEI;					//�޸�
	int nEG;
	int nEC;
	int nEL;
	int nEV;

	bool FirstTime;				//�޸�
	double* JTJ;				//����Ż�
	
	ConvolveKernel GaussKernel;
	ConvolveKernel GaussDeriKernel;

	LMData(int iNumOfControlPoint,int iNumOfFrame,int iNumOfCurvePointPerFrame,int iWidth,int iHeight,int iIntermediateSamplePerCurve,int ixLength,int iNumOfParam);
	~LMData();

	

};


#endif


