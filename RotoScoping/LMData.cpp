#include "../RotoScoping/stdafx.h"
#include "../RotoScoping/LMData.h"

LMData::LMData(
	int iNumOfControlPoint,
	int iNumOfFrame,
	int iNumOfCurvePointPerFrame,
	int iWidth,
	int iHeight,
	int iIntermediateSamplePerCurve,
	int ixLength,
	int iNumOfParam)
	:NumOfControlPoint(iNumOfControlPoint),
	NumOfFrame(iNumOfFrame),
	NumOfCurvePointPerFrame(iNumOfCurvePointPerFrame),
	Width(iWidth),
	Height(iHeight),
	IntermediateSamplePerCurve(iIntermediateSamplePerCurve),
	xLength(ixLength),
	NumOfParam(iNumOfParam)
{

	std::cout << "NumOfParam = " << NumOfParam << std::endl;	
	std::cout << "xLength = " << xLength << std::endl;
	std::cout << "NumOfFrame = " << NumOfFrame << std::endl;
	std::cout << "NumOfControlPoint = " << NumOfControlPoint << std::endl;
	std::cout << "IntermediateSamplePerCurve = " << IntermediateSamplePerCurve << std::endl;


	xCp = new double *[NumOfControlPoint];
	yCp = new double *[NumOfControlPoint];
	xAp = new double *[NumOfControlPoint];
	yAp = new double *[NumOfControlPoint];
	FixMask = new bool *[NumOfControlPoint];
	for (int p=0;p<NumOfControlPoint;p++)
	{
		xCp[p] = new double[NumOfFrame];
		yCp[p] = new double[NumOfFrame];
		xAp[p] = new double[NumOfFrame];
		yAp[p] = new double[NumOfFrame];
		FixMask[p] = new bool[NumOfFrame];
		memset(FixMask[p],false,sizeof(bool)*NumOfFrame);
	}


	x = new double *[NumOfCurvePointPerFrame];
	y = new double *[NumOfCurvePointPerFrame];
	NormalX = new double *[NumOfCurvePointPerFrame];
	NormalY = new double *[NumOfCurvePointPerFrame];

	for (int i=0;i<NumOfCurvePointPerFrame;i++)
	{
		x[i] = new double[NumOfFrame];
		y[i] = new double[NumOfFrame];
		NormalX[i] = new double[NumOfFrame];
		NormalY[i] = new double[NumOfFrame];

	}

	R = new unsigned char *[NumOfFrame];
	G = new unsigned char *[NumOfFrame];
	B = new unsigned char *[NumOfFrame];
	Gradient = new double *[NumOfFrame];
	for (int i=0;i<NumOfFrame;i++)
	{
		R[i] = new unsigned char[Width*Height];
		G[i] = new unsigned char[Width*Height];
		B[i] = new unsigned char[Width*Height];
		Gradient[i] = new double[Width*Height];
	}

	M = new double [NumOfCurvePointPerFrame];


	FirstTime = true;   // 修改


	JTJ = new double[NumOfParam * NumOfParam];			//深度优化 
	memset(JTJ,0,sizeof(double)*NumOfParam * NumOfParam);		//Block 优化	
	
	
	GenerateKernel(1.0,GaussKernel,GaussDeriKernel);

}
LMData::~LMData()
{
	printf("~LMData Called!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	for (int i=0;i<NumOfControlPoint;i++)
	{
		delete []xCp[i];
		delete []yCp[i];
		delete []xAp[i];
		delete []yAp[i];
		delete []FixMask[i];
	}
	delete[]xCp;
	delete[]yCp;
	delete[]xAp;
	delete[]yAp;
	delete[]FixMask;


	for (int i=0;i<NumOfCurvePointPerFrame;i++)
	{
		delete []x[i];
		delete []y[i];
		delete []NormalX[i];
		delete []NormalY[i];
	}
	delete[]x;
	delete[]y;
	delete[]NormalX;
	delete[]NormalY;


	for (int i=0;i<NumOfFrame;i++)
	{
		delete []R[i];
		delete []B[i];
		delete []G[i];
		delete []Gradient[i];
	}
	delete []R;
	delete []B;
	delete []G;
	delete []Gradient;

	delete []M;

	delete []JTJ;  //深度优化
}