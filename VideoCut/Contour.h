#ifndef CONTOUR_H
#define CONTOUR_H
#include "../VideoEditing/UserScribble.h"
#include "LocalClassifier.h"
#include "../include/Graphcut/graph.h"
#include "../ClosedFormMatting/Matting.h"
#include <vector>
#include <queue>
#include <list>
#include "ximage.h"


using std::list;
using std::vector;

typedef struct TransClassifierParam_t
{
     /************************************************************************/
     /* Mode:                                                                */
     /* 0--> don't change FG and BG GMM                                      */
     /* 1--> don't change FG GMM and change BG GMM                           */
     /* 2--> don't change BG GMM and change FG GMM                           */
     /* 3--> change FG GMM and BG GMM                                        */
     /************************************************************************/
     int mode;
     // Other parameters...

}TRANS_CLASSIFIER_PARAM;

typedef struct MattingParam_t
{
     int  winSize;
     int  level;
     int  activeLevel;
     int  erodeWinSize;
     double thrAlpha;
     double epsilon;

     MattingParam_t()
     {
          level=1;
          activeLevel=1;
          winSize=1;
          epsilon=0.0000001;
          thrAlpha=0.02;
          erodeWinSize=1;
     }

}MATTING_PARAM;

class Contour
{
     // Frame info


     //int * label;
     unsigned char *    label;
     CxImage *image;

     // Local window info
     int      localWinSize;
     //VVPOINTS winCenters;
     //list<LocalClassifier *> localWins;

     // Bound probability
     VPOINTS  boundPts;
     vector<double> Pf;
     vector<double> Pb;
     char *   boundMask;

     // Graph cut
     Graph *  graph;
     void **  graphPoint;

     
     RGBQUAD rgb;

private:

     void   ClearLocalWins();
     void   GetProb(CxImage& boundPtsIndict);
     double GetNeighborWeight(double Beta, int x1, int y1, int x2, int y2); 

     void InitGraph(CxImage& boundPtsIndict, std::map<std::pair<int, int>, int>& nodeIndex);// for large image 
	 void InitGraph(CxImage& boundPtsIndict, int* nodeIndex);
     void DoCut(unsigned char *label, CxImage& boundPtsIndict);

//private:
public:
     void GetBound(int *flag, VPOINTS &points);
	 void GetConnectedBound(const POINT& sPoint, int* flagImg, VPOINTS& points);
     bool IsValidCoords(int x, int y);
	 float GetWinMinDis( POINT  point, VPOINTS& pointSet );
    // void GetALLWindowCenters(VVPOINTS &vec);
	void GetAllWinowCenters(vector<VPOINTS> &vec);

     void GetOrderedBound(const POINT &sPoint, unsigned char **alpha, int *flag, VPOINTS &points, int num);
     void GetWindowCenter(const VPOINTS &bound, VPOINTS &centers, VVPOINTS &cb);
 

     int  GetNeighbourValue(int x, int y, int dir, int &xtemp, int &ytemp, unsigned char **alpha);
     void GetSelectWin(VPOINTS &selWins, CxImage &scrib);
     void UpdateLocalClassifier(VPOINTS &selWins, const TRANS_CLASSIFIER_PARAM &param);
     void InitCurrentFrame(CxImage &result, CxImage &scrib, 
                           TRANS_CLASSIFIER_PARAM &param);

public:
     vector<LocalClassifier *> localWins;
	 VVPOINTS winCenters;
     Contour();
     ~Contour();
	 int      width;
	 int      height;
	 int      pixelNum;

     void Clear();
     int  GetLocalWinSize();
     void SetFrameInfo(CxImage *frame, unsigned char *l);
     void SetNewLabel(unsigned char *l, int size);
	 unsigned char* GetLabel();
     void SetLocalWinSize(int newWinSize);
     double GetBeta();
	 double GetBeta_WholeImg();

     /************************************************************************/
     /* Clear all local windows and create new ones.                         */
     /************************************************************************/
     void GetLocalClassifier(const TRANS_CLASSIFIER_PARAM &param, int nfirst=0);
     /************************************************************************/
     /*  Propagate local windows to the next frame                           */
     /*  param controls some options.                                        */
     /************************************************************************/
     void TransClassifier(CxImage *nextFrame,
                          VVPOINTS &newCenters, 
                          TRANS_CLASSIFIER_PARAM &param);

     //void UpdateLocal(CxImage &result, CxImage &scrib, 
     //                 TRANS_CLASSIFIER_PARAM &param, unsigned char *label);

     int  GetLocalPixels(CxImage &alpha, CxImage &scrib, VPOINTS &pixels, CxImage& localTrimap);
     int GetLocalPixels(unsigned char* _label, CxImage& scrib, VPOINTS& pixels, CxImage& localTrimap);
     int GetLocalPixels(unsigned char* _label, UserScribble& scrib, VPOINTS& pixels, CxImage& localTrimap);
	 void smoothLabel(unsigned char *label,unsigned char* newlabel,int width,int height);


     void GetLocalWinBound(int &left, int &right, int &top, int &bottom);

     // Notice: the size of the array "label" must be the same as the image.
     void GetCutResultUC(unsigned char *label);
     void GetCutResult(CxImage &res);
     void GetAlphaResult(CxImage &res);

     // Maybe it looks bad.
     vector<LocalClassifier *> *GetLocalWins();
     const VVPOINTS *GetLocalWinCenters();

     // Do matting in the boundary
     void RunMatting(Matting &matting, CxImage &result);
     void RunMatting(MATTING_PARAM &param, CxImage &result);

     void SetColor(BYTE r,BYTE g,BYTE b);

     //test
     void Test();
     void GetOutline();
     void TestLocalClassifier();
     void TestLocalWinPosition(CxImage *image,const VVPOINTS &newcenters);
     void TestParameters();
     void TestResult(int *label);
     void TestSelPixels(VPOINTS &pixels);
	 RGBQUAD GetRGB();
     double   beta;
};


#endif