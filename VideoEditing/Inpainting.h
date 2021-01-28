#pragma once
//#include "Algorithm.h"
#include "ZImage.h"
#include "WmlMathLib.h"

class CInpainting
//: public CAlgorithm
{
public:
	CInpainting(void);
	virtual ~CInpainting(void);

	static ZByteImage ExemplarBasedInpainting(ZByteImage& SrcImg, ZByteImage& IptMsk, int W = 4);
	static ZByteImage ExemplarBasedInpainting(ZByteImage& SrcImg, int W = 4);

	static bool StereoInpainting(ZByteImage& DstImg, ZFloatImage& DstDsp, ZByteImage& SrcImg, ZFloatImage& SrcDsp, ZByteImage& IptMsk, float fDspMin = 1e-6, float fDspMax = 0.01, int W = 4, float fDspWt1 = 13.0f, float fDspWt2 = 30.0);

	static ZFloatImage DepthInpainting(ZFloatImage& SrcDsp, ZIntImage& IptMsk);

	static ZFloatImage OptFlowInpainting(ZFloatImage& SrcOptFlow, ZIntImage& IptMsk);

private:
	static Wml::Vector2f BoundaryNormal(const Wml::Vector2<int>& P, ZByteImage& RegFilMap);

	static Wml::Vector2<int> m_NbOfSt[8];
};
