#pragma once
//#include "Algorithm.h"
#include "ZImage.h"
#include "cximage.h"

class CDisparity
	//: public CAlgorithm
{
public:
	CDisparity(/*CDocument* pDoc = NULL*/void);
	virtual ~CDisparity(void);

	static ZFloatImage LoadDspMap(int iWidth, int iHeight, const CString& DspFileName);
	static bool SaveDspMap(ZFloatImage& DspMap, const CString& DspFileName);

	static ZByteImage DspMap2ByteImg(ZFloatImage& DspMap, float fDspMin = 1.0e-6, float fDspMax = 0.01);
	static CxImage DspMap2CxImg(ZFloatImage& DspMap, float fDspMin = 1.0e-6, float fDspMax = 0.01);
	static ZFloatImage CxImg2DspMap(CxImage& DspXImg, float fDspMin = 1.0e-6, float fDspMax = 0.01);
	static ZFloatImage ByteImg2DspMap(ZByteImage& DspImg, float fDspMin = 1.0e-6, float fDspMax = 0.01);
	//virtual bool Execute(void);
};
