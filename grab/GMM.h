#pragma  once

#include "../VideoEditing/stdafx.h"
#include "../include/CxImage/ximage.h"
#include "../include/Matrix Pro/cmatrix"
#include "cluster.h"
#include <vector>
using namespace std;
#define  COMNUM 5

typedef struct GMM_node
{
	dMatrix u;//mean of this model
	dMatrix inverseConviance;//inverse of conviance matrix
	double pi;//weight
	double det;//determin of conviance
	int count;

}GMMNode;
void initGMM(vector< vector <colorNode>> &vec,GMMNode node[COMNUM]);//根据第一次cluster的结果初始化GMM
void calGMMPara(GMMNode &gmm,vector<pixelNode> &vec,int total,CxImage &image);

