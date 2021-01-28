#pragma once
#include <cximage.h>
#include "WmlMathLib.h"
class FlowCalculator_LiuCe
{
public:
	FlowCalculator_LiuCe(void);
	~FlowCalculator_LiuCe(void);
	void OpticalFlow(CxImage& pFrm, CxImage& pRfFrm, float*u, float* v, double alpha = 0.01, double Rt = 0.75, int MnWd = 30, int ItrNm_OtFwPDE = 15, int ItrNm_InFwPDE = 1, int ItrNm_CG = 40);

};

