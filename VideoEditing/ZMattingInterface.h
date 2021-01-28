#pragma once

#include "ZImage.h"

namespace MattingAlgorithm{

class ZMattingInterface
{
public:
	ZMattingInterface(void);
	~ZMattingInterface(void);

public:
	//Input & output Interface: if not available, fill 0
	virtual bool ImageSolve(ZFloatImage* pSrcImg, /*Source Input Image*/
		ZByteImage* pTriMap,/*Input Trimap*/
		ZFloatImage* pBgPrior, /*Input Bg Prior: (r,g,b,w)*/
		ZFloatImage* pDataCost, /*Input Data cost Prior: (bg,fg,weight)*/
		ZFloatImage* pSmoothCost, /*Input Smoothness Prior: (EAST,SOUTH,WEST,NORTH,weight)*/	

		ZByteImage* pAlpha, /*Output Alpha*/
		ZFloatImage* pBgImg, /*Output Bg Image*/
		ZFloatImage* pFgImg /*Output Fg Image*/
		) = 0;

	//Input & output Interface: if not available, fill 0
	virtual bool VideoSolve(FloatImgList* pSrcImg, /*Source Input Images*/
		ByteImgList* pTriMap,/*Input Trimaps*/
		FloatImgList* pBgPrior, /*Input Bg Priors: (r,g,b,w)*/
		FloatImgList* pDataCost, /*Input Data cost Priors: (bg,fg,weight)*/
		FloatImgList* pSmoothCost, /*Input Smoothness Priors: (EAST,SOUTH,WEST,NORTH,weight)*/	
		IntImgList* pTMaps, /*Input termproal prior: (x1,y1,x2,y2)*/
		bool bStaticCamera, /*Camera is static or not*/

		ByteImgList* pAlpha, /*Output Alpha Imgs*/
		FloatImgList* pBgImg, /*Output Bg Images*/
		FloatImgList* pFgImg /*Output Fg Images*/
		) = 0;
};

}
