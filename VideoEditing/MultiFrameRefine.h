#pragma once
#include "stdafx.h"
#include "ImageBuffer.h"
#include "../grab/cluster.h"
#include "../grab/BiImageProcess.h"
#include "../include/CxImage/ximage.h"
#include "../grab/GMM.h"
#include "../Graphcut/graph.h"
#include "imageIO.h"
#include "FlowConstructor.h"
class CMultiFrameRefine
{
public:
	CMultiFrameRefine(void);
	~CMultiFrameRefine(void);

	void Refine(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange,int winSize);//local refine window size:2*winSzie+1
	void TrimapRefine(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange,int trimapWidth);//use trimap not windows
private:
	//Members
	void CalOpticalFlow(const CxImage &curr,const CxImage &next,float* u,float* v);//optical flow calculation
	void LocalRefine(int x,int y,Graph* graph,Graph::node_id *gnode,float **u,float **v,int **index,int winSize,ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer);
	void SampleGMM(GMMNode fgmm[COMNUM],GMMNode bgmm[COMNUM],int x,int y,int winSize, CxImage &image, CxImage &label,CxImage &trimap);
	void SampleGMM(GMMNode fgmm[COMNUM],GMMNode bgmm[COMNUM],int x,int y,int winSize, CxImage &image, CxImage &label);

	float CalForPoss(GMMNode fgmm[COMNUM],int gmmindex,RGBQUAD rgb);
	float CalBackPoss(GMMNode bgmm[COMNUM],int gmmindex,RGBQUAD rgb);
	float CalForTWeight(GMMNode fgmm[COMNUM],RGBQUAD rgb);
	float CalBackTWeight(GMMNode bgmm[COMNUM],RGBQUAD rgb);

	double calN(double distance, double coldis);
	double coldistance(RGBQUAD c1,RGBQUAD c2);
	float CalNWeight(RGBQUAD rgb1,RGBQUAD rgb2);
	double GetBeta(CxImage* colimage,int x,int y,int winSize);
	double CalTemporalWeight(int r1,int g1,int b1,int r2,int g2,int b2);

	void GenerateTrimap(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange,int trimapWidth);
	void ConstructGraph(int x,int y,Graph* graph,Graph::node_id *gnode,float **u,float **v,float **u2,float **v2,int **index,int winSize,ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int &count);


	
	//Attribute
	int width,height;
	int rrange;
	double beta;
	int gmmnum;

	//GMMNode fgmm[3];
	//GMMNode bgmm[3];


};
