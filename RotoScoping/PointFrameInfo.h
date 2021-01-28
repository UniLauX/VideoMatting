#ifndef POINTFRAMEINFO_H
#define POINTFRAMEINFO_H

#include "cv.h"
#include "cxcore.h"
#include "cxcore.h"
#include "cvaux.h"
#include "highgui.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "../RotoScoping/PointFrameInfo.h"
using namespace std;


class FrameInfo{
public:
	int FrameId;
	int NumOfControlPoint;

	CvMat * ControlPointMask;			// Store the index of a Control Point in a given pixel. If this pixel is not a Control Point, then its values is -1
	CvMat * BezierAdjustPointsMask;		// Store the index of a Adjust  Point in a given pixel. If this pixel is not a Adjust  Point, then its values is -1


	vector<CvPoint>	ControlPoints;		// point specified by the user (red)
	vector<CvPoint> BezierAdjustPoints; // point use to adjust the Bezier curve (yellow)  
	/******** The size of ControlPoints should be the same as that of the BezierAdjustmentPoints********/
	vector<CvPoint> CurvePoints;		// point on the Bezier curve (blue)

	

	void AssignMemory(int Width ,int Height){
		ControlPointMask = cvCreateMat(Height,Width,CV_32SC1);
		BezierAdjustPointsMask = cvCreateMat(Height,Width,CV_32SC1);


		
	}
	~FrameInfo(){
		cvReleaseMat(&ControlPointMask);
		cvReleaseMat(&BezierAdjustPointsMask);
	}
	



};






#endif