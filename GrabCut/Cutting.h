#ifndef CUTTING_H
#define CUTTING_H
#include "Global.h"
#include "../VideoEditing/UserScribble.h"
#include <vector>
#include <map>
#include <hash_map>

using namespace std;

class Cutting
{
public:
	int width;
	int height;
	int pixelNum;
	
	CxImage *	image;
	//double *	bgPixels;
	vector<double> bgPixels;
	int         bgPixelsNum;
	CLUSTER *	bgCluster;

	//double *	fgPixels;
	vector<double> fgPixels;
	int		    fgPixelsNum;
	CLUSTER *	fgCluster;

	//int *label;
	unsigned char *       label;
	int         X[2],Y[2];
	int         left,right;
	int         top,bottom;

	Graph *     graph;
	void **     graphPoint;
	double beta;

	//std::map<std::pair<int, int>, int> graphIndex;
	int* graphIndex;
	//stdext::hash_map<std::pair<int, int>, int> graphIndex;
	RGBQUAD     rgb;

	inline bool IsValid(int x, int y);
	void Adjust(int &x1, int &y1, int &x2, int &y2);

	double GetBeta();
	void ImageBeta();
	double GetTLinkWeight(int x, int y, CLUSTER *cluster);
	double CalNLinkWeight(double Beta, int x1, int y1, int x2, int y2);
	void   InitGraph();
	void   InitLocalGraph(vector<POINT> &pixels, CxImage &alpha,
		CLUSTER *fgCluster, CLUSTER *bgCluster, CxImage& localTrimap);

public:
    Cutting();
    ~Cutting();

    void Init();
    void Init(CxImage *image);
    void Clear();

    //void GetCuttingRegion(int x1, int y1, int x2, int y2);
    void Calculate();
    void Update();
    void Update(CxImage &alpha);
    void GetResult(CxImage &result);
    void GetAlphaResult(CxImage &result);

    void SetImage(CxImage *image);

    void UpdateLabel(CxImage &scrib);
    void UpdateLabel(UserScribble& scrib);
    void UpdateLabel(CxImage &alpha, CxImage &scrib);
    void UpdateLabel(CxImage& alpha, UserScribble& scrib);
    void GetLocalCuttingRegion(vector<POINT> &pixels, CLUSTER *fgCluster, CLUSTER *bgCluster);
    void LocalUpdate(vector<POINT> &pixels, CxImage &alpha, CLUSTER *fgCluster, CLUSTER *bgCluster);
    //void LocalCalculate(vector<POINT> &pixels, CxImage &scrib, CxImage &alpha, CxImage& localTrimap);
    void LocalCalculate(vector<POINT> &pixels, UserScribble &scrib, CxImage &alpha, CxImage& localTrimap);
    void SetColor(BYTE r,BYTE g,BYTE b);
    void SetLabel(unsigned char *label);
    void GetTrimap(CxImage &trimap,int dis);

	void GetTrimapBound(vector<POINT>& pixels, vector<POINT>& vec_bound, CxImage& _trimap);
    unsigned char *GetLabels();
    CxImage *GetFrame();
};

#endif