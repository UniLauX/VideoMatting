#ifndef BEZIER_H
#define BEZIER_H
#include "cv.h"

struct Normal
{
	double NormalX;
	double NormalY;
};

class Bezier{

	Bezier(){}
	Bezier(const Bezier& in){}
public:
	
	static Bezier * GetInstance();
	void GenerateCurve(CvPoint2D32f x0, CvPoint2D32f x1, CvPoint2D32f x2, CvPoint2D32f x3,int NumOfIntermediatePoint,CvPoint2D32f * Output);
	void GenerateCurveAndNormal(CvPoint2D32f p0, CvPoint2D32f p1, CvPoint2D32f p2, CvPoint2D32f p3,int NumOfIntermediatePoint,CvPoint2D32f * Output,Normal * normals);
};
#endif
