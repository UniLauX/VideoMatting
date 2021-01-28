#include "../RotoScoping/Bezier.h"
#include "../RotoScoping/stdafx.h"
#include "../RotoScoping/LMFunc.h"
#include "../RotoScoping/PublicStructures.h"
#include "../RotoScoping/Configuration.h"
#include "cv.h"
#include "cxcore.h"
#include "cvaux.h"
#include "highgui.h"
#include <vector>
#include <fstream>
#include <iostream>
#include "../RotoScoping/CSparseMat.h"
#include "../RotoScoping/Convolve.h"
#include "../RotoScoping/LMData.h"
#include "../RotoScoping/MatMulFunc.h"
#include <time.h>
using namespace  std;

CSparseMat dXcurvedXcontrol;			//曲线上每个点相对于控制点和调整点的雅阁比矩阵
BlockMatProxy JacBlockMat;				//将雅阁比矩阵按分块的稀疏存储



//这个仅仅是权宜之计，无法解决有很大角度变化的情况
//根本的方法是确定内部方向
void AlighToSameSideOfEdge(double Normalx1, double Normaly1, double & Normalx2,double & Normaly2)
{

	if (Normalx1 * Normalx2 + Normaly1 * Normaly2 < 0.0)
	{
		Normalx2 *= -1;
		Normaly2 *= -1;
	}

}

inline bool IsZero(double Val)
{
	static const double ZERO = 1e-8;
	if (Val < ZERO && Val > -ZERO)
	{
		return true;
	}
	return false;
}
double CalculateVar(int n,double * x)
{
	double mean = 0.0;
	double var = 0.0;
	for (int i=0 ;i<n;i++)
	{
		mean += x[i];
	}
	mean /= n;


	for (int i=0;i<n;i++)
	{
		var += (x[i] - mean)*(x[i] - mean);
	}

	var/= n;
	return var;
}

double CalculateStdVar(int n, double * x)
{
	double Var = CalculateVar(n,x);
	return sqrt(Var);
}

void ScaleVector(int n, double * x, double scalefactor)
{
	for (int i=0;i<n;i++)
	{
		x[i] *= scalefactor;
	}
}



// m is the number of params, n is the number of measurement
void lmfunc(double *p, double *x, int NumOfParams, int NumOfMeasurement, void *adata)
{
	LMData * globs = (LMData *) adata;

	int pcounter = 0;  // 用来记住目前到达的p的序号....
	
	double sum=0.0;
	
																					//printf("globs->NumOfFrame =%d\n",globs->NumOfFrame);;
	/************************************************************************************************************************
						Read and update the coordinate of the Control Points and Adjust Points
	************************************************************************************************************************/
	for (int f=0;f<globs->NumOfFrame;f++)
	{
		//printf("f = %d\n",f);
		for (int cpid=0;cpid<globs->NumOfControlPoint;cpid++)
		{
			if (!globs->FixMask[cpid][f]) // is not fixed, then we use the info inside p and update the global info
			{
				globs->xCp[cpid][f] = p[pcounter++];
				globs->yCp[cpid][f] = p[pcounter++];
				globs->xAp[cpid][f] = p[pcounter++];
				globs->yAp[cpid][f] = p[pcounter++];
				
			}
		}

	}

	/************************************************************************************************************************
										Update the coordinates of the Curve Points
	************************************************************************************************************************/	
	


	int Opx,Opy;
	CvPoint2D32f *CurvePoint = new CvPoint2D32f[globs->IntermediateSamplePerCurve+2];				//这里是加2，因为包括了第一个和最后一个点
	Normal *normals= new Normal[globs->IntermediateSamplePerCurve+2];	//这里是加2，因为包括了第一个和最后一个点

	IplImage * NormalImage = cvCreateImage(cvSize(globs->Width,globs->Height),8,3);

	for (int f=0;f<globs->NumOfFrame;f++)
	{
		int CurvePointId=0;

		for (int cpid = 0;cpid<globs->NumOfControlPoint - 1;cpid++)
		{	
			//globs->x[CurvePointId][f] = globs->xCp[cpid][f];			// the first point is the control point
			//globs->y[CurvePointId][f] = globs->yCp[cpid][f];
			//CurvePointId++;

			Opx = globs->xCp[cpid+1][f] - (globs->xAp[cpid+1][f] - globs->xCp[cpid+1][f]);
			Opy = globs->yCp[cpid+1][f] - (globs->yAp[cpid+1][f] - globs->yCp[cpid+1][f]);

			//Bezier::GetInstance()->GenerateCurve(cvPoint(globs->xCp[cpid][f],globs->yCp[cpid][f]),
			//									 cvPoint(globs->xAp[cpid][f],globs->yAp[cpid][f]),
			//									 cvPoint(Opx,Opy),
			//									 cvPoint(globs->xCp[cpid+1][f],globs->yCp[cpid+1][f]),
			//									 globs->IntermediateSamplePerCurve,
			//									 CurvePoint);
			Bezier::GetInstance()->GenerateCurveAndNormal(cvPoint2D32f(globs->xCp[cpid][f],globs->yCp[cpid][f]),
				cvPoint2D32f(globs->xAp[cpid][f],globs->yAp[cpid][f]),
				cvPoint2D32f(Opx,Opy),
				cvPoint2D32f(globs->xCp[cpid+1][f],globs->yCp[cpid+1][f]),
				globs->IntermediateSamplePerCurve,
				CurvePoint,
				normals);

			for (int i=0;i<globs->IntermediateSamplePerCurve+1;i++)
			{
				globs->x[CurvePointId][f] = CurvePoint[i].x;
				globs->y[CurvePointId][f] = CurvePoint[i].y;
				globs->NormalX[CurvePointId][f] = normals[i].NormalX;
				globs->NormalY[CurvePointId][f] = normals[i].NormalY;
				CurvePointId++;
			}


		}			


		// And don't forget the last control point
		globs->x[CurvePointId][f] = globs->xCp[globs->NumOfControlPoint-1][f];
		globs->y[CurvePointId][f] = globs->yCp[globs->NumOfControlPoint-1][f];
		globs->NormalX[CurvePointId][f] = normals[globs->IntermediateSamplePerCurve+1].NormalX;
		globs->NormalY[CurvePointId][f] = normals[globs->IntermediateSamplePerCurve+1].NormalY;


		//cvZero(NormalImage);
		//for (int i=0;i<globs->NumOfCurvePointPerFrame;i++)
		//{
		//	printf(" globs->NormalX[i][f]=%f, globs->NormalY[i][f]=%f\n", globs->NormalX[i][f], globs->NormalY[i][f]);
		//	cvLine(NormalImage,cvPoint(globs->x[i][f],globs->y[i][f]),cvPoint(globs->x[i][f] + 10 * globs->NormalX[i][f],globs->y[i][f] + 10 * globs->NormalY[i][f]),cvScalar(255,0,255),1);
		//	cvCircle(NormalImage,cvPoint(globs->x[i][f],globs->y[i][f]),2,cvScalar(0,255,0),-1);			
		//}
		//cvShowImage("NormalImage",NormalImage);
		//cvWaitKey(0);
		//printf("\n\n\n\n");
	}
	delete [] CurvePoint;


	register int xcounter=0;
	

		/************************************************************************************************************************
	            					                     The EV Term    
											 2 * NumOfCurvePoint * (NumOfFrame -1)
		************************************************************************************************************************/



		for (int f = 0; f<globs->NumOfFrame-1 ; f++)
		{
			for (int pid = 0; pid<globs->NumOfCurvePointPerFrame ; pid++)
			{
				x[xcounter++] = (globs->x[pid][f] - globs->x[pid][f+1]);    //修改
			}

		}
		for (int f = 0; f<globs->NumOfFrame-1 ; f++)
		{
			for (int pid = 0; pid<globs->NumOfCurvePointPerFrame ; pid++)
			{
				x[xcounter++] = (globs->y[pid][f] - globs->y[pid][f+1]);    
			}

		}
	/************************************************************************************************************************
														The EG Term    
												  NumOfFrame * NumOfCurvePoint
	************************************************************************************************************************/
	
				
	{
		int index;
		double KMax = 1200;   //KMAX = sqrt(255.0*255.0*2.0)*3 = 1081
		for (int f= 0;f<globs->NumOfFrame;f++)
		{
			for (int pid = 0;pid < globs->NumOfCurvePointPerFrame;pid++)
			{
				index = int(globs->y[pid][f]) * globs->Width + int(globs->x[pid][f]);
				x[xcounter++] = ( (KMax - (globs->Gradient[f])[index] ) / globs->M[pid]);    //修改
				
			}
		}
	}


		/************************************************************************************************************************
														The EL Term    
												(NumOfCurvePoint-1) * (NumOfFrame-1)
	************************************************************************************************************************/


	double disti,distip1;
	for (int f=0;f<globs->NumOfFrame-1;f++)
	{
		for (int pid=0;pid<globs->NumOfCurvePointPerFrame-1;pid++)
		{
			disti = (globs->x[pid][f] - globs->x[pid+1][f])*(globs->x[pid][f] - globs->x[pid+1][f]) + 
					(globs->y[pid][f] - globs->y[pid+1][f])*(globs->y[pid][f] - globs->y[pid+1][f]); 
			
			distip1 = (globs->x[pid][f+1] - globs->x[pid+1][f+1]) * (globs->x[pid][f+1] - globs->x[pid+1][f+1]) + 
					  (globs->y[pid][f+1] - globs->y[pid+1][f+1]) * (globs->y[pid][f+1] - globs->y[pid+1][f+1]);
			x[xcounter++] = (disti -distip1);			//修改

		}


	}


	
	/************************************************************************************************************************
														The EC Term    
											2 * (NumOfCurvePoint-2) * (NumOfFrame-1)
	************************************************************************************************************************/


	double term1,term2;
	for (int f=0;f<globs->NumOfFrame-1;f++)
	{
		//printf("EC: f=%d\n",f);
		for (int pid=0;pid<globs->NumOfCurvePointPerFrame-2;pid++)
		{
			
			term1 = globs->x[pid][f]   - 2* globs->x[pid+1][f]   + globs->x[pid+2][f];
			term2 = globs->x[pid][f+1] - 2* globs->x[pid+1][f+1] + globs->x[pid+2][f+1];

			x[xcounter++] = (term1 - term2);				//修改

		}
	}

	for (int f=0;f<globs->NumOfFrame-1;f++)
	{
		//printf("EC: f=%d\n",f);
		for (int pid=0;pid<globs->NumOfCurvePointPerFrame-2;pid++)
		{

			term1 = globs->y[pid][f]   - 2* globs->y[pid+1][f]   + globs->y[pid+2][f];
			term2 = globs->y[pid][f+1] - 2* globs->y[pid+1][f+1] + globs->y[pid+2][f+1];

			x[xcounter++] = (term1 - term2);				//修改


		}
	}

		/************************************************************************************************************************
															The EI Term    
												NumOfCurvePoint * (NumOfFrame-1) * (2K+1) *3
		************************************************************************************************************************/
	
		{
	
			double dx,dy,norm,NormalX1,NormalY1,NormalX2,NormalY2;
			double x0,y0,x1,y1,x2,y2,x3,y3;
			double NewX1,NewX2,NewY1,NewY2;
			int index1,index2;
			double t=0.0,step = 1.0/(globs->IntermediateSamplePerCurve + 1.0);
			int CurrentTIndex;
			int CurrentCurveIndex;
			for (int f =0;f<globs->NumOfFrame-1;f++)
			{
				//cvZero(Normal);
				for (int pid = 0;pid<globs->NumOfCurvePointPerFrame-1;pid++)
				{

					CurrentTIndex = pid%(globs->IntermediateSamplePerCurve+1);
					CurrentCurveIndex = pid/(globs->IntermediateSamplePerCurve+1);
					t = CurrentTIndex * step;
	
					x0 = globs->xCp[CurrentCurveIndex][f];
					y0 = globs->yCp[CurrentCurveIndex][f];
					x1 = globs->xAp[CurrentCurveIndex][f];
					y1 = globs->yAp[CurrentCurveIndex][f];
	
					x2 = globs->xAp[CurrentCurveIndex+1][f];
					y2 = globs->yAp[CurrentCurveIndex+1][f];
					x3 = globs->xCp[CurrentCurveIndex+1][f];
					y3 = globs->yCp[CurrentCurveIndex+1][f];
					dx = 3 * x1 * (t - 1)*(t - 1) - 3*x0*(t - 1)*(t - 1) - 6*t*x2 + 12*t*x3 + 9*t*t*x2 - 15*t*t*x3 + 3*t*x1*(2*t - 2);
					dy = 3 * y1 * (t - 1)*(t - 1) - 3*y0*(t - 1)*(t - 1) - 6*t*y2 + 12*t*y3 + 9*t*t*y2 - 15*t*t*y3 + 3*t*y1*(2*t - 2);
	
					norm = sqrt(dx*dx + dy * dy);
					NormalX1 = -dy/norm;
					NormalY1 = dx/norm;
	
					x0 = globs->xCp[CurrentCurveIndex][f+1];
					y0 = globs->yCp[CurrentCurveIndex][f+1];
					x1 = globs->xAp[CurrentCurveIndex][f+1];
					y1 = globs->yAp[CurrentCurveIndex][f+1];
	
					x2 = globs->xAp[CurrentCurveIndex+1][f+1];
					y2 = globs->yAp[CurrentCurveIndex+1][f+1];
					x3 = globs->xCp[CurrentCurveIndex+1][f+1];
					y3 = globs->yCp[CurrentCurveIndex+1][f+1];
					dx = 3 * x1 * (t - 1)*(t - 1) - 3*x0*(t - 1)*(t - 1) - 6*t*x2 + 12*t*x3 + 9*t*t*x2 - 15*t*t*x3 + 3*t*x1*(2*t - 2);
					dy = 3 * y1 * (t - 1)*(t - 1) - 3*y0*(t - 1)*(t - 1) - 6*t*y2 + 12*t*y3 + 9*t*t*y2 - 15*t*t*y3 + 3*t*y1*(2*t - 2);
	
	
	
					norm = sqrt(dx*dx + dy * dy);
					NormalX2 = -dy/norm;
					NormalY2 = dx/norm;
	
	
					AlighToSameSideOfEdge(NormalX1,NormalY1,NormalX2,NormalY2);    //修改
	
					for (int k=-K;k<=K;k++)														//修改
					{
						NewX1 = globs->x[pid][f] + k*NormalX1;
						NewY1 = globs->y[pid][f] + k*NormalY1;
	
						NewX2 = globs->x[pid][f+1] + k*NormalX2;
						NewY2 = globs->y[pid][f+1] + k*NormalY2;
	
						index1 = int(NewY1)*globs->Width + int(NewX1);		//这样是对的
						index2 = int(NewY2)*globs->Width + int(NewX2);
	
	
						x[xcounter++] = (int((globs->R[f])[ index1]) - int((globs->R[f+1])[ index2]));   //???不成功???
						x[xcounter++] = (int((globs->G[f])[ index1]) - int((globs->G[f+1])[ index2]));
						x[xcounter++] = (int((globs->B[f])[ index1]) - int((globs->B[f+1])[ index2]));
	

					}
	
				}
	
				// the last point 
				t = 1.0;
				x2 = globs->xAp[globs->NumOfControlPoint-1][f];
				y2 = globs->yAp[globs->NumOfControlPoint-1][f];
				x3 = globs->xCp[globs->NumOfControlPoint-1][f];
				y3 = globs->yCp[globs->NumOfControlPoint-1][f];
				dx =  - 6*x2 + 12*x3 + 9*x2 - 15*x3 ;
				dy =  - 6*y2 + 12*y3 + 9*y2 - 15*y3 ;
	
				norm = sqrt(dx*dx + dy * dy);
				NormalX1 = -dy/norm;
				NormalY1 = dx/norm;
	
				x2 = globs->xAp[globs->NumOfControlPoint-1][f+1];
				y2 = globs->yAp[globs->NumOfControlPoint-1][f+1];
				x3 = globs->xCp[globs->NumOfControlPoint-1][f+1];
				y3 = globs->yCp[globs->NumOfControlPoint-1][f+1];
				dx =  - 6*x2 + 12*x3 + 9*x2 - 15*x3 ;
				dy =  - 6*y2 + 12*y3 + 9*y2 - 15*y3 ;
	
				norm = sqrt(dx*dx + dy * dy);
				NormalX2 = -dy/norm;
				NormalY2 = dx/norm;
	
	
				AlighToSameSideOfEdge(NormalX1,NormalY1,NormalX2,NormalY2);    //修改


				for (int k=-K;k<=K;k++)														//修改
				{
					NewX1 = globs->x[globs->NumOfCurvePointPerFrame-1][f] + k*NormalX1;
					NewY1 = globs->y[globs->NumOfCurvePointPerFrame-1][f] + k*NormalY1;
	
					NewX2 = globs->x[globs->NumOfCurvePointPerFrame-1][f+1] + k*NormalX2;
					NewY2 = globs->y[globs->NumOfCurvePointPerFrame-1][f+1] + k*NormalY2;
	
					index1 = int(NewY1)*globs->Width + int(NewX1);
					index2 = int(NewY2)*globs->Width + int(NewX2);
	
					x[xcounter++] = (int((globs->R[f])[ index1]) - int((globs->R[f+1])[ index2]));
					x[xcounter++] = (int((globs->G[f])[ index1]) - int((globs->G[f+1])[ index2]));
					x[xcounter++] = (int((globs->B[f])[ index1]) - int((globs->B[f+1])[ index2]));
	

				}
			}
		}
	
	
	
	    if (globs->FirstTime)
	    {
			double StdEV = CalculateStdVar(globs->nEV, x);						//修改
			printf("StdEV = %f\n",StdEV);
			globs->StdEV = StdEV;												//修改

			double StdEG = CalculateStdVar(globs->nEG, x + globs->nEV);			//修改
			printf("StdEG = %f\n",StdEG);
			globs->StdEG = StdEG;												//修改

			double StdEL = CalculateStdVar(globs->nEL, x + globs->nEV + globs->nEG);						//修改
			printf("StdEL = %f\n",StdEL);
			globs->StdEL = StdEL;												//修改

			double StdEC = CalculateStdVar(globs->nEC, x + globs->nEV + globs->nEG + globs->nEL);						//修改
			printf("StdEC = %f\n",StdEC);
			globs->StdEC = StdEC;												//修改


			double StdEI = CalculateStdVar(globs->nEI , x + globs->nEV + globs->nEG + globs->nEL + globs->nEC);			//修改
			printf("StdEI = %f\n",StdEI);
			globs->StdEI = StdEI;												//修改


			
	    }
		
		ScaleVector(globs->nEV, x, 1.0 / globs->StdEV);    // EV
		ScaleVector(globs->nEG, x + globs->nEV, 1.0 / globs->StdEG);    // EG
		ScaleVector(globs->nEL, x + globs->nEV + globs->nEG, 1.0 / globs->StdEL);    // EL
		ScaleVector(globs->nEC, x + globs->nEV + globs->nEG + globs->nEL ,1.0 / globs->StdEC);  //EC	
		ScaleVector(globs->nEI, x + globs->nEV + globs->nEG + globs->nEL + globs->nEC,1.0 / globs->StdEI);  //EI		//修改 
		

		cout << "End Of lmfunc" << endl;
		
}

void InitDXcurvedDXcontrol(LMData * globs) 
{
	/******************************************************************************************************************************************************
																	d(Xcurve)/d(Xcontrol&adjust)
	------------------------------------------------------------------------------------------------------------------------------------------------------------
				frame1: xc1,yc1,xa1,ya1.............|    frame2 : ...................
	------------------------------------------------------------------------------------------------------------------------------------------------------------

	******************************************************************************************************************************************************/

	
	int ElemPerFrame = 4*globs->NumOfControlPoint;		
	int FakeNumOfParam = (4 * globs->NumOfControlPoint * globs->NumOfFrame);										//这里先假设第一和最后一帧都是未知的
	//创建这个稀疏矩阵
	dXcurvedXcontrol.Create(2 * globs->NumOfCurvePointPerFrame * globs->NumOfFrame , FakeNumOfParam);


	double step = 1.0/(globs->IntermediateSamplePerCurve+1.0);
	double t=0.0;
	int xyCounter=0;
	//cout << "globs->NumOfControlPoint" <<globs->NumOfControlPoint<< "   NumOfParma = " << globs->NumOfParam << "NumOfCurvePointPerFrame = " << globs->NumOfCurvePointPerFrame << endl; 
	for (int f=0;f<globs->NumOfFrame;f++)// For now suppose the first and the second frames are unknowns params.
	{
		if (f==0)			//将第一帧和最后一帧的导数都设置为0
		{
			xyCounter+= (2 * globs->NumOfCurvePointPerFrame);
		}
		else if (f==globs->NumOfFrame-1)
		{
			break;
		}
		else{
			for (int cpid = 0;cpid<globs->NumOfControlPoint - 1;cpid++)
			{	// (1-t)*(1-t)*(1-t) * x0 + 3*(1-t)*(1-t)*t * x1 + 3*(1-t)*t*t * (2*x3-x2)  + t*t*t * x3
				t=0.0;
				for (int i=0;i<globs->IntermediateSamplePerCurve+1;i++,t+=step)
				{
					//x0 : xControlPoint1

					dXcurvedXcontrol(xyCounter,ElemPerFrame * f + 4*cpid) = (1-t)*(1-t)*(1-t);

					//x1 : xAdjustPoint1

					dXcurvedXcontrol(xyCounter,ElemPerFrame * f + 4*cpid + 2) = 3*(1-t)*(1-t)*t;

					//x2 : xAdjustPoint2

					dXcurvedXcontrol(xyCounter,ElemPerFrame * f + 4*cpid + 6) = -3*(1-t)*t*t;

					//x3 : xControlPoint2

					dXcurvedXcontrol(xyCounter,ElemPerFrame * f + 4*cpid + 4) = 6*(1-t)*t*t + t*t*t;  

					xyCounter++;

					//y0 : yControlPoint1

					dXcurvedXcontrol(xyCounter,ElemPerFrame * f + 4*cpid + 1) = (1-t)*(1-t)*(1-t);

					//y1 : yAdjustPoint1

					dXcurvedXcontrol(xyCounter,ElemPerFrame * f + 4*cpid + 3) = 3*(1-t)*(1-t)*t;

					//y2 : xAdjustPoint2

					dXcurvedXcontrol(xyCounter,ElemPerFrame * f + 4*cpid + 7) = -3*(1-t)*t*t;

					//y3 : xControlPoint2

					dXcurvedXcontrol(xyCounter,ElemPerFrame * f + 4*cpid + 5) = 6*(1-t)*t*t + t*t*t;  

					xyCounter++;

				}	
			}
			int cpid = globs->NumOfControlPoint-2;

			//x3 : xControlPoint2

			dXcurvedXcontrol(xyCounter,ElemPerFrame * f + 4*cpid + 4) = 1.0;
			xyCounter++;


			//y3 : xControlPoint2

			dXcurvedXcontrol(xyCounter,ElemPerFrame * f + 4*cpid + 5) = 1.0;
			xyCounter++;
		}
	}
}

void FirtstTimeDo( LMData * globs ) 
{

	/*****************************************************************************
	Block Mat Proxy，用来存储Jac
	*****************************************************************************/

	clock_t start, stop; 
	double duration; 

	start = clock(); 
	InitializeBlockMat(globs->NumOfFrame,globs->NumOfParam,globs->NumOfControlPoint,globs,globs->xLength,JacBlockMat);
	stop = clock(); 
	duration = ((double)(stop - start))/CLK_TCK;	
	printf("InitializeBlockMat duration=%f\n",duration);


	/*****************************************************************************
	DXcurvedDXcontrol，用来存储曲线上的各个点相对于控制点的导数
	*****************************************************************************/

	start = clock();
	InitDXcurvedDXcontrol(globs);
	stop = clock(); 
	duration = ((double)(stop - start))/CLK_TCK;	
	printf("InitDXcurvedDXcontrol duration=%f\n",duration);
}
void lmjacfunc(double *p, double *jac, int m, int n, void *adata)
{   
	LMData * globs = (LMData *) adata;
	
	int pcounter = 0;						// 用来记住目前到达的p的序号....
	
	
																					//printf("globs->NumOfFrame =%d\n",globs->NumOfFrame);;
	/************************************************************************************************************************
						Read and update the coordinate of the Control Points and Adjust Points
	************************************************************************************************************************/
	for (int f=0;f<globs->NumOfFrame;f++)
	{
		//printf("f = %d\n",f);
		for (int cpid=0;cpid<globs->NumOfControlPoint;cpid++)
		{
			if (!globs->FixMask[cpid][f]) // is not fixed, then we use the info inside p and update the global info
			{
				globs->xCp[cpid][f] = p[pcounter++];
				globs->yCp[cpid][f] = p[pcounter++];
				globs->xAp[cpid][f] = p[pcounter++];
				globs->yAp[cpid][f] = p[pcounter++];
				//printf("cpid = %d, x = %f, y =%f \n",cpid,globs->x[cpid][f],globs->y[cpid][f]);
			}
		}
		//system("pause");
	}
	
	/************************************************************************************************************************
										Update the coordinates of the Curve Points
	************************************************************************************************************************/	
	

	int Opx,Opy;
	CvPoint2D32f *CurvePoint = new CvPoint2D32f[globs->IntermediateSamplePerCurve+2];				//这里是加2，因为包括了第一个和最后一个点
	Normal *normals= new Normal[globs->IntermediateSamplePerCurve+2];	//这里是加2，因为包括了第一个和最后一个点

	IplImage * NormalImage = cvCreateImage(cvSize(globs->Width,globs->Height),8,3);

	for (int f=0;f<globs->NumOfFrame;f++)
	{
		int CurvePointId=0;

		for (int cpid = 0;cpid<globs->NumOfControlPoint - 1;cpid++)
		{	
			//globs->x[CurvePointId][f] = globs->xCp[cpid][f];			// the first point is the control point
			//globs->y[CurvePointId][f] = globs->yCp[cpid][f];
			//CurvePointId++;

			Opx = globs->xCp[cpid+1][f] - (globs->xAp[cpid+1][f] - globs->xCp[cpid+1][f]);
			Opy = globs->yCp[cpid+1][f] - (globs->yAp[cpid+1][f] - globs->yCp[cpid+1][f]);

			//Bezier::GetInstance()->GenerateCurve(cvPoint(globs->xCp[cpid][f],globs->yCp[cpid][f]),
			//									 cvPoint(globs->xAp[cpid][f],globs->yAp[cpid][f]),
			//									 cvPoint(Opx,Opy),
			//									 cvPoint(globs->xCp[cpid+1][f],globs->yCp[cpid+1][f]),
			//									 globs->IntermediateSamplePerCurve,
			//									 CurvePoint);
			Bezier::GetInstance()->GenerateCurveAndNormal(cvPoint2D32f(globs->xCp[cpid][f],globs->yCp[cpid][f]),
				cvPoint2D32f(globs->xAp[cpid][f],globs->yAp[cpid][f]),
				cvPoint2D32f(Opx,Opy),
				cvPoint2D32f(globs->xCp[cpid+1][f],globs->yCp[cpid+1][f]),
				globs->IntermediateSamplePerCurve,
				CurvePoint,
				normals);

			for (int i=0;i<globs->IntermediateSamplePerCurve+1;i++)
			{
				globs->x[CurvePointId][f] = CurvePoint[i].x;
				globs->y[CurvePointId][f] = CurvePoint[i].y;
				globs->NormalX[CurvePointId][f] = normals[i].NormalX;
				globs->NormalY[CurvePointId][f] = normals[i].NormalY;
				CurvePointId++;
			}


		}			


		// And don't forget the last control point
		globs->x[CurvePointId][f] = globs->xCp[globs->NumOfControlPoint-1][f];
		globs->y[CurvePointId][f] = globs->yCp[globs->NumOfControlPoint-1][f];
		globs->NormalX[CurvePointId][f] = normals[globs->IntermediateSamplePerCurve+1].NormalX;
		globs->NormalY[CurvePointId][f] = normals[globs->IntermediateSamplePerCurve+1].NormalY;


		//cvZero(NormalImage);
		//for (int i=0;i<globs->NumOfCurvePointPerFrame;i++)
		//{
		//	printf(" globs->NormalX[i][f]=%f, globs->NormalY[i][f]=%f\n", globs->NormalX[i][f], globs->NormalY[i][f]);
		//	cvLine(NormalImage,cvPoint(globs->x[i][f],globs->y[i][f]),cvPoint(globs->x[i][f] + 10 * globs->NormalX[i][f],globs->y[i][f] + 10 * globs->NormalY[i][f]),cvScalar(255,0,255),1);
		//	cvCircle(NormalImage,cvPoint(globs->x[i][f],globs->y[i][f]),2,cvScalar(0,255,0),-1);			
		//}
		//cvShowImage("NormalImage",NormalImage);
		//cvWaitKey(0);
		//printf("\n\n\n\n");
	}
	delete [] CurvePoint;



	/********************************************************************************************************************************************************

	------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	frame 1   xy xy xy  |  frame2 xy xy xy  ........................................
	------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	********************************************************************************************************************************************************/




	/************************************************************************************************************************
												 初始化所有的全局变量
	*************************************************************************************************************************/
	register int xcounter = 0;
	
	if (globs->FirstTime)
	{
		memset(jac,0,sizeof(double)* globs->NumOfParam * globs->xLength);
					
		FirtstTimeDo(globs);

		globs->FirstTime = false;
	}



	int ElemPerFrame = 2*globs->NumOfCurvePointPerFrame;
	int ElemPerRow = ElemPerFrame * globs->NumOfFrame;

	
	/*
	________________________________________________________________________________________________________________________
	EL : (NumOfCurvePoint-1) * (NumOfFrame-1)  |  EC : 2 * (NumOfCurvePoint-2) * (NumOfFrame-1) | EV: 2 * NumOfCurvePoint * (NumOfFrame -1)| 
  	------------------------------------------------------------------------------------------------------------------------
	------------------------------------------------------------------------------------------------------------------------
	EI : NumOfCurvePoint * (NumOfFrame-1) * (2K+1) *3  |  EG: NumOfCurvePoint * NumOfFrame
	------------------------------------------------------------------------------------------------------------------------
	*/

		/******************************************************************************************************************************************************
		EV
		******************************************************************************************************************************************************/
		int CurrentBlockRowIndex = 0;
		int OffsetInBlock = 0;
		int NumOfParamPerFrame = 4 * globs->NumOfControlPoint;
		//关于x分量的导数
		double EVWeight = 1.0 / globs->StdEV;
		for (int f = 0; f<globs->NumOfFrame-1 ; f++)
		{
			if (f==0)
			{
				for (int cpid = 0,pid = 0; pid<globs->NumOfCurvePointPerFrame ; pid++)
				{
					//每完成了1+SAMPLESPERCURVE个点就相当于完成了一截的贝塞尔曲线，进入下一截，所以要清0
					//注意这里 pid!= && pid!=globglobs->NumOfCurvePointPerFrame - 1 
					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}


//					TempJac(xcounter, ElemPerFrame * (f+1) + 2*pid) = -EVWeight;

					//Xi,t+1
					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * (f+1);		//f=0的时候，第0帧是没有的，只有第1帧
					int RowIndex = ElemPerFrame * (f+1) + 2*pid;
					for (int k = 0;k<8;k++)
					{
						double Result =-EVWeight * dXcurvedXcontrol(RowIndex,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}


					xcounter++;
					OffsetInBlock++;											//修改这里

				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里

			}
			else if (f==globs->NumOfFrame-2)
			{
				for (int cpid = 0,pid = 0; pid<globs->NumOfCurvePointPerFrame ; pid++)
				{

					//每完成了1+SAMPLESPERCURVE个点就相当于完成了一截的贝塞尔曲线，进入下一截，所以要清0
					//注意这里 pid!= && pid!=globglobs->NumOfCurvePointPerFrame - 1 
					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					//Xi,t

	//				TempJac(xcounter,ElemPerFrame * f + 2*pid) = EVWeight;

					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;	// f = globs->NumOfFrame-2时，最后一帧是没有的，只有当前帧
					int RowIndex = ElemPerFrame * f + 2*pid;
					for (int k = 0;k<8;k++)
					{
						double Result= EVWeight * dXcurvedXcontrol(RowIndex,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}

					xcounter++;
					OffsetInBlock++;											//修改这里
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里

			}
			else{
				for (int cpid = 0,pid = 0; pid<globs->NumOfCurvePointPerFrame ; pid++)
				{
					//每完成了1+SAMPLESPERCURVE个点就相当于完成了一截的贝塞尔曲线，进入下一截，所以要清0
					//注意这里 pid!= && pid!=globglobs->NumOfCurvePointPerFrame - 1 
					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}


					//Xi,t

//					TempJac(xcounter,ElemPerFrame * f + 2*pid) = EVWeight;
					
					//Xi,t+1

//					TempJac(xcounter,ElemPerFrame * (f+1) + 2*pid) = -EVWeight;



					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;			//当前帧
					int RowIndex1 = ElemPerFrame * f + 2*pid;
					int RowIndex2 = ElemPerFrame * (f+1) + 2*pid;
					for (int k = 0;k<8;k++)
					{
						double Result = EVWeight * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}
					ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * (f+1);			//下一帧
					for (int k = 0;k<8;k++)
					{
						double Result = - EVWeight * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}


					xcounter++;
					OffsetInBlock++;											//修改这里
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里			
				
			}

		}

		//关于y分量的导数
		for (int f = 0; f<globs->NumOfFrame-1 ; f++)
		{
			if (f==0)
			{
				for (int cpid = 0,pid = 0; pid<globs->NumOfCurvePointPerFrame ; pid++)
				{

					//每完成了1+SAMPLESPERCURVE个点就相当于完成了一截的贝塞尔曲线，进入下一截，所以要清0
					//注意这里 pid!= && pid!=globglobs->NumOfCurvePointPerFrame - 1 
					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					//Yi,t+1		
			
//					TempJac(xcounter, ElemPerFrame * (f+1) + 2*pid+1) = -EVWeight;


					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * (f+1);		//f=0的时候，第0帧是没有的，只有第1帧
					int RowIndex = ElemPerFrame * (f+1) + 2*pid+1;
					for (int k = 0;k<8;k++)
					{
						double Result = -EVWeight * dXcurvedXcontrol(RowIndex,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}


					xcounter++;
					OffsetInBlock++;											//修改这里

				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里
			}
			else if (f==globs->NumOfFrame-2)
			{
				for (int cpid = 0,pid = 0; pid<globs->NumOfCurvePointPerFrame ; pid++)
				{
					//每完成了1+SAMPLESPERCURVE个点就相当于完成了一截的贝塞尔曲线，进入下一截，所以要清0
					//注意这里 pid!= && pid!=globglobs->NumOfCurvePointPerFrame - 1 
					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					//Yi,t

//					TempJac(xcounter,ElemPerFrame * f + 2*pid+1) = EVWeight;

					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;	// f = globs->NumOfFrame-2时，最后一帧是没有的，只有当前帧
					int RowIndex = ElemPerFrame * f + 2*pid+1;
					for (int k = 0;k<8;k++)
					{
						double Result = EVWeight * dXcurvedXcontrol(RowIndex,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}

					xcounter++;
					OffsetInBlock++;											//修改这里
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里
			}
			else{
				for (int cpid = 0,pid = 0; pid<globs->NumOfCurvePointPerFrame ; pid++)
				{
					//每完成了1+SAMPLESPERCURVE个点就相当于完成了一截的贝塞尔曲线，进入下一截，所以要清0
					//注意这里 pid!= && pid!=globglobs->NumOfCurvePointPerFrame - 1 
					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}


					//Yi,t

//					TempJac(xcounter,ElemPerFrame * f + 2*pid+1) = EVWeight;


					//Yi,t+1

//					TempJac(xcounter,ElemPerFrame * (f+1) + 2*pid+1) = -EVWeight;


					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;			//当前帧
					int RowIndex1 = ElemPerFrame * f + 2*pid+1;
					int RowIndex2 = ElemPerFrame * (f+1) + 2*pid+1;
					for (int k = 0;k<8;k++)
					{
						double Result = EVWeight * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}
					ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * (f+1);			//下一帧
					for (int k = 0;k<8;k++)
					{
						double Result= - EVWeight * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}


					xcounter++;
					OffsetInBlock++;											//修改这里
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里
			}


		}
		
		/************************************************************************************************************************
														The EG Term    
												  NumOfFrame * NumOfCurvePoint
	************************************************************************************************************************/
	
			

{
	
	double KMax = 1200;   //KMAX = sqrt(255.0*255.0*2.0)*3 = 1081
	double EGWeight = 1.0 / globs->StdEG;
	for (int f= 0;f<globs->NumOfFrame;f++)
	{
		if(f==0){
			xcounter += globs->NumOfCurvePointPerFrame;
			CurrentBlockRowIndex += globs->NumOfControlPoint - 1;			//修改了这里
		}
		else if(f==globs->NumOfFrame-1){
			xcounter += globs->NumOfCurvePointPerFrame;
			CurrentBlockRowIndex += globs->NumOfControlPoint - 1;			//修改了这里
		}
		else{
			int ElemPerFrame_f =ElemPerFrame * f;
			for (int cpid = 0,pid = 0;pid < globs->NumOfCurvePointPerFrame;pid++)
			{
				if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
				{
					CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
					OffsetInBlock = 0;
					cpid ++;
				}


				////Xi,t

				////TempJac(xcounter,ElemPerFrame * f + 2*pid) = -EGWeight * ConvolveX(globs->Gradient[f],int(globs->x[pid][f]),int(globs->y[pid][f]),int(globs->Width)) / globs->M[pid];
				//TempJac(xcounter,ElemPerFrame_f + 2*pid) = -EGWeight * ConvolveX(globs->Gradient[f],int(globs->x[pid][f]),int(globs->y[pid][f]),int(globs->Width)) / globs->M[pid];
				////Yi,t

				////TempJac(xcounter,ElemPerFrame * f + 2*pid + 1) = -EGWeight * ConvolveY(globs->Gradient[f],int(globs->x[pid][f]),int(globs->y[pid][f]),int(globs->Width)) / globs->M[pid];
				//TempJac(xcounter,ElemPerFrame_f + 2*pid + 1) = -EGWeight * ConvolveY(globs->Gradient[f],int(globs->x[pid][f]),int(globs->y[pid][f]),int(globs->Width)) / globs->M[pid];


				double val1 = -EGWeight * ConvolveX(globs->Gradient[f],int(globs->x[pid][f]),int(globs->y[pid][f]),int(globs->Width)) / globs->M[pid];	//修改了这里
				double val2 = -EGWeight * ConvolveY(globs->Gradient[f],int(globs->x[pid][f]),int(globs->y[pid][f]),int(globs->Width)) / globs->M[pid];	//修改了这里

				int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;			//当前帧
				int RowIndex1 = ElemPerFrame_f + 2*pid;
				int RowIndex2 = ElemPerFrame_f + 2*pid + 1;
				for (int k = 0;k<8;k++)
				{
					double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k);
					JacBlockMat.setValue2(
						Result,	//修改了这里
						CurrentBlockRowIndex,
						OffsetInBlock,
						ColIndexOffset + k - NumOfParamPerFrame); 
					jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
				}



				xcounter++;
				OffsetInBlock++;											//修改这里
			}
			CurrentBlockRowIndex++;											//修改这里
			OffsetInBlock = 0;												//修改这里
		}
	}
}

	
		/******************************************************************************************************************************************************
																		EL
		******************************************************************************************************************************************************/
		double ELWeight = 1.0 / globs->StdEL;					//			修改
		for (int f=0;f<globs->NumOfFrame-1;f++)
		{
			if (f==0)	// it's the first frame ,then all the derivative related to this frames are 0
			{
				int ElemPerFrame_f_1 = ElemPerFrame*(f+1);
				for (int cpid = 0,pid = 0;pid < globs->NumOfCurvePointPerFrame-1;pid++)
				{

					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}


					////Xi+1,t+1


					////TempJac(xcounter,ElemPerFrame*(f+1) + 2*(pid+1)) = -2 * (globs->x[pid+1][f+1] - globs->x[pid][f+1]) * ELWeight;
					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+1)) = -2 * (globs->x[pid+1][f+1] - globs->x[pid][f+1]) * ELWeight;

					////Xi,t+1


					////TempJac(xcounter,ElemPerFrame*(f+1) + 2*pid) = 2 * (globs->x[pid+1][f+1] - globs->x[pid][f+1]) * ELWeight;
					//TempJac(xcounter,ElemPerFrame_f_1 + 2*pid) = 2 * (globs->x[pid+1][f+1] - globs->x[pid][f+1]) * ELWeight;
	
					////Yi+1,t+1

					////TempJac(xcounter,ElemPerFrame*(f+1) + 2*(pid+1) + 1) = -2 * (globs->y[pid+1][f+1] - globs->y[pid][f+1]) * ELWeight;
					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+1) + 1) = -2 * (globs->y[pid+1][f+1] - globs->y[pid][f+1]) * ELWeight;
					//
					////Yi,t+1

					////TempJac(xcounter,ElemPerFrame*(f+1) + 2*pid + 1) = 2 * (globs->y[pid+1][f+1] - globs->y[pid][f+1]) * ELWeight;
					//TempJac(xcounter,ElemPerFrame_f_1 + 2*pid + 1) = 2 * (globs->y[pid+1][f+1] - globs->y[pid][f+1]) * ELWeight;


					double val1 = -2 * (globs->x[pid+1][f+1] - globs->x[pid][f+1]) * ELWeight;
					double val2 = 2 * (globs->x[pid+1][f+1] - globs->x[pid][f+1]) * ELWeight;
					double val3 = -2 * (globs->y[pid+1][f+1] - globs->y[pid][f+1]) * ELWeight;
					double val4 = 2 * (globs->y[pid+1][f+1] - globs->y[pid][f+1]) * ELWeight;

					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * (f+1);				//下一帧
					int RowIndex1 = ElemPerFrame_f_1 + 2*(pid+1);
					int RowIndex2 = ElemPerFrame_f_1 + 2*pid;
					int RowIndex3 = ElemPerFrame_f_1 + 2*(pid+1) + 1;
					int RowIndex4 = ElemPerFrame_f_1 + 2*pid + 1;

					for (int k = 0;k<8;k++)
					{
						double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k) +
							val4 * dXcurvedXcontrol(RowIndex4,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result ,	//修改了这里
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}



					xcounter ++ ;
					OffsetInBlock++;											//修改这里
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里
			}
			else if(f==globs->NumOfFrame-2){
				int ElemPerFrame_f = ElemPerFrame*f;
				for (int cpid = 0,pid = 0;pid < globs->NumOfCurvePointPerFrame-1;pid++)
				{


					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					////Xi+1,t


					//TempJac(xcounter,ElemPerFrame_f + 2*(pid+1)) = 2 * (globs->x[pid+1][f] - globs->x[pid][f]) * ELWeight;

					////Xi,t

					//TempJac(xcounter,ElemPerFrame_f + 2*pid) = -2 * (globs->x[pid+1][f] - globs->x[pid][f]) * ELWeight;				//修改


	
					////Yi+1,t

					//TempJac(xcounter,ElemPerFrame_f + 2*(pid+1) + 1) = 2 * (globs->y[pid+1][f] - globs->y[pid][f]) * ELWeight;				//修改
					//
					////Yi,t

					//TempJac(xcounter,ElemPerFrame_f + 2*pid + 1) = -2 * (globs->y[pid+1][f] - globs->y[pid][f]) * ELWeight;				//修改


					double val1 = 2 * (globs->x[pid+1][f] - globs->x[pid][f]) * ELWeight;
					double val2 = -2 * (globs->x[pid+1][f] - globs->x[pid][f]) * ELWeight;
					double val3 = 2 * (globs->y[pid+1][f] - globs->y[pid][f]) * ELWeight;
					double val4 = -2 * (globs->y[pid+1][f] - globs->y[pid][f]) * ELWeight;


					int RowIndex1 = ElemPerFrame_f + 2*(pid+1);
					int RowIndex2 = ElemPerFrame_f + 2*pid;
					int RowIndex3 = ElemPerFrame_f + 2*(pid+1) + 1;
					int RowIndex4 = ElemPerFrame_f + 2*pid + 1;

					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;	// f = globs->NumOfFrame-2时，最后一帧是没有的，只有当前帧

					for (int k = 0;k<8;k++)
					{
						double Result =val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k) +
							val4 * dXcurvedXcontrol(RowIndex4,ColIndexOffset + k) ;
						JacBlockMat.setValue2(
							Result,	//修改了这里
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}

					xcounter ++ ;
					OffsetInBlock++;											//修改这里
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里
			}
			else{
				int ElemPerFrame_f = ElemPerFrame*f;
				int ElemPerFrame_f_1 = ElemPerFrame*(f+1);
				for (int cpid = 0,pid = 0;pid < globs->NumOfCurvePointPerFrame-1;pid++)
				{


					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					////Xi+1,t

					//TempJac(xcounter,ElemPerFrame_f + 2*(pid+1)) = 2 * (globs->x[pid+1][f] - globs->x[pid][f]) * ELWeight;				//修改
					//
					////Xi,t

				 //	TempJac(xcounter,ElemPerFrame_f + 2*pid) = 	-2 * (globs->x[pid+1][f] - globs->x[pid][f]) * ELWeight;				//修改
	
					////Yi+1,t

					//TempJac(xcounter,ElemPerFrame_f + 2*(pid+1) + 1) = 2 * (globs->y[pid+1][f] - globs->y[pid][f]) * ELWeight;				//修改
					//
					//
					////Yi,t

					//TempJac(xcounter,ElemPerFrame_f + 2*pid + 1) = -2 * (globs->y[pid+1][f] - globs->y[pid][f]) * ELWeight;				//修改



					double val1 = 2 * (globs->x[pid+1][f] - globs->x[pid][f]) * ELWeight;
					double val2 = -2 * (globs->x[pid+1][f] - globs->x[pid][f]) * ELWeight;
					double val3 = 2 * (globs->y[pid+1][f] - globs->y[pid][f]) * ELWeight;
					double val4 = -2 * (globs->y[pid+1][f] - globs->y[pid][f]) * ELWeight;


					int RowIndex1 = ElemPerFrame_f + 2*(pid+1);
					int RowIndex2 = ElemPerFrame_f + 2*pid;
					int RowIndex3 = ElemPerFrame_f + 2*(pid+1) + 1;
					int RowIndex4 = ElemPerFrame_f + 2*pid + 1;

					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;	// 当前帧 

					for (int k = 0;k<8;k++)
					{
						double Result= val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k) +
							val4 * dXcurvedXcontrol(RowIndex4,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result ,	//修改了这里
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}



					////Xi+1,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+1)) = -2 * (globs->x[pid+1][f+1] - globs->x[pid][f+1]) * ELWeight;			//修改
					//
					//
					////Xi,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*pid) = 2 * (globs->x[pid+1][f+1] - globs->x[pid][f+1]) * ELWeight;			//修改


	
					////Yi+1,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+1) + 1) = -2 * (globs->y[pid+1][f+1] - globs->y[pid][f+1]) * ELWeight;			//修改
					//
					//
					////Yi,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*pid + 1) = 2 * (globs->y[pid+1][f+1] - globs->y[pid][f+1]) * ELWeight;			//修改



					val1 = -2 * (globs->x[pid+1][f+1] - globs->x[pid][f+1]) * ELWeight;
					val2 = 2 * (globs->x[pid+1][f+1] - globs->x[pid][f+1]) * ELWeight;
					val3 = -2 * (globs->y[pid+1][f+1] - globs->y[pid][f+1]) * ELWeight;
					val4 = 2 * (globs->y[pid+1][f+1] - globs->y[pid][f+1]) * ELWeight;

					ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * (f+1);				//下一帧
					RowIndex1 = ElemPerFrame_f_1 + 2*(pid+1);
					RowIndex2 = ElemPerFrame_f_1 + 2*pid;
					RowIndex3 = ElemPerFrame_f_1 + 2*(pid+1) + 1;
					RowIndex4 = ElemPerFrame_f_1 + 2*pid + 1;

					for (int k = 0;k<8;k++)
					{
						double Result=val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k) +
							val4 * dXcurvedXcontrol(RowIndex4,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result ,	//修改了这里
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}


					xcounter ++ ;
					OffsetInBlock++;											//修改这里
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里
			}
		}
	
		
	
		/******************************************************************************************************************************************************
																		EC
		******************************************************************************************************************************************************/	
	
		double ECWeight = 1.0 / globs->StdEC;
		//x
		for (int f=0;f<globs->NumOfFrame-1;f++)
		{
			if (f==0)
			{
				int ElemPerFrame_f_1 = ElemPerFrame*(f+1);
				
				for (int cpid = 0,pid=0;pid<globs->NumOfCurvePointPerFrame-2;pid++)
				{

					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-2 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					////Xi,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*pid) = -1.0 * ECWeight;															//修改
					//
					////Xi+1,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+1)) = 2.0 * ECWeight;															//修改
					//
					//
					////Xi+2,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+2)) = -1.0 * ECWeight;															//修改



					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * (f+1);		//f=0的时候，第0帧是没有的，只有第1帧
					int RowIndex1 = ElemPerFrame_f_1 + 2*pid;
					int RowIndex2 = ElemPerFrame_f_1 + 2*(pid+1);
					int RowIndex3 = ElemPerFrame_f_1 + 2*(pid+2);
					double val1=-1.0 * ECWeight;
					double val2=2.0 * ECWeight;
					double val3=-1.0 * ECWeight;

					for (int k = 0;k<8;k++)
					{
						double Result=val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) + 
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}
					if (pid == globs->NumOfCurvePointPerFrame-2 || (pid + 1)%(1+ SAMPLESPERCURVE)==0 )
					{
						for (int k=8;k<12;k++)
						{
							double Result=val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
								val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) + 
								val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
							JacBlockMat.setValue2(
								Result,
								CurrentBlockRowIndex,
								OffsetInBlock,
								ColIndexOffset + k - NumOfParamPerFrame); 
							jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
						}
					}

					xcounter++;	
					OffsetInBlock++;											//修改这里
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里
			}
			else if (f == globs->NumOfFrame-2)
			{
				int ElemPerFrame_f = ElemPerFrame*(f);
				for (int cpid = 0,pid=0;pid<globs->NumOfCurvePointPerFrame-2;pid++)
				{

					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-2 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					////Xi,t

					//TempJac(xcounter , ElemPerFrame_f + 2*pid) = 1.0 * ECWeight;															//修改
					//
					////Xi+1,t

					//TempJac(xcounter, ElemPerFrame_f + 2*(pid+1)) = -2.0 * ECWeight;															//修改
					//
					////Xi+2,t

					//TempJac(xcounter, ElemPerFrame_f + 2*(pid+2)) = 1.0 * ECWeight;															//修改



					double val1 = 1.0 * ECWeight;
					double val2 = -2.0 * ECWeight;
					double val3 = 1.0 * ECWeight;


					int RowIndex1 = ElemPerFrame_f + 2*pid;
					int RowIndex2 = ElemPerFrame_f + 2*(pid+1);
					int RowIndex3 = ElemPerFrame_f + 2*(pid+2);

					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;	// f = globs->NumOfFrame-2时，最后一帧是没有的，只有当前帧

					for (int k = 0;k<8;k++)
					{
						double Result =val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}				

					if (pid == globs->NumOfCurvePointPerFrame-2 || (pid + 1)%(1+ SAMPLESPERCURVE)==0 )
					{
						for (int k=8;k<12;k++)
						{
							double Result =val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
								val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
								val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
							JacBlockMat.setValue2(
								Result,
								CurrentBlockRowIndex,
								OffsetInBlock,
								ColIndexOffset + k - NumOfParamPerFrame); 
							jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
						}
					}
					xcounter++;
					OffsetInBlock++;											//修改这里
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里
			}
			else{
				int ElemPerFrame_f_1 = ElemPerFrame*(f+1);
				int ElemPerFrame_f = ElemPerFrame*(f);
				for (int cpid = 0,pid=0;pid<globs->NumOfCurvePointPerFrame-2;pid++)
				{

					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-2 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					////Xi,t

					//TempJac(xcounter,ElemPerFrame_f + 2*pid) = 1.0 * ECWeight;															//修改


					////Xi+1,t

					//TempJac(xcounter,ElemPerFrame_f + 2*(pid+1)) = -2.0 * ECWeight;															//修改
					////Xi+2,t

					//TempJac(xcounter,ElemPerFrame_f + 2*(pid+2)) = 1.0 * ECWeight;															//修改
					//


					double val1 = 1.0 * ECWeight;
					double val2 = -2.0 * ECWeight;
					double val3 = 1.0 * ECWeight;


					int RowIndex1 = ElemPerFrame_f + 2*pid;
					int RowIndex2 = ElemPerFrame_f + 2*(pid+1);
					int RowIndex3 = ElemPerFrame_f + 2*(pid+2);

					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;		// 当前帧

					for (int k = 0;k<8;k++)
					{
						double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}		

					if (pid == globs->NumOfCurvePointPerFrame-2 || (pid + 1)%(1+ SAMPLESPERCURVE)==0 )
					{
						for (int k=8;k<12;k++)
						{
							double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
								val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
								val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
							JacBlockMat.setValue2(
								Result,
								CurrentBlockRowIndex,
								OffsetInBlock,
								ColIndexOffset + k - NumOfParamPerFrame); 
							jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
						}
					}


					////Xi,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*pid) = -1.0 * ECWeight;															//修改
					////Xi+1,t+1
					//

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+1)) = 2.0 * ECWeight;															//修改
					//
					////Xi+2,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+2)) = -1.0 * ECWeight;															//修改
					//


					ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * (f+1);		//下一帧
					RowIndex1 = ElemPerFrame_f_1 + 2*pid;
					RowIndex2 = ElemPerFrame_f_1 + 2*(pid+1);
					RowIndex3 = ElemPerFrame_f_1 + 2*(pid+2);
					val1=-1.0 * ECWeight;
					val2=2.0 * ECWeight;
					val3=-1.0 * ECWeight;

					for (int k = 0;k<8;k++)
					{
						double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) + 
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}


					if (pid == globs->NumOfCurvePointPerFrame-2 || (pid + 1)%(1+ SAMPLESPERCURVE)==0 )
					{
						for (int k=8;k<12;k++)
						{
							double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
								val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) + 
								val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
							JacBlockMat.setValue2(
								Result,
								CurrentBlockRowIndex,
								OffsetInBlock,
								ColIndexOffset + k - NumOfParamPerFrame); 
							jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
						}
					}		


					xcounter++;
					OffsetInBlock++;											//修改这里

	
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里
			}
		}
		//y
		for (int f=0;f<globs->NumOfFrame-1;f++)
		{
			if (f==0)
			{
				int ElemPerFrame_f_1 = ElemPerFrame*(f+1);
				for (int cpid=0,pid=0;pid<globs->NumOfCurvePointPerFrame-2;pid++)
				{

					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-2 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					////Yi,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*pid + 1) = -1.0 * ECWeight;															//修改

					////Yi+1,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+1) + 1) = 2.0 * ECWeight;															//修改



					////Yi+2,t+1

					//TempJac(xcounter, ElemPerFrame_f_1 + 2*(pid+2) + 1) = -1.0 * ECWeight;															//修改



					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * (f+1);		//f=0的时候，第0帧是没有的，只有第1帧
					int RowIndex1 = ElemPerFrame_f_1 + 2*pid + 1;
					int RowIndex2 = ElemPerFrame_f_1 + 2*(pid+1) + 1;
					int RowIndex3 = ElemPerFrame_f_1 + 2*(pid+2) + 1;
					double val1=-1.0 * ECWeight;
					double val2=2.0 * ECWeight;
					double val3=-1.0 * ECWeight;

					for (int k = 0;k<8;k++)
					{
						double Result =val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) + 
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k); 
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}


					if (pid == globs->NumOfCurvePointPerFrame-2 || (pid + 1)%(1+ SAMPLESPERCURVE)==0 )
					{
						for (int k=8;k<12;k++)
						{
							double Result =val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
								val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) + 
								val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k); 
							JacBlockMat.setValue2(
								Result,
								CurrentBlockRowIndex,
								OffsetInBlock,
								ColIndexOffset + k - NumOfParamPerFrame); 
							jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
						}
					}	

					xcounter++;
					OffsetInBlock++;											//修改这里
				}
				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里

			}
			else if (f == globs->NumOfFrame-2)
			{
				int ElemPerFrame_f = ElemPerFrame*(f);
				for (int cpid = 0,pid=0;pid<globs->NumOfCurvePointPerFrame-2;pid++)
				{

					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-1 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					////Yi,t

					//TempJac(xcounter, ElemPerFrame_f + 2*pid + 1) = 1.0 * ECWeight;															//修改

					////Yi+1,t

					//TempJac(xcounter, ElemPerFrame_f + 2*(pid+1) + 1) = -2.0 * ECWeight;															//修改

					////Yi+2,t

					//TempJac(xcounter, ElemPerFrame_f + 2*(pid+2) + 1) = 1.0 * ECWeight;															//修改


					double val1 = 1.0 * ECWeight;
					double val2 = -2.0 * ECWeight;
					double val3 = 1.0 * ECWeight;


					int RowIndex1 = ElemPerFrame_f + 2*pid + 1;
					int RowIndex2 = ElemPerFrame_f + 2*(pid+1) + 1;
					int RowIndex3 = ElemPerFrame_f + 2*(pid+2) + 1;

					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;	// f = globs->NumOfFrame-2时，最后一帧是没有的，只有当前帧

					for (int k = 0;k<8;k++)
					{
						double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
						JacBlockMat.setValue2(Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}				

					if (pid == globs->NumOfCurvePointPerFrame-2 || (pid + 1)%(1+ SAMPLESPERCURVE)==0 )
					{
						for (int k=8;k<12;k++)
						{
							double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
								val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
								val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
							JacBlockMat.setValue2(Result,
								CurrentBlockRowIndex,
								OffsetInBlock,
								ColIndexOffset + k - NumOfParamPerFrame); 
							jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
						}
					}	


					xcounter++;
					OffsetInBlock++;											//修改这里
				}

				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里
			}
			else{
				int ElemPerFrame_f_1 = ElemPerFrame*(f+1);
				int ElemPerFrame_f = ElemPerFrame*(f);
				for (int cpid = 0,pid=0;pid<globs->NumOfCurvePointPerFrame-2;pid++)
				{
				
					if ( pid!=0 && pid!= globs->NumOfCurvePointPerFrame-2 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里
					{
						CurrentBlockRowIndex++;		//这里的CurrentBlockRowIndex就相当于ControlPointIndex
						OffsetInBlock = 0;
						cpid ++;
					}

					////Yi,t


					//TempJac(xcounter,ElemPerFrame_f + 2*pid + 1) = 1.0 * ECWeight;															//修改

					////Yi+1,t


					//TempJac(xcounter,ElemPerFrame_f + 2*(pid+1) + 1) = -2.0 * ECWeight;															//修改
					////Yi+2,t

					//TempJac(xcounter,ElemPerFrame_f + 2*(pid+2) + 1) = 1.0 * ECWeight;															//修改


					double val1 = 1.0 * ECWeight;
					double val2 = -2.0 * ECWeight;
					double val3 = 1.0 * ECWeight;


					int RowIndex1 = ElemPerFrame_f + 2*pid + 1;
					int RowIndex2 = ElemPerFrame_f + 2*(pid+1) + 1;
					int RowIndex3 = ElemPerFrame_f + 2*(pid+2) + 1;

					int ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * f;		// 当前帧

					for (int k = 0;k<8;k++)
					{
						double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
						val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
						val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
						JacBlockMat.setValue2(Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}		

					if (pid == globs->NumOfCurvePointPerFrame-2 || (pid + 1)%(1+ SAMPLESPERCURVE)==0 )
					{
						for (int k=8;k<12;k++)
						{
							double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
								val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) +
								val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
							JacBlockMat.setValue2(Result,
								CurrentBlockRowIndex,
								OffsetInBlock,
								ColIndexOffset + k - NumOfParamPerFrame); 
							jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
						}
					}	
					////Yi,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*pid + 1) = -1.0 * ECWeight;															//修改


					////Yi+1,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+1) + 1) = 2.0 * ECWeight;															//修改

					////Yi+2,t+1

					//TempJac(xcounter,ElemPerFrame_f_1 + 2*(pid+2) + 1) = -1.0 * ECWeight;															//修改


					ColIndexOffset = 4 * cpid + 4*globs->NumOfControlPoint * (f+1);		//下一帧
					RowIndex1 = ElemPerFrame_f_1 + 2*pid + 1;
					RowIndex2 = ElemPerFrame_f_1 + 2*(pid+1) + 1;
					RowIndex3 = ElemPerFrame_f_1 + 2*(pid+2) + 1;
					val1=-1.0 * ECWeight;
					val2=2.0 * ECWeight;
					val3=-1.0 * ECWeight;

					for (int k = 0;k<8;k++)
					{
						double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
							val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) + 
							val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
						JacBlockMat.setValue2(
							Result,
							CurrentBlockRowIndex,
							OffsetInBlock,
							ColIndexOffset + k - NumOfParamPerFrame); 
						jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
					}

					if (pid == globs->NumOfCurvePointPerFrame-2 || (pid + 1)%(1+ SAMPLESPERCURVE)==0 )
					{
						for (int k=8;k<12;k++)
						{
							double Result = val1 * dXcurvedXcontrol(RowIndex1,ColIndexOffset + k) + 
								val2 * dXcurvedXcontrol(RowIndex2,ColIndexOffset + k) + 
								val3 * dXcurvedXcontrol(RowIndex3,ColIndexOffset + k);
							JacBlockMat.setValue2(
								Result,
								CurrentBlockRowIndex,
								OffsetInBlock,
								ColIndexOffset + k - NumOfParamPerFrame); 
							jac[xcounter * globs->NumOfParam + ColIndexOffset + k - NumOfParamPerFrame] = Result;
						}
					}	
					xcounter++;
					OffsetInBlock++;											//修改这里
				}

				CurrentBlockRowIndex++;											//修改这里
				OffsetInBlock = 0;												//修改这里

			}
		}

	
		/************************************************************************************************************************
												这里算出真正的jac并且将其方到jac矩阵立面
	************************************************************************************************************************/


	//修改
	
	
	
	////MatMulCRS(TempJac,dXcurvedXcontrol,globs->TempJacdXcurveXcontrol);
	//MatMulCRSNonZeroToBlock(
	//	TempJac,
	//	dXcurvedXcontrol,
	//	JacNonZeroRowIndexOfEachColumn,
	//	4*globs->NumOfControlPoint,
	//	globs->NumOfParam,
	//	jac,
	//	JacBlockMat);



/*
	double * R = globs->TempJacdXcurveXcontrol;
	int Offset = 4*globs->NumOfControlPoint;        //4*globs->NumOfControlPoint是第一帧里面的参数值总量  乘以4是没有错的


	for (int r=0;r<xcounter;r++)
	{
		for (int c=0;c<globs->NumOfParam;c++)		//用真实的param值
		{
			double Value = R[r*FakeNumOfParam + Offset + c];	
			jac[r*globs->NumOfParam + c] = Value;  	
			
			if (!IsZero(Value))			//这句是筛选处数值比较稳定的项
			{
				JacColForm[c][r] = Value;
			}
			else
			{
				JacColForm[c][r] = 0.0;
			}
				
			
		}
	}
*/


	/************************************************************************************************************************
														The EI Term    
											NumOfCurvePoint * (NumOfFrame-1) * (2K+1) *3
											
					
						XHat = X + k * nx 
						YHat = Y + k * ny 
						X = (1-t)^3*x0 + 3*(1-t)^2*t*x1 -3*(1-t)*t^2*x2 + (6*(1-t)*t^2 + t^3)*x3
						Y = (1-t)^3*y0 + 3*(1-t)^2*t*y1 -3*(1-t)*t^2*y2 + (6*(1-t)*t^2 + t^3)*y3

						dXdx0 = dYdy0 =(1-t)*(1-t)*(1-t) ;
						dXdx1 = dYdy1 = 3*(1-t)*(1-t)*t  ;
						dXdx2 = dYdy2 = -3*(1-t)*t*t;
						dXdx3 = dYdy3 = (6*(1-t)*t^2 + t^3);
										 
						dx = diff(A,t) = 3*x1*(t - 1)^2 - 3*x0*(t - 1)^2 - 6*t*x2 + 12*t*x3 + 9*t^2*x2 - 15*t^2*x3 + 3*t*x1*(2*t - 2)
						dy = diff(A,t) = 3*y1*(t - 1)^2 - 3*y0*(t - 1)^2 - 6*t*y2 + 12*t*y3 + 9*t^2*y2 - 15*t^2*y3 + 3*t*y1*(2*t - 2)
						nx = -dy/norm;
						ny = dx/norm;		//we think that the norm is constant and ignore it when calculating derivatives



						因为dHatdx0 仅仅与t有关，而与帧数f以及其他的东西都无关，所以可以这样写:
						dEIdx0it = dEIdXitHat * dXitHatdx0it + dEIdYitHat * dYitHatdx0it; 
								 = dEIdXitHat * dXHatdx0 + dEIdYitHat * dYHatdx0;
						同样可得:
						dEIdx1it = dEIdXitHat * dXHatdx1 + dEIdYitHat * dYHatdx1;
						dEIdx2it = dEIdXitHat * dXHatdx2 + dEIdYitHat * dYHatdx2;
						dEIdx3it = dEIdXitHat * dXHatdx3 + dEIdYitHat * dYHatdx3;
						
						dEIdy0it = dEIdXitHat * dXHatdy0 + dEIdYitHat * dYHatdy0;
						dEIdy1it = dEIdXitHat * dXHatdy1 + dEIdYitHat * dYHatdy1;
						dEIdy2it = dEIdXitHat * dXHatdy2 + dEIdYitHat * dYHatdy2;
						dEIdy3it = dEIdXitHat * dXHatdy3 + dEIdYitHat * dYHatdy3;

						dEIdx0it_1 = dEIdXit_1Hat * dXHatdx0 + dEIdYit_1Hat * dYHatdx0;
						dEIdx1it_1 = dEIdXit_1Hat * dXHatdx1 + dEIdYit_1Hat * dYHatdx1;
						dEIdx2it_1 = dEIdXit_1Hat * dXHatdx2 + dEIdYit_1Hat * dYHatdx2;
						dEIdx3it_1 = dEIdXit_1Hat * dXHatdx3 + dEIdYit_1Hat * dYHatdx3;

						dEIdy0it_1 = dEIdXit_1Hat * dXHatdy0 + dEIdYit_1Hat * dYHatdy0;
						dEIdy1it_1 = dEIdXit_1Hat * dXHatdy1 + dEIdYit_1Hat * dYHatdy1;
						dEIdy2it_1 = dEIdXit_1Hat * dXHatdy2 + dEIdYit_1Hat * dYHatdy2;
						dEIdy3it_1 = dEIdXit_1Hat * dXHatdy3 + dEIdYit_1Hat * dYHatdy3;
					

						
						直接在最后的Jacobian里面插入EI方面的导数

	************************************************************************************************************************/
	{
		//用来记录当前的block的行号的
		int nEVblock = 2 * (globs->NumOfControlPoint - 1)*(globs->NumOfFrame - 1);
		int nEGblock = (globs->NumOfControlPoint - 1)*(globs->NumOfFrame) ;			//比较特殊，第一帧和最后一帧是剪裁了的
		int nELblock = (globs->NumOfControlPoint - 1)*(globs->NumOfFrame - 1);
		int nECblock = 2 * (globs->NumOfControlPoint - 1)*(globs->NumOfFrame - 1);
		int CurrentBlockRowIndex = nEVblock + nEGblock + nELblock + nECblock;
		int OffsetInBlock = 0;

		int EIstartOffset = xcounter;

		double dx,dy,norm,NormalX1,NormalY1,NormalX2,NormalY2,norm1,norm2;
		double x0,y0,x1,y1,x2,y2,x3,y3;
		double XHatit,XHatit_1,YHatit,YHatit_1;
		double EIWeight = 1.0 / globs->StdEI;    // 修改
		//printf("1.0 / StdVar = %f\n", EIWeight);
		//printf("StdEI=%f\n",StdEI);
		//system("pause");

		int ColIndex ;  //深度优化

		double t=0.0,step = 1.0/(globs->IntermediateSamplePerCurve + 1.0);
		int CurrentTIndex;
		int CurrentControlPointIndex;				//当前的curve由CurrentControlPointIndex 以及CurrentControlPointIndex+1的control point和adjust point 对来决定
		int NumOfParamPerFrame = globs->NumOfControlPoint*4;  // adjust point的数量和control point的数量是相等的，一个点由x y表示，所以乘以4
		int ActualNumOfParam = NumOfParamPerFrame * (globs->NumOfFrame-2);


		double dXHatdx0,dXHatdx1,dXHatdx2,dXHatdx3,dXHatdy0,dXHatdy1,dXHatdy2,dXHatdy3;
		double dYHatdx0,dYHatdx1,dYHatdx2,dYHatdx3,dYHatdy0,dYHatdy1,dYHatdy2,dYHatdy3;
		double dEIdx0it,dEIdx1it,dEIdx2it,dEIdx3it,dEIdy0it,dEIdy1it,dEIdy2it,dEIdy3it;
		double dEIdx0it_1,dEIdx1it_1,dEIdx2it_1,dEIdx3it_1,dEIdy0it_1,dEIdy1it_1,dEIdy2it_1,dEIdy3it_1;
		double dEIdXitHat,dEIdYitHat,dEIdXit_1Hat,dEIdYit_1Hat;

		int JacIndex;
		int CurrentRowIndex = EIstartOffset;	
		for (int f = 0;f<globs->NumOfFrame-1;f++)		
		{
			
			for (int pid = 0;pid<globs->NumOfCurvePointPerFrame-1;pid++)			
			{

				//每完成了1+SAMPLESPERCURVE个点就相当于完成了一截的贝塞尔曲线，进入下一截，所以要清0
				if ( pid!=0 && pid % (1 + SAMPLESPERCURVE)==0)					//修改了这里,注意这里是pid而不是pid+1 !!
				{
					CurrentBlockRowIndex++;
					OffsetInBlock = 0;
				}

				/*
				A = (1-t)^3*x0 + 3*(1-t)^2*t*x1 -3*(1-t)*t^2*x2 + (6*(1-t)*t^2 + t^3)*x3
				diff(A,t) = 3*x1*(t - 1)^2 - 3*x0*(t - 1)^2 - 6*t*x2 + 12*t*x3 + 9*t^2*x2 - 15*t^2*x3 + 3*t*x1*(2*t - 2)

				*/
				CurrentTIndex = pid%(globs->IntermediateSamplePerCurve+1);
				CurrentControlPointIndex = pid/(globs->IntermediateSamplePerCurve+1);
				t = CurrentTIndex * step;

				x0 = globs->xCp[CurrentControlPointIndex][f];
				y0 = globs->yCp[CurrentControlPointIndex][f];
				x1 = globs->xAp[CurrentControlPointIndex][f];
				y1 = globs->yAp[CurrentControlPointIndex][f];

				x2 = globs->xAp[CurrentControlPointIndex+1][f];
				y2 = globs->yAp[CurrentControlPointIndex+1][f];
				x3 = globs->xCp[CurrentControlPointIndex+1][f];
				y3 = globs->yCp[CurrentControlPointIndex+1][f];
				dx = 3 * x1 * (t - 1)*(t - 1) - 3*x0*(t - 1)*(t - 1) - 6*t*x2 + 12*t*x3 + 9*t*t*x2 - 15*t*t*x3 + 3*t*x1*(2*t - 2);
				dy = 3 * y1 * (t - 1)*(t - 1) - 3*y0*(t - 1)*(t - 1) - 6*t*y2 + 12*t*y3 + 9*t*t*y2 - 15*t*t*y3 + 3*t*y1*(2*t - 2);

				norm1 = norm = sqrt(dx*dx + dy * dy);
				NormalX1 = -dy/norm;
				NormalY1 = dx/norm;

				x0 = globs->xCp[CurrentControlPointIndex][f+1];
				y0 = globs->yCp[CurrentControlPointIndex][f+1];
				x1 = globs->xAp[CurrentControlPointIndex][f+1];
				y1 = globs->yAp[CurrentControlPointIndex][f+1];

				x2 = globs->xAp[CurrentControlPointIndex+1][f+1];
				y2 = globs->yAp[CurrentControlPointIndex+1][f+1];
				x3 = globs->xCp[CurrentControlPointIndex+1][f+1];
				y3 = globs->yCp[CurrentControlPointIndex+1][f+1];
				dx = 3 * x1 * (t - 1)*(t - 1) - 3*x0*(t - 1)*(t - 1) - 6*t*x2 + 12*t*x3 + 9*t*t*x2 - 15*t*t*x3 + 3*t*x1*(2*t - 2);
				dy = 3 * y1 * (t - 1)*(t - 1) - 3*y0*(t - 1)*(t - 1) - 6*t*y2 + 12*t*y3 + 9*t*t*y2 - 15*t*t*y3 + 3*t*y1*(2*t - 2);



				norm2 = norm = sqrt(dx*dx + dy * dy);
				NormalX2 = -dy/norm;
				NormalY2 = dx/norm;

				AlighToSameSideOfEdge(NormalX1,NormalY1,NormalX2,NormalY2);    //修改
/*
				if (NormalX1*NormalX2*NormalY1*NormalY2<0)			// 让他们方向一致????????????????????????????????????修改
				{
					NormalX1 = -NormalX1;
					NormalY1 = -NormalY1;
				}*/


				/**********************************************
					         预先计算好各种导数
				**********************************************/

				double t2 = t*t;
				double t3 = t*t*t;
				double t_13 = (t - 1)*(t - 1)*(t - 1);
				double t_12 = (t - 1)*(t - 1);
				/*
				dXHatdx0 = -(t - 1)^3;
				dXHatdx1 = 3*t*(t - 1)^2;
				dXHatdx2 = t^2*(3*t - 3);
				dXHatdx3 = t^3 - t^2*(6*t - 6);	

				dYHatdy0 = -(t - 1)^3;
				dYHatdy1 = 3*t*(t - 1)^2;
				dYHatdy2 = t^2*(3*t - 3);
				dYHatdy3 = t^3 - t^2*(6*t - 6);

				dXHatdy0 = (3*k*(t - 1)^2)/norm;
				dXHatdy1 =-(k*(3*t*(2*t - 2) + 3*(t - 1)^2))/norm;
				dXHatdy2 =-(k*(2*t*(3*t - 3) + 3*t^2))/norm;
				dXHatdy3 =(k*(2*t*(6*t - 6) + 3*t^2))/norm;

				dYHatdx0 =-(3*k*(t - 1)^2)/norm;
				dYHatdx1 = (k*(3*t*(2*t - 2) + 3*(t - 1)^2))/norm;
				dYHatdx2 = (k*(2*t*(3*t - 3) + 3*t^2))/norm;
				dYHatdx3 = -(k*(2*t*(6*t - 6) + 3*t^2))/norm;

				为了避免norm的影响，将norm单独提出来

				dXHatdy0 = (3*k*(t - 1)^2);
				dXHatdy1 =-(k*(3*t*(2*t - 2) + 3*(t - 1)^2));
				dXHatdy2 =-(k*(2*t*(3*t - 3) + 3*t^2));
				dXHatdy3 =(k*(2*t*(6*t - 6) + 3*t^2));

				dYHatdx0 =-(3*k*(t - 1)^2);
				dYHatdx1 = (k*(3*t*(2*t - 2) + 3*(t - 1)^2));
				dYHatdx2 = (k*(2*t*(3*t - 3) + 3*t^2));
				dYHatdx3 = -(k*(2*t*(6*t - 6) + 3*t^2));
				*/

				dYHatdy0=dXHatdx0  = -t_13;
				dYHatdy1=dXHatdx1  = 3*t*t_12;
				dYHatdy2=dXHatdx2  = t2*(3*t - 3);
				dYHatdy3=dXHatdx3  = t3 - t2*(6*t - 6);


				for (int k=-K;k<=K;k++)															//修改
				{
					XHatit = globs->x[pid][f] + k*NormalX1;
					YHatit = globs->y[pid][f] + k*NormalY1;

					XHatit_1 = globs->x[pid][f+1] + k*NormalX2;
					YHatit_1 = globs->y[pid][f+1] + k*NormalY2;


					// It - It+1
					 dEIdXitHat = ConvolveX(globs->R[f],int(XHatit),int(YHatit),globs->Width, globs->Height) *EIWeight;
					 dEIdYitHat = ConvolveY(globs->R[f],int(XHatit),int(YHatit),globs->Width, globs->Height) *EIWeight;
					 dEIdXit_1Hat = -ConvolveX(globs->R[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height) *EIWeight;
					 dEIdYit_1Hat = -ConvolveY(globs->R[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height) *EIWeight;



					 dXHatdy0 = (3*k*t_12);
					 dXHatdy1 =-(k*(3*t*(2*t - 2) + 3*t_12));
					 dXHatdy2 =-(k*(2*t*(3*t - 3) + 3*t2));
					 dXHatdy3 =(k*(2*t*(6*t - 6) + 3*t2));

					 dYHatdx0 =-dXHatdy0;
					 dYHatdx1 =-dXHatdy1;
					 dYHatdx2 =-dXHatdy2;
					 dYHatdx3 =-dXHatdy3;



					dEIdx0it = dEIdXitHat * dXHatdx0 + dEIdYitHat * dYHatdx0/norm1;
					dEIdx1it = dEIdXitHat * dXHatdx1 + dEIdYitHat * dYHatdx1/norm1;
					dEIdx2it = dEIdXitHat * dXHatdx2 + dEIdYitHat * dYHatdx2/norm1;
					dEIdx3it = dEIdXitHat * dXHatdx3 + dEIdYitHat * dYHatdx3/norm1;

					dEIdy0it = dEIdXitHat * dXHatdy0/norm1 + dEIdYitHat * dYHatdy0;
					dEIdy1it = dEIdXitHat * dXHatdy1/norm1 + dEIdYitHat * dYHatdy1;
					dEIdy2it = dEIdXitHat * dXHatdy2/norm1 + dEIdYitHat * dYHatdy2;
					dEIdy3it = dEIdXitHat * dXHatdy3/norm1 + dEIdYitHat * dYHatdy3;

					dEIdx0it_1 = dEIdXit_1Hat * dXHatdx0 + dEIdYit_1Hat * dYHatdx0/norm2;
					dEIdx1it_1 = dEIdXit_1Hat * dXHatdx1 + dEIdYit_1Hat * dYHatdx1/norm2;
					dEIdx2it_1 = dEIdXit_1Hat * dXHatdx2 + dEIdYit_1Hat * dYHatdx2/norm2;
					dEIdx3it_1 = dEIdXit_1Hat * dXHatdx3 + dEIdYit_1Hat * dYHatdx3/norm2;

					dEIdy0it_1 = dEIdXit_1Hat * dXHatdy0/norm2 + dEIdYit_1Hat * dYHatdy0;
					dEIdy1it_1 = dEIdXit_1Hat * dXHatdy1/norm2 + dEIdYit_1Hat * dYHatdy1;
					dEIdy2it_1 = dEIdXit_1Hat * dXHatdy2/norm2 + dEIdYit_1Hat * dYHatdy2;
					dEIdy3it_1 = dEIdXit_1Hat * dXHatdy3/norm2 + dEIdYit_1Hat * dYHatdy3;

					if(f==0){//如果是第一帧，那么我们就要跳过f-1的那个部分

						//第t+1帧 
						ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;   //深度优化
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						
						jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
						jac[JacIndex + 1] = dEIdy0it_1;
												
						jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it_1;

						jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it_1;

						jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
						jac[JacIndex + 5] = dEIdy3it_1;


/*
	
						
						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
						
						*/


						JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
						CurrentRowIndex++;
						OffsetInBlock++;													//修改这里

					}
					else if (f == globs->NumOfFrame-2)  //如果是最后一帧，就要跳过f那部分
					{
						//第t帧
						ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;

						jac[JacIndex + 0] = dEIdx0it;  // control point 1
						jac[JacIndex + 1] = dEIdy0it;

						jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it;

						jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it;

						jac[JacIndex + 4] = dEIdx3it;  // control point 2
						jac[JacIndex + 5] = dEIdy3it;


/*
						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;
*/


						JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

						CurrentRowIndex++;
						OffsetInBlock++;													//修改这里
					}
					else{
						//第t帧
						ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						jac[JacIndex + 0] = dEIdx0it;  // control point 1
						jac[JacIndex + 1] = dEIdy0it;

						jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it;

						jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it;

						jac[JacIndex + 4] = dEIdx3it;  // control point 2
						jac[JacIndex + 5] = dEIdy3it;



/*

						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;

*/



						JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

						//第t+1帧 
						ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
						jac[JacIndex + 1] = dEIdy0it_1;

						jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it_1;

						jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it_1;

						jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
						jac[JacIndex + 5] = dEIdy3it_1;




/*

						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/




						JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
						CurrentRowIndex++;
						OffsetInBlock++;													//修改这里
					}
					













					// It - It+1
					dEIdXitHat = ConvolveX(globs->G[f],int(XHatit),int(YHatit),globs->Width, globs->Height) *EIWeight;
					dEIdYitHat = ConvolveY(globs->G[f],int(XHatit),int(YHatit),globs->Width, globs->Height) *EIWeight;
					dEIdXit_1Hat = -ConvolveX(globs->G[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height) *EIWeight;
					dEIdYit_1Hat = -ConvolveY(globs->G[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height) *EIWeight;


					dXHatdy0 = (3*k*t_12);
					dXHatdy1 =-(k*(3*t*(2*t - 2) + 3*t_12));
					dXHatdy2 =-(k*(2*t*(3*t - 3) + 3*t2));
					dXHatdy3 =(k*(2*t*(6*t - 6) + 3*t2));

					dYHatdx0 =-dXHatdy0;
					dYHatdx1 =-dXHatdy1;
					dYHatdx2 =-dXHatdy2;
					dYHatdx3 =-dXHatdy3;



					dEIdx0it = dEIdXitHat * dXHatdx0 + dEIdYitHat * dYHatdx0/norm1;
					dEIdx1it = dEIdXitHat * dXHatdx1 + dEIdYitHat * dYHatdx1/norm1;
					dEIdx2it = dEIdXitHat * dXHatdx2 + dEIdYitHat * dYHatdx2/norm1;
					dEIdx3it = dEIdXitHat * dXHatdx3 + dEIdYitHat * dYHatdx3/norm1;

					dEIdy0it = dEIdXitHat * dXHatdy0/norm1 + dEIdYitHat * dYHatdy0;
					dEIdy1it = dEIdXitHat * dXHatdy1/norm1 + dEIdYitHat * dYHatdy1;
					dEIdy2it = dEIdXitHat * dXHatdy2/norm1 + dEIdYitHat * dYHatdy2;
					dEIdy3it = dEIdXitHat * dXHatdy3/norm1 + dEIdYitHat * dYHatdy3;

					dEIdx0it_1 = dEIdXit_1Hat * dXHatdx0 + dEIdYit_1Hat * dYHatdx0/norm2;
					dEIdx1it_1 = dEIdXit_1Hat * dXHatdx1 + dEIdYit_1Hat * dYHatdx1/norm2;
					dEIdx2it_1 = dEIdXit_1Hat * dXHatdx2 + dEIdYit_1Hat * dYHatdx2/norm2;
					dEIdx3it_1 = dEIdXit_1Hat * dXHatdx3 + dEIdYit_1Hat * dYHatdx3/norm2;

					dEIdy0it_1 = dEIdXit_1Hat * dXHatdy0/norm2 + dEIdYit_1Hat * dYHatdy0;
					dEIdy1it_1 = dEIdXit_1Hat * dXHatdy1/norm2 + dEIdYit_1Hat * dYHatdy1;
					dEIdy2it_1 = dEIdXit_1Hat * dXHatdy2/norm2 + dEIdYit_1Hat * dYHatdy2;
					dEIdy3it_1 = dEIdXit_1Hat * dXHatdy3/norm2 + dEIdYit_1Hat * dYHatdy3;

					if(f==0){//如果是第一帧，那么我们就要跳过f-1的那个部分

						//第t+1帧 
						ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
						jac[JacIndex + 1] = dEIdy0it_1;

						jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it_1;

						jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it_1;

						jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
						jac[JacIndex + 5] = dEIdy3it_1;





/*

						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/


						JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

						CurrentRowIndex++;
						OffsetInBlock++;													//修改这里
					}
					else if (f == globs->NumOfFrame-2)  //如果是最后一帧，就要跳过f那部分
					{
						//第t帧
						ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						jac[JacIndex + 0] = dEIdx0it;  // control point 1
						jac[JacIndex + 1] = dEIdy0it;

						jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it;

						jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it;

						jac[JacIndex + 4] = dEIdx3it;  // control point 2
						jac[JacIndex + 5] = dEIdy3it;




/*

						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;
*/


						JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

						CurrentRowIndex++;
						OffsetInBlock++;													//修改这里
					}
					else{
						//第t帧
						ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						jac[JacIndex + 0] = dEIdx0it;  // control point 1
						jac[JacIndex + 1] = dEIdy0it;

						jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it;

						jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it;

						jac[JacIndex + 4] = dEIdx3it;  // control point 2
						jac[JacIndex + 5] = dEIdy3it;



/*
						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;
*/


						JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);


						//第t+1帧 
						ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
						jac[JacIndex + 1] = dEIdy0it_1;

						jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it_1;

						jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it_1;

						jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
						jac[JacIndex + 5] = dEIdy3it_1;

/*


						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/


						JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
						CurrentRowIndex++;
						OffsetInBlock++;													//修改这里
					}













					// It - It+1
					 dEIdXitHat = ConvolveX(globs->B[f],int(XHatit),int(YHatit),globs->Width, globs->Height) *EIWeight;
					 dEIdYitHat = ConvolveY(globs->B[f],int(XHatit),int(YHatit),globs->Width, globs->Height) *EIWeight;
					 dEIdXit_1Hat = -ConvolveX(globs->B[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height) *EIWeight;
					 dEIdYit_1Hat = -ConvolveY(globs->B[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height) *EIWeight;

					 dXHatdy0 = (3*k*t_12);
					 dXHatdy1 =-(k*(3*t*(2*t - 2) + 3*t_12));
					 dXHatdy2 =-(k*(2*t*(3*t - 3) + 3*t2));
					 dXHatdy3 =(k*(2*t*(6*t - 6) + 3*t2));

					 dYHatdx0 =-dXHatdy0;
					 dYHatdx1 =-dXHatdy1;
					 dYHatdx2 =-dXHatdy2;
					 dYHatdx3 =-dXHatdy3;



					dEIdx0it = dEIdXitHat * dXHatdx0 + dEIdYitHat * dYHatdx0/norm1;
					dEIdx1it = dEIdXitHat * dXHatdx1 + dEIdYitHat * dYHatdx1/norm1;
					dEIdx2it = dEIdXitHat * dXHatdx2 + dEIdYitHat * dYHatdx2/norm1;
					dEIdx3it = dEIdXitHat * dXHatdx3 + dEIdYitHat * dYHatdx3/norm1;

					dEIdy0it = dEIdXitHat * dXHatdy0/norm1 + dEIdYitHat * dYHatdy0;
					dEIdy1it = dEIdXitHat * dXHatdy1/norm1 + dEIdYitHat * dYHatdy1;
					dEIdy2it = dEIdXitHat * dXHatdy2/norm1 + dEIdYitHat * dYHatdy2;
					dEIdy3it = dEIdXitHat * dXHatdy3/norm1 + dEIdYitHat * dYHatdy3;

					dEIdx0it_1 = dEIdXit_1Hat * dXHatdx0 + dEIdYit_1Hat * dYHatdx0/norm2;
					dEIdx1it_1 = dEIdXit_1Hat * dXHatdx1 + dEIdYit_1Hat * dYHatdx1/norm2;
					dEIdx2it_1 = dEIdXit_1Hat * dXHatdx2 + dEIdYit_1Hat * dYHatdx2/norm2;
					dEIdx3it_1 = dEIdXit_1Hat * dXHatdx3 + dEIdYit_1Hat * dYHatdx3/norm2;

					dEIdy0it_1 = dEIdXit_1Hat * dXHatdy0/norm2 + dEIdYit_1Hat * dYHatdy0;
					dEIdy1it_1 = dEIdXit_1Hat * dXHatdy1/norm2 + dEIdYit_1Hat * dYHatdy1;
					dEIdy2it_1 = dEIdXit_1Hat * dXHatdy2/norm2 + dEIdYit_1Hat * dYHatdy2;
					dEIdy3it_1 = dEIdXit_1Hat * dXHatdy3/norm2 + dEIdYit_1Hat * dYHatdy3;

					if(f==0){//如果是第一帧，那么我们就要跳过f-1的那个部分

						//第t+1帧 
						ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
						jac[JacIndex + 1] = dEIdy0it_1;

						jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it_1;

						jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it_1;

						jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
						jac[JacIndex + 5] = dEIdy3it_1;




/*

						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/


						JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
						CurrentRowIndex++;
						OffsetInBlock++;													//修改这里
					}
					else if (f == globs->NumOfFrame-2)  //如果是最后一帧，就要跳过f那部分
					{
						//第t帧
						ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						jac[JacIndex + 0] = dEIdx0it;  // control point 1
						jac[JacIndex + 1] = dEIdy0it;

						jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it;

						jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it;

						jac[JacIndex + 4] = dEIdx3it;  // control point 2
						jac[JacIndex + 5] = dEIdy3it;


/*

						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;
*/


						JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
						CurrentRowIndex++;
						OffsetInBlock++;													//修改这里
					}
					else{
						//第t帧
						ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						jac[JacIndex + 0] = dEIdx0it;  // control point 1
						jac[JacIndex + 1] = dEIdy0it;

						jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it;

						jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it;

						jac[JacIndex + 4] = dEIdx3it;  // control point 2
						jac[JacIndex + 5] = dEIdy3it;

/*

						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;
*/


						JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

						//第t+1帧 
						ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
						JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
						jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
						jac[JacIndex + 1] = dEIdy0it_1;

						jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
						jac[JacIndex + 3] = dEIdy1it_1;

						jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
						jac[JacIndex + 7] = dEIdy2it_1;

						jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
						jac[JacIndex + 5] = dEIdy3it_1;

/*

						JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
						JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
						JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
						JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
						JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
						JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
						JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
						JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/

						JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
						JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
						JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
						JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
						JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
						JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
						JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
						JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
						CurrentRowIndex++;
						OffsetInBlock++;													//修改这里
					}










				}


			}


			/********************************************************************
										最后一个点
			********************************************************************/

			

			t = 1.0;
			x2 = globs->xAp[globs->NumOfControlPoint-1][f];
			y2 = globs->yAp[globs->NumOfControlPoint-1][f];
			x3 = globs->xCp[globs->NumOfControlPoint-1][f];
			y3 = globs->yCp[globs->NumOfControlPoint-1][f];
			dx =  - 6*x2 + 12*x3 + 9*x2 - 15*x3 ;
			dy =  - 6*y2 + 12*y3 + 9*y2 - 15*y3 ;

			norm1 = norm = sqrt(dx*dx + dy * dy);
			NormalX1 = -dy/norm;
			NormalY1 = dx/norm;

			x2 = globs->xAp[globs->NumOfControlPoint-1][f+1];
			y2 = globs->yAp[globs->NumOfControlPoint-1][f+1];
			x3 = globs->xCp[globs->NumOfControlPoint-1][f+1];
			y3 = globs->yCp[globs->NumOfControlPoint-1][f+1];
			dx =  - 6*x2 + 12*x3 + 9*x2 - 15*x3 ;
			dy =  - 6*y2 + 12*y3 + 9*y2 - 15*y3 ;

			norm2 = norm = sqrt(dx*dx + dy * dy);
			NormalX2 = -dy/norm;
			NormalY2 = dx/norm;

			AlighToSameSideOfEdge(NormalX1,NormalY1,NormalX2,NormalY2);    //修改
			
/*
			if (NormalX1*NormalX2*NormalY1*NormalY2<0)			// 让他们方向一致????????????????????????????????????  修改
			{
				NormalX1 = -NormalX1;
				NormalY1 = -NormalY1;
			}*/





				/**********************************************
					         预先计算好各种导数
				**********************************************/

				double t2 = t*t;
				double t3 = t*t*t;
				double t_13 = (t - 1)*(t - 1)*(t - 1);
				double t_12 = (t - 1)*(t - 1);

				dYHatdy0=dXHatdx0  = -t_13;
				dYHatdy1=dXHatdx1  = 3*t*t_12;
				dYHatdy2=dXHatdx2  = t2*(3*t - 3);
				dYHatdy3=dXHatdx3  = t3 - t2*(6*t - 6);


			for (int k=-K;k<=K;k++)															//修改
			{
				XHatit = globs->x[globs->NumOfCurvePointPerFrame-1][f] + k*NormalX1;
				YHatit = globs->y[globs->NumOfCurvePointPerFrame-1][f] + k*NormalY1;

				XHatit_1 = globs->x[globs->NumOfCurvePointPerFrame-1][f+1] + k*NormalX2;
				YHatit_1 = globs->y[globs->NumOfCurvePointPerFrame-1][f+1] + k*NormalY2;



				// It - It+1
				 dEIdXitHat = ConvolveX(globs->R[f],int(XHatit),int(YHatit),globs->Width, globs->Height)*EIWeight;
				 dEIdYitHat = ConvolveY(globs->R[f],int(XHatit),int(YHatit),globs->Width, globs->Height)*EIWeight;
				 dEIdXit_1Hat = -ConvolveX(globs->R[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height)*EIWeight;
				 dEIdYit_1Hat = -ConvolveY(globs->R[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height)*EIWeight;



				 dXHatdy0 = (3*k*t_12);
				 dXHatdy1 =-(k*(3*t*(2*t - 2) + 3*t_12));
				 dXHatdy2 =-(k*(2*t*(3*t - 3) + 3*t2));
				 dXHatdy3 =(k*(2*t*(6*t - 6) + 3*t2));

				 dYHatdx0 =-dXHatdy0;
				 dYHatdx1 =-dXHatdy1;
				 dYHatdx2 =-dXHatdy2;
				 dYHatdx3 =-dXHatdy3;



				 dEIdx0it = dEIdXitHat * dXHatdx0 + dEIdYitHat * dYHatdx0/norm1;
				 dEIdx1it = dEIdXitHat * dXHatdx1 + dEIdYitHat * dYHatdx1/norm1;
				 dEIdx2it = dEIdXitHat * dXHatdx2 + dEIdYitHat * dYHatdx2/norm1;
				 dEIdx3it = dEIdXitHat * dXHatdx3 + dEIdYitHat * dYHatdx3/norm1;

				 dEIdy0it = dEIdXitHat * dXHatdy0/norm1 + dEIdYitHat * dYHatdy0;
				 dEIdy1it = dEIdXitHat * dXHatdy1/norm1 + dEIdYitHat * dYHatdy1;
				 dEIdy2it = dEIdXitHat * dXHatdy2/norm1 + dEIdYitHat * dYHatdy2;
				 dEIdy3it = dEIdXitHat * dXHatdy3/norm1 + dEIdYitHat * dYHatdy3;

				 dEIdx0it_1 = dEIdXit_1Hat * dXHatdx0 + dEIdYit_1Hat * dYHatdx0/norm2;
				 dEIdx1it_1 = dEIdXit_1Hat * dXHatdx1 + dEIdYit_1Hat * dYHatdx1/norm2;
				 dEIdx2it_1 = dEIdXit_1Hat * dXHatdx2 + dEIdYit_1Hat * dYHatdx2/norm2;
				 dEIdx3it_1 = dEIdXit_1Hat * dXHatdx3 + dEIdYit_1Hat * dYHatdx3/norm2;

				 dEIdy0it_1 = dEIdXit_1Hat * dXHatdy0/norm2 + dEIdYit_1Hat * dYHatdy0;
				 dEIdy1it_1 = dEIdXit_1Hat * dXHatdy1/norm2 + dEIdYit_1Hat * dYHatdy1;
				 dEIdy2it_1 = dEIdXit_1Hat * dXHatdy2/norm2 + dEIdYit_1Hat * dYHatdy2;
				 dEIdy3it_1 = dEIdXit_1Hat * dXHatdy3/norm2 + dEIdYit_1Hat * dYHatdy3;

				if(f==0){//如果是第一帧，那么我们就要跳过f-1的那个部分

					//第t+1帧 
					ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
					jac[JacIndex + 1] = dEIdy0it_1;

					jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it_1;

					jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it_1;

					jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
					jac[JacIndex + 5] = dEIdy3it_1;


/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/



					JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
					CurrentRowIndex++;
					OffsetInBlock++;													//修改这里

				}
				else if (f == globs->NumOfFrame-2)  //如果是最后一帧，就要跳过f那部分
				{
					//第t帧
					ColIndex =NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it;  // control point 1
					jac[JacIndex + 1] = dEIdy0it;

					jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it;

					jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it;

					jac[JacIndex + 4] = dEIdx3it;  // control point 2
					jac[JacIndex + 5] = dEIdy3it;


/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;
*/


					JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

					CurrentRowIndex++;
					OffsetInBlock++;													//修改这里
				}
				else{
					//第t帧
					ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it;  // control point 1
					jac[JacIndex + 1] = dEIdy0it;

					jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it;

					jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it;

					jac[JacIndex + 4] = dEIdx3it;  // control point 2
					jac[JacIndex + 5] = dEIdy3it;

/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;
*/


					JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

					//第t+1帧 
					ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
					jac[JacIndex + 1] = dEIdy0it_1;

					jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it_1;

					jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it_1;

					jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
					jac[JacIndex + 5] = dEIdy3it_1;


/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/


					JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
					CurrentRowIndex++;
					OffsetInBlock++;													//修改这里
				}














				// It - It+1
				 dEIdXitHat = ConvolveX(globs->G[f],int(XHatit),int(YHatit),globs->Width, globs->Height) *EIWeight;
				 dEIdYitHat = ConvolveY(globs->G[f],int(XHatit),int(YHatit),globs->Width, globs->Height) *EIWeight;
				 dEIdXit_1Hat = -ConvolveX(globs->G[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height) *EIWeight;
				 dEIdYit_1Hat = -ConvolveY(globs->G[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height) *EIWeight;


				 dXHatdy0 = (3*k*t_12);
				 dXHatdy1 =-(k*(3*t*(2*t - 2) + 3*t_12));
				 dXHatdy2 =-(k*(2*t*(3*t - 3) + 3*t2));
				 dXHatdy3 =(k*(2*t*(6*t - 6) + 3*t2));

				 dYHatdx0 =-dXHatdy0;
				 dYHatdx1 =-dXHatdy1;
				 dYHatdx2 =-dXHatdy2;
				 dYHatdx3 =-dXHatdy3;



				 dEIdx0it = dEIdXitHat * dXHatdx0 + dEIdYitHat * dYHatdx0/norm1;
				 dEIdx1it = dEIdXitHat * dXHatdx1 + dEIdYitHat * dYHatdx1/norm1;
				 dEIdx2it = dEIdXitHat * dXHatdx2 + dEIdYitHat * dYHatdx2/norm1;
				 dEIdx3it = dEIdXitHat * dXHatdx3 + dEIdYitHat * dYHatdx3/norm1;

				 dEIdy0it = dEIdXitHat * dXHatdy0/norm1 + dEIdYitHat * dYHatdy0;
				 dEIdy1it = dEIdXitHat * dXHatdy1/norm1 + dEIdYitHat * dYHatdy1;
				 dEIdy2it = dEIdXitHat * dXHatdy2/norm1 + dEIdYitHat * dYHatdy2;
				 dEIdy3it = dEIdXitHat * dXHatdy3/norm1 + dEIdYitHat * dYHatdy3;

				 dEIdx0it_1 = dEIdXit_1Hat * dXHatdx0 + dEIdYit_1Hat * dYHatdx0/norm2;
				 dEIdx1it_1 = dEIdXit_1Hat * dXHatdx1 + dEIdYit_1Hat * dYHatdx1/norm2;
				 dEIdx2it_1 = dEIdXit_1Hat * dXHatdx2 + dEIdYit_1Hat * dYHatdx2/norm2;
				 dEIdx3it_1 = dEIdXit_1Hat * dXHatdx3 + dEIdYit_1Hat * dYHatdx3/norm2;

				 dEIdy0it_1 = dEIdXit_1Hat * dXHatdy0/norm2 + dEIdYit_1Hat * dYHatdy0;
				 dEIdy1it_1 = dEIdXit_1Hat * dXHatdy1/norm2 + dEIdYit_1Hat * dYHatdy1;
				 dEIdy2it_1 = dEIdXit_1Hat * dXHatdy2/norm2 + dEIdYit_1Hat * dYHatdy2;
				 dEIdy3it_1 = dEIdXit_1Hat * dXHatdy3/norm2 + dEIdYit_1Hat * dYHatdy3;

				if(f==0){//如果是第一帧，那么我们就要跳过f-1的那个部分

					//第t+1帧 
					ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
					jac[JacIndex + 1] = dEIdy0it_1;

					jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it_1;

					jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it_1;

					jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
					jac[JacIndex + 5] = dEIdy3it_1;


/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/


					JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
					CurrentRowIndex++;
					OffsetInBlock++;													//修改这里
				}
				else if (f == globs->NumOfFrame-2)  //如果是最后一帧，就要跳过f那部分
				{
					//第t帧
					ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it;  // control point 1
					jac[JacIndex + 1] = dEIdy0it;

					jac[JacIndex + 2] = dEIdx1it;  // adjust point 1										//??这里究竟是怎么回事?
					//jac[JacIndex + 3] = dEIdy0it;
					jac[JacIndex + 3] = dEIdy1it;

					jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
					//jac[JacIndex + 7] = dEIdy0it;
					jac[JacIndex + 7] = dEIdy2it;

					jac[JacIndex + 4] = dEIdx3it;  // control point 2											//????????????????
					//jac[JacIndex + 5] = dEIdy0it;
					jac[JacIndex + 5] = dEIdy3it;
/*


					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;
*/


					JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

					CurrentRowIndex++;
					OffsetInBlock++;													//修改这里
				}
				else{
					//第t帧
					ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it;  // control point 1
					jac[JacIndex + 1] = dEIdy0it;

					jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it;

					jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it;

					jac[JacIndex + 4] = dEIdx3it;  // control point 2
					jac[JacIndex + 5] = dEIdy3it;

/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;

*/


					JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

					//第t+1帧 
					ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
					jac[JacIndex + 1] = dEIdy0it_1;

					jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it_1;

					jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it_1;

					jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
					jac[JacIndex + 5] = dEIdy3it_1;

/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/


					JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
					CurrentRowIndex++;
					OffsetInBlock++;													//修改这里
				}













				// It - It+1
				 dEIdXitHat = ConvolveX(globs->B[f],int(XHatit),int(YHatit),globs->Width, globs->Height) *EIWeight;
				 dEIdYitHat = ConvolveY(globs->B[f],int(XHatit),int(YHatit),globs->Width, globs->Height) *EIWeight;
				 dEIdXit_1Hat = -ConvolveX(globs->B[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height) *EIWeight;
				 dEIdYit_1Hat = -ConvolveY(globs->B[f+1],int(XHatit_1),int(YHatit_1),globs->Width, globs->Height) *EIWeight;



				 dXHatdy0 = (3*k*t_12);
				 dXHatdy1 =-(k*(3*t*(2*t - 2) + 3*t_12));
				 dXHatdy2 =-(k*(2*t*(3*t - 3) + 3*t2));
				 dXHatdy3 =(k*(2*t*(6*t - 6) + 3*t2));

				 dYHatdx0 =-dXHatdy0;
				 dYHatdx1 =-dXHatdy1;
				 dYHatdx2 =-dXHatdy2;
				 dYHatdx3 =-dXHatdy3;



				 dEIdx0it = dEIdXitHat * dXHatdx0 + dEIdYitHat * dYHatdx0/norm1;
				 dEIdx1it = dEIdXitHat * dXHatdx1 + dEIdYitHat * dYHatdx1/norm1;
				 dEIdx2it = dEIdXitHat * dXHatdx2 + dEIdYitHat * dYHatdx2/norm1;
				 dEIdx3it = dEIdXitHat * dXHatdx3 + dEIdYitHat * dYHatdx3/norm1;

				 dEIdy0it = dEIdXitHat * dXHatdy0/norm1 + dEIdYitHat * dYHatdy0;
				 dEIdy1it = dEIdXitHat * dXHatdy1/norm1 + dEIdYitHat * dYHatdy1;
				 dEIdy2it = dEIdXitHat * dXHatdy2/norm1 + dEIdYitHat * dYHatdy2;
				 dEIdy3it = dEIdXitHat * dXHatdy3/norm1 + dEIdYitHat * dYHatdy3;

				 dEIdx0it_1 = dEIdXit_1Hat * dXHatdx0 + dEIdYit_1Hat * dYHatdx0/norm2;
				 dEIdx1it_1 = dEIdXit_1Hat * dXHatdx1 + dEIdYit_1Hat * dYHatdx1/norm2;
				 dEIdx2it_1 = dEIdXit_1Hat * dXHatdx2 + dEIdYit_1Hat * dYHatdx2/norm2;
				 dEIdx3it_1 = dEIdXit_1Hat * dXHatdx3 + dEIdYit_1Hat * dYHatdx3/norm2;

				 dEIdy0it_1 = dEIdXit_1Hat * dXHatdy0/norm2 + dEIdYit_1Hat * dYHatdy0;
				 dEIdy1it_1 = dEIdXit_1Hat * dXHatdy1/norm2 + dEIdYit_1Hat * dYHatdy1;
				 dEIdy2it_1 = dEIdXit_1Hat * dXHatdy2/norm2 + dEIdYit_1Hat * dYHatdy2;
				 dEIdy3it_1 = dEIdXit_1Hat * dXHatdy3/norm2 + dEIdYit_1Hat * dYHatdy3;

				if(f==0){//如果是第一帧，那么我们就要跳过f-1的那个部分

					//第t+1帧 
					ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
					jac[JacIndex + 1] = dEIdy0it_1;

					jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it_1;

					jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it_1;

					jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
					jac[JacIndex + 5] = dEIdy3it_1;

/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/


					JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
					CurrentRowIndex++;
					OffsetInBlock++;													//修改这里
				}
				else if (f == globs->NumOfFrame-2)  //如果是最后一帧，就要跳过f那部分
				{
					//第t帧
					ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it;  // control point 1
					jac[JacIndex + 1] = dEIdy0it;

					jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it;

					jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it;

					jac[JacIndex + 4] = dEIdx3it;  // control point 2
					jac[JacIndex + 5] = dEIdy3it;
/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;
*/


					JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

					CurrentRowIndex++;
					OffsetInBlock++;													//修改这里
				}
				else{
					//第t帧
					ColIndex = NumOfParamPerFrame * (f-1) + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it;  // control point 1
					jac[JacIndex + 1] = dEIdy0it;

					jac[JacIndex + 2] = dEIdx1it;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it;

					jac[JacIndex + 6] = dEIdx2it;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it;

					jac[JacIndex + 4] = dEIdx3it;  // control point 2
					jac[JacIndex + 5] = dEIdy3it;
/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it;
*/


					JacBlockMat.setValue2(dEIdx0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);

					//第t+1帧 
					ColIndex = NumOfParamPerFrame * f + 4*CurrentControlPointIndex;
					JacIndex = CurrentRowIndex * ActualNumOfParam + ColIndex;
					jac[JacIndex + 0] = dEIdx0it_1;  // control point 1
					jac[JacIndex + 1] = dEIdy0it_1;

					jac[JacIndex + 2] = dEIdx1it_1;  // adjust point 1
					jac[JacIndex + 3] = dEIdy1it_1;

					jac[JacIndex + 6] = dEIdx2it_1;  // adjust point 2
					jac[JacIndex + 7] = dEIdy2it_1;

					jac[JacIndex + 4] = dEIdx3it_1;  // control point 2
					jac[JacIndex + 5] = dEIdy3it_1;

/*

					JacColForm[ColIndex + 0][CurrentRowIndex] = dEIdx0it_1;
					JacColForm[ColIndex + 1][CurrentRowIndex] = dEIdy0it_1;
					JacColForm[ColIndex + 2][CurrentRowIndex] = dEIdx1it_1;
					JacColForm[ColIndex + 3][CurrentRowIndex] = dEIdy1it_1;
					JacColForm[ColIndex + 6][CurrentRowIndex] = dEIdx2it_1;
					JacColForm[ColIndex + 7][CurrentRowIndex] = dEIdy2it_1;
					JacColForm[ColIndex + 4][CurrentRowIndex] = dEIdx3it_1;
					JacColForm[ColIndex + 5][CurrentRowIndex] = dEIdy3it_1;
*/


					JacBlockMat.setValue2(dEIdx0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 0);
					JacBlockMat.setValue2(dEIdy0it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 1);
					JacBlockMat.setValue2(dEIdx1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 2);
					JacBlockMat.setValue2(dEIdy1it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 3);
					JacBlockMat.setValue2(dEIdx2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 6);
					JacBlockMat.setValue2(dEIdy2it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 7);
					JacBlockMat.setValue2(dEIdx3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 4);
					JacBlockMat.setValue2(dEIdy3it_1,CurrentBlockRowIndex,OffsetInBlock,ColIndex + 5);
					CurrentRowIndex++;
					OffsetInBlock++;													//修改这里
				}


			}

			//这里是完成了对最后一个点的计算，之后必然是转入下一个block的，所以要reset一下
			CurrentBlockRowIndex++;														//修改这里
			OffsetInBlock = 0;															//修改了这里
		}



		//xcounter = CurrentRowIndex;


	}

	JTJJacColSSE(
		JacBlockMat,
		JacBlockMat.NonZeroRowBlockIndex,
		globs->NumOfFrame,
		globs->NumOfControlPoint,
		globs->NumOfParam,
		globs->JTJ);


	cout << "End Of lmjacfunc" << endl;



	/******************************************************************************************************************************************
						
										These are procedures for printing out the resultant matrix.
												Uncomment them to export result.
						
	******************************************************************************************************************************************/
		

	//ofstream out("F:\\personalStuff\\Desktop\\RotoScoping\\JacWrong.txt");


	//cout << "Print Jac" << endl;

	//for (int i=0;i<xcounter;i++)
	//{
	//for (int j=0;j<globs->NumOfParam;j++)
	//{
	//out << setiosflags(ios::left) << setw(15) << jac[i*globs->NumOfParam + j];
	//}
	//out << i << endl;

	//}


	//out.close();
	//system("pause");





/*
	ofstream out1("F:\\personalStuff\\Desktop\\RotoScoping\\JTJ.txt");
	cout << "Print JTJ" << endl;

	for (int i=0;i<globs->NumOfParam;i++)
	{
		for (int j=0;j<globs->NumOfParam;j++)
		{
			out1 << setiosflags(ios::left) << setw(15) << globs->JTJ[i*globs->NumOfParam + j];
		}
		out1 << i << endl;

	}
	out1.close();	
	system("pause");

*/



}








