#ifndef MATMULFUNC_H
#define MATMULFUNC_H
#include "CSparseMat.h"
#include "BlockMatProxy.h"

void JTJJacColSSE(
	BlockMatProxy& JacBlockMat,
	std::vector<int> *JacColNonZeroBlockRowIndex,
	int NumOfFrame, 
	int NumOfControlPoint,
	int NumOfParam,
	double * JTJ);

#endif