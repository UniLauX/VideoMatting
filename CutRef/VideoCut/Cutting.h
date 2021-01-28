#ifndef CUTTING_H
#define CUTTING_H

#include "../include/CxImage/ximage.h"
#include "../GMM/gmm.h"
#include "../include/MRF/mrf.h"
#include <vector>
using namespace  std;

class GrabCut_ywz
{

	CxImage	 image;
    int *    label;
    int *    info;
    float *  prob;
	CLUSTER_ywz *bgCluster;
	CLUSTER_ywz *fgCluster;

    // Parameters
    int      unkownWidth;
    int      bgWidth;

private:

    void GetCutInfo(int *info);
    void InitGMM(const int *info);
    void GetProb(float *prob);

public:

    GrabCut_ywz();
    ~GrabCut_ywz();

    void Init(const CxImage &image, const int *label);
    void Init(const CxImage &image, const int *label, const float *P);
    void Clear();

    void Cut(int *label, float *prior=NULL);
	void Cut_graphcut(int *label, CxImage& trimap, int pointNum, float * prior=NULL);
	void SolveMRF_graphcut(int w, int h, MRF::CostVal *D,	
		MRF::CostVal *hCues, MRF::CostVal *vCues,int *res, CxImage& trimap, int pointNum);
	void GetTrimapBound(CxImage& trimap, vector<POINT>& vec_bound);
};

#endif