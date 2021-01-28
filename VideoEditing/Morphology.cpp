#include "StdAfx.h"
#include "Morphology.h"
#include "Z:/Vision/Others/ltilib/headerFiles/ltiBinaryKernels.h"
#include "Z:/Vision/Others/ltilib/headerFiles/ltiErosion.h"
#include "Z:/Vision/Others/ltilib/headerFiles/ltiDilation.h"
#include "Z:/Vision/Others/ltilib/ltiliblink.h"

CMorphology::CMorphology(/*CDocument* pDoc*/)
//: CAlgorithm(pDoc)
{
}

CMorphology::~CMorphology(void)
{
}

ZByteImage CMorphology::Erosion(ZByteImage& SrcMsk, int KrnSz)
{
	int iWidth = SrcMsk.GetWidth();
	int iHeight = SrcMsk.GetHeight();

	lti::erosion Erosion;                   // the erosion functor
	lti::erosion::parameters ErosionParam; // the parameters
	lti::cityBlockKernel<float> Kern(KrnSz * 2 + 1);   // diamond shaped kernel
	// binary erosion
	ErosionParam.mode = lti::erosion::parameters::Binary;
	ErosionParam.setKernel(Kern);
	// set the parameters
	Erosion.setParameters(ErosionParam);
	// apply the erosion to a channel "src" and leave the result in "dest"
	lti::channel8 ErsSrc(iWidth, iHeight);
	for(int y = 0; y < iHeight; y++)
	{
		for(int x = 0; x < iWidth; x++)
		{
			ErsSrc.at(x, y) = SrcMsk.at(x, y);
		}
	}
	lti::channel8 ErsDst;
	Erosion.apply(ErsSrc, ErsDst);

	ZByteImage DstMsk(iWidth, iHeight);
	for(int y = 0; y < iHeight; y++)
	{
		for(int x = 0; x < iWidth; x++)
		{
			DstMsk.at(x, y) = ErsDst.at(x, y);
		}
	}
	return DstMsk;
}

ZByteImage CMorphology::Dilation(ZByteImage& SrcMsk, int KrnSz)
{
	int iWidth = SrcMsk.GetWidth();
	int iHeight = SrcMsk.GetHeight();

	lti::dilation Dilation;                   // the erosion functor
	lti::dilation::parameters DilationParam; // the parameters
	lti::cityBlockKernel<float> Kern(KrnSz * 2 + 1);   // diamond shaped kernel
	// binary erosion
	DilationParam.mode = lti::dilation::parameters::Binary;
	DilationParam.setKernel(Kern);
	// set the parameters
	Dilation.setParameters(DilationParam);
	// apply the dilation to a channel "src" and leave the result in "dest"
	lti::channel8 DltSrc(iWidth, iHeight);
	for(int y = 0; y < iHeight; y++)
	{
		for(int x = 0; x < iWidth; x++)
		{
			DltSrc.at(x, y) = SrcMsk.at(x, y);
		}
	}
	lti::channel8 DltDst;
	Dilation.apply(DltSrc, DltDst);

	ZByteImage DstMsk(iWidth, iHeight);
	for(int y = 0; y < iHeight; y++)
	{
		for(int x = 0; x < iWidth; x++)
		{
			DstMsk.at(x, y) = DltDst.at(x, y);
		}
	}
	return DstMsk;
}

//bool CMorphology::Execute(void)
//{
//}