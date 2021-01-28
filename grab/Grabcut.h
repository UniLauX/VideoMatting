#pragma once
#include <iostream>
#include <fstream>
#include "GMM.h"
#include "../include/Graphcut/graph.h"
#include "../include/CxImage/ximage.h"
#include <map>
//#include "../GrabCut/Global.h"

#define NMAX 1000
#define  MIN_WIDTH 200;
using namespace std;

typedef struct Nweight
{
	double up;
	double down;
	double left;
	double right;

	int upIndex;
	int downIndex;
	int leftIndex;
	int rightIndex;

}NWeight;

class Grabcut
{
public:
	Grabcut(void);
	//Grabcut(CxImage &image,CPoint s,CPoint e);
	void init(CxImage &image,int x1,int y1,int x2,int y2,RECT rect);
	~Grabcut(void);


	int dosegonce(int num);
	void doseg();
	void interactiveOnce();

	void setForAlpha(int x,int y);
	void setForTrip(int x,int y);
	void setBackAlpha(int x,int y);
	void setBackTrip(int x,int y);
	void GetResult(CxImage &result);
	void GetAlphaResult(CxImage &result);
	CxImage* GetFrame();
	void SetImage(CxImage *image);
	unsigned char* GetLabels();
	void LocalCalculate(vector<POINT> pixels,CxImage& scrible);
	void UpdateLabel(CxImage &alpha, CxImage &scrib);
	void UpdateLabel(CxImage &scrib);
	void RRelease();
	void RReleaseHighLevel();
	void RReleaseLowLevel();
	void SetLabel();

	void SetLabel(unsigned char *label);
	//void GetTrimap(CxImage &trimap,int dis);
	void SetColor(BYTE r,BYTE g,BYTE b);

private:

	void initializeGMM();
	void reInitializeGMM();
	void assignGMM();
	void assignGMM2();
	void recalGMM();
	void recalGMM2();
	void constructgraph();

	double calposs(double alpha,int gmmindex,RGBQUAD rgb);
	double distance(int x1,int y1,int x2,int y2);
	double coldistance(RGBQUAD c1,RGBQUAD c2);
	double calBeta();
	void calculateNweight();
	double calN(double distance,double coldis);

	void getBound(vector<POINT> &bound, float **label,CxImage& tag,int w,int h);
	void assignGMMHighlevel(std::map<std::pair<int, int>, int>& comp,CxImage&  tag);//vector<POINT> &bound);
	void recalGMMHighlevel(std::map<std::pair<int, int>, int>& comp,CxImage& tag);//vector<POINT> &bound);
	void highLevelCut();
	int  highLevelCutOnce(const vector<POINT> &bound,Graph::node_id **graphNode,CxImage& tag);


	void calHighLevelNWeight(NWeight **weight);//,vector<POINT> &bound);
	double calHighLevelNWeight(int x1, int y1, int x2, int y2);
	void calLocalNWeight(vector<POINT> &pixels,Nweight ** weight);
	//local update
	void initLocalGMM(vector<POINT>& pixels);
	void constructLocalGraph(vector<POINT>& pixels,CxImage& scribble,NWeight **weight,Graph::node_id **localGNode);
	//interpolation
	bool BSInterpolation(float **alphaSource,float **alphaDes,int oldX,int oldY,int newX,int newY);
	double KernelBSpline(const float x);
	
	//binary image process
	void checkConnectivity(float **label,int w,int h);
	void checkConnectivity(unsigned char *label,int w,int h);
	void manhattan(float **label,int w,int h);
	void dilate(float** label,int w,int h,int k);
	void erode(float**label,int w,int h,int k);

	bool point_equal (POINT ptFirst, POINT ptSecond)
	{
		return (ptFirst.x == ptSecond.x && ptFirst.y == ptSecond.y);
	}

	//test
	void save(int num);

	public:
	int **component;//存储每个pixel对应的GMM的index
	float **alpha;
	float **finalAlpha;
	int ** triBack;//表示background的pixel
	int **triFor;//标示forground的pixel
	int **triUnknown;//表示未知区域pixel
	NWeight **nweight;
        int  width;
	int height;
	int xstart;
	int xend;
	int ystart;
	int yend;
	Graph::node_id **gnode;
	GMMNode fgmm[COMNUM];
	GMMNode bgmm[COMNUM];
	CxImage originalImage;//non_cropped image
	CxImage sourceImage;//cropped image
	CxImage colimage;//resampled cropped image
	Graph *graph;
	double beta;
	ofstream outf;
	bool first;
	 
	vector<POINT> bound;
	int scalar;
	double maxDis;
	int centerX,centerY;

	Graph::node_id **graphNode;//remember to release
	RGBQUAD back_rgb;
private:
	RECT cropRect;
	//int *label;
	unsigned char* label;
};


static inline bool isValid(int x,int y,int w,int h)
{
	return (x>=0&&x<w&&y>=0&&y<h);
}

static inline bool neighbourNCaled(int i,int j,int w,int h,unsigned char **P)
{
	int x=i;
	int y=j;

	int temp1=0,temp2=0,temp3=0,temp4=0;

	if (isValid(x-1,y,w,h))
	{
		if(P[x-1][y])
			temp1=1;
		else
			temp1=0;
	}
	else
	{
		temp1=0;
	}

	if (isValid(x,y-1,w,h))
	{
		if(P[x][y-1])
			temp2=1;
		else
			temp2=0;
	}
	else
	{
		temp2=0;
	}

	if (isValid(x-1,y-1,w,h))
	{
		if(P[x-1][y-1])
			temp3=1;
		else
			temp3=0;
	}
	else
		temp3=0;

	if (isValid(x-1,y+1,w,h))
	{
		if (P[x-1][y+1])
			temp4=1;
		else
			temp4=0;
	}
	else 
		temp4=0;

	return (temp1||temp2||temp3||temp4);
}

static inline int NeighbourMax(int i,int j,int w,int h,unsigned char **P)
{
	int temp1=0,temp2=0,temp3=0,temp4=0;

	if(isValid(i-1,j,w,h))
		temp1=P[i-1][j];

	if (isValid(i-1,j-1,w,h))
		temp2=P[i-1][j-1];

	if (isValid(i-1,j+1,w,h))
		temp3=P[i-1][j+1];

	if(isValid(i,j-1,w,h))
		temp4=P[i][j-1];


	return _MAX(temp4,_MAX(temp3,_MAX(temp2,temp1)));

}

static inline void UpdateCoexists(int i,int j,int w,int h,unsigned char **P,unsigned char **coexist)
{
	if(isValid(i-1,j,w,h))
	{
		coexist[P[i][j]][P[i-1][j]]=1;
		coexist[P[i-1][j]][P[i][j]]=1;
	}
	if (isValid(i-1,j-1,w,h))
	{
		coexist[P[i][j]][P[i-1][j-1]]=1;
		coexist[P[i-1][j-1]][P[i][j]]=1;
	}
	if (isValid(i-1,j+1,w,h))
	{
		coexist[P[i][j]][P[i-1][j+1]]=1;
		coexist[P[i-1][j+1]][P[i][j]]=1;

	}
	if(isValid(i,j-1,w,h))
	{
		coexist[P[i][j]][P[i][j-1]]=1;
		coexist[P[i][j-1]][P[i][j]]=1;

	}
}