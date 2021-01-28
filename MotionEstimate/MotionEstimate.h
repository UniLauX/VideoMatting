#pragma once
#include "cximage.h"
#include "..\VideoCut\LocalClassifier.h"
#include "WmlMathLib.h"
#include "colorcode.h"
#include <cv.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
//#include <..\include\mexOpticalFlow\mex>
#include "..\mexOpticalFlow\mex\OpticalFlow.h"
//using namespace std;



class MotionEstimate
{
public:
	MotionEstimate(void);
	~MotionEstimate(void);

	void getCentersOfNextFrame(CxImage& preImage,CxImage& curImage,
		const unsigned char *preLabel,unsigned char* newLabel,const VVPOINTS&localWins,VVPOINTS& newCenters,int winSize);
	static float GetDistance( int x1, int y1, int x2, int y2 );
	static CxImage GetCombinedMaskImg( CxImage dstImg, unsigned char* dstLabelMask, float alpha );
	static void SmoothTransferMask(CxImage dstImg, unsigned char* labelMask );




	static Wml::GMatrix<Wml::Vector2f> GenOpticalFlow(CxImage srcImg, CxImage dstImg,const RECT& contrRct,double alpha = 0.01, double Rt = 0.75, int MnWd = 30, int ItrNm_OtFwPDE = 15, int ItrNm_InFwPDE = 1, int ItrNm_CG = 40);
	static Wml::GMatrix<float> GenOpticalFlowError( const Wml::GMatrix<Wml::Vector2f>& OptFlwMatrixForeward, const Wml::GMatrix<Wml::Vector2f>& OptFlwMatrixBackward );
	Wml::GMatrix<Wml::Vector2f> GenMeanFlowInContour( const Wml::GMatrix<Wml::Vector2f>& OptFlwMatrix,const unsigned char *preLabel,CxImage& preImage,int localWinSize,const RECT& contrRct );
	static void PropagateLyrMaskByMeanFlow(CxImage& preImage,CxImage& curImage,const unsigned char *preLabel,unsigned char* newLabel, const Wml::GMatrix<Wml::Vector2f>& MeanOptFlwMatrix,const RECT& contrRct );
	static void PropagateWinsByMeanFlow( CxImage srcFrm, CxImage& dstFrm,const unsigned char *preLabel,unsigned char* newLabel,const VVPOINTS&localWins,VVPOINTS& newCenters,int winSize, const Wml::GMatrix<Wml::Vector2f>& MeanOptFlwMatrix,const RECT& contrRct );
};
