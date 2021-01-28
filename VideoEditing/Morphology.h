#pragma once
//#include "Algorithm.h"
#include "ZImage.h"

class CMorphology
//: public CAlgorithm
{
public:
	CMorphology(/*CDocument* pDoc = NULL*/);
	virtual ~CMorphology(void);

	static ZByteImage Erosion(ZByteImage& SrcMsk, int KrnSz = 2);
	static ZByteImage Dilation(ZByteImage& SrcMsk, int KrnSz = 2);
	//virtual bool Execute(void);
};
