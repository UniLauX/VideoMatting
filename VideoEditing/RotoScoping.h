#ifndef ROTOSCOPING_H
#define ROTOSCOPING_H


#include "nurbs.h"
#include "cv.h"
class NurbsCurveDataManager;
class CxImage;
class ImageSequenceManager;
//class IplImage;

class RotoScoping
{
//public:
//	RotoScoping();
//	~RotoScoping();
//
//public:
//	static void initialize();
//	static void initPointLocation(int ta, int tb);
//	static void rotoScope(int ta, int tb);
//	static void setNurbsNum(int num);
//	//shape term
//	static double eLsigleTerm(ON_2dPoint ti, ON_2dPoint ti1, ON_2dPoint t1i, ON_2dPoint t1i1);
//	static double eCsigleTerm(ON_2dPoint ti, ON_2dPoint ti1, ON_2dPoint ti2, ON_2dPoint t1i, ON_2dPoint t1i1, ON_2dPoint t1i2);
//	static double eVsigleTerm(ON_2dPoint ti, ON_2dPoint t1i);
//	//image term
//	static double eIsigleTerm(int t, int i, int k, ON_2dPoint ti, ON_2dPoint t1i);
//	static double eGsigleTerm(int t, ON_2dPoint ti, double m);
//	static double computSingleM(int ta, int tb, int i);
//	static void computM(int ta, int tb);
//
//	static double distance2(ON_2dPoint& p1, ON_2dPoint& p2);
//	static void pointFitToImage(int& i, int& j);
//	static bool isInImage(int i, int j);
//	static double gradient(CxImage * image, int i, int j);
//
//	static void initilaizePostion();
//	static void initilizeX(double * x, int xnum);
//	/*number of p is the twice of point: 2 * (tb - ta - 1) *  poiNumOfPerFrame;  np
//	 * number of x : el-- (tb - ta) * (poiNumOfPerFrame - 1)
//	 *				 ec-- (tb - ta) * (poiNumOfPerFrame - 2)
//	 *				 ev-- (tb - ta) * poiNumOfPerFrame
//	 *               ei-- (tb - ta) * poiNumOfPerFrame * (kup - kdown  + 1)
//	 *               ev-- (tb - ta + 1) * poiNumOfPerFrame
//	 *           p:  (tb - ta - 1) * poiNumOfPerFrame * 2
//	*/
//	static void func(double * p, double *x, int m, int n, void *data);//LM  function  fk
//	static void jacfunc(double *p, double *jac, int m, int n, void *data);
//
//
//
//	static void getResult();
//	static void simpleNurbsPropagate(int st, int en, int key);
//
//	static void computeK(int ta);
//
//	static void RotoScoping::showImage(CxImage& c, int i);
//
//
//	static ImageSequenceManager imageSequnce; 
//	static Nurbs * vecNurbs;
//	static NurbsCurveDataManager * curveDataManager;
//	static int kdown;
//	static int kup;
//	static int imageheight;
//	static int imagewidth;
//	static double K;
//	static double * M;
//	static int taIndex;
//	static int tbIndex;
//	static int poiNumOfPerFrame;
//	static ON_2dPoint * Direct;
//	static double * position;
//	static ON_2dPoint * posTa;
//	static ON_2dPoint * posTb;
//	static int pnum;
//	static int xnum;
//
//
//	static IplImage ** img;

};



#endif