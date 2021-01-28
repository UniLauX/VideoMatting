#ifndef LMFUNC_H
#define LMFUNC_H



#include "cv.h"
#include "cxcore.h"
#include "cxcore.h"
#include "cvaux.h"
#include "highgui.h"
#include <vector>
#include <fstream>
#include <iostream>
#include "../RotoScoping/CSparseMat.h"
#include "../RotoScoping/Convolve.h"
#include "../RotoScoping/LMData.h"
#include "../RotoScoping/BlockMatProxy.h"



extern CSparseMat dXcurvedXcontrol;			//������ÿ��������ڿ��Ƶ�͵�������Ÿ�Ⱦ���
extern BlockMatProxy JacBlockMat;			//���Ÿ�Ⱦ��󰴷ֿ��ϡ��洢

// m is the number of params, n is the number of measurement
void lmfunc(double *p, double *x, int NumOfParams, int NumOfMeasurement, void *adata);
void lmjacfunc(double *p, double *jac, int m, int n, void *adata);
void GetJacColIndex(LMData * globs);







#endif