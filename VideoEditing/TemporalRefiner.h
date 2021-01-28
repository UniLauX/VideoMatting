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
#include "WmlGMatrix.h"
#define  NUMOFC 3;

class CTemporalRefiner
{
public:
	CTemporalRefiner(void);
	~CTemporalRefiner(void);
	void Refine(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange,int trimapWidth);
private:
	void CalGMM();
	void SampleColor(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange,int trimapWidth);
	void UpdateGmmParam();
	void UpdateGmmParam(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange);
	void CalGmmParam(GMMNode &gmm,vector<RGBQUAD> vec,int total);

	float CalForPoss(int gmmindex,RGBQUAD rgb);
	float CalBackPoss(int gmmindex,RGBQUAD rgb);

	float CalFMi(int gmmindex,RGBQUAD rgb);
	float CalBMi(int gmmindex,RGBQUAD rgb);

	double GetBeta(CxImage* image);
	double calN(double distance, double coldis);


	double coldistance(RGBQUAD c1,RGBQUAD c2);

	void constructGraph(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange);

	void updateGraph(ImageBuffer * alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange);
	float CalForTWeight(RGBQUAD rgb);
	float CalBackTWeight(RGBQUAD rgb);
	float CalNWeight(RGBQUAD rgb1,RGBQUAD rgb2);
	double Distance(int x1,int y1,int x2,int y2);
	double CalTemporalWeight(int r1,int g1,int b1,int r2,int g2,int b2);

	
	int  DoSeg(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange);

	void CalOpticalFlow(const CxImage &curr,const CxImage &next,float* u,float* v);
	
	vector<colorNode> fortempvec;//all forground pixel
	vector<colorNode> backtempvec;//all background pixel
	GMMNode fgmm[COMNUM];
	GMMNode bgmm[COMNUM];
	double beta;
	Graph *graph;
	int **index;
	Graph::node_id *gnode;
	int w;
	int h;

};
