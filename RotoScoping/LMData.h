#ifndef LMDATA_H
#define  LMDATA_H
#include "Convolve.h"
#include <iostream>
struct LMData{

	bool **FixMask;	
	double **x;				// 曲线上的点的x坐标	第一个系数为点的序号，第二个系数为所在帧
	double **y;				// 曲线上的点的y坐标	第一个系数为点的序号，第二个系数为所在帧
	double **NormalX;		// 曲线上的点的法向量的x分量	第一个系数为点的序号，第二个系数为所在帧
	double **NormalY;		// 曲线上的点的法向量的x分量	第一个系数为点的序号，第二个系数为所在帧
	
	unsigned char ** R;		// 存放R通道图	Have NumOfFrame terms, each term is an image
	unsigned char ** G;		// 存放G通道图	Have NumOfFrame terms, each term is an image
	unsigned char ** B;		// 存放B通道图	Have NumOfFrame terms, each term is an image
	double **Gradient;		// 存放梯度图	Have NumOfFrame terms, each term is an image
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

	double StdEI;				//修改
	double StdEG;
	double StdEC;
	double StdEL;
	double StdEV;

	int nEI;					//修改
	int nEG;
	int nEC;
	int nEL;
	int nEV;

	bool FirstTime;				//修改
	double* JTJ;				//深度优化
	
	ConvolveKernel GaussKernel;
	ConvolveKernel GaussDeriKernel;

	LMData(int iNumOfControlPoint,int iNumOfFrame,int iNumOfCurvePointPerFrame,int iWidth,int iHeight,int iIntermediateSamplePerCurve,int ixLength,int iNumOfParam);
	~LMData();

	

};


#endif


