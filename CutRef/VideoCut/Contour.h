#ifndef CONTOUR_H
#define CONTOUR_H

#include "LocalClassifier.h"
#include "../include/Graphcut/graph.h"
#include <vector>
#include <list>

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

class Contour_ywz
{
     // Frame info
     int      width;
     int      height;
     int      pixelNum;
     int *    label;
     CxImage *image;

     // Local window info
     int      localWinSize;
     VVPOINTS winCenters;
     list<LocalClassifier_ywz *> localWins;

     // Bound probability
     VPOINTS  boundPts;
     vector<double> Pf;
     vector<double> Pb;

private:

     void ClearLocalWins();
     void GetProb();
     void DoCut(int *label);

private:

     void GetBound(int *flag, VPOINTS &points);
     void GetOrderedBound(const POINT &sPoint, int **alpha, int *flag, VPOINTS &points, int num);
     void GetWindowCenter(const VPOINTS &bound, VPOINTS &centers);
	 void GetWindowCenter_jin(const VPOINTS &bound, VPOINTS &centers, vector<VPOINTS>& cb);
     void GetALLWindowCenters(VVPOINTS &vec);
     int  GetNeighbourValue(int x, int y, int dir, int &xtemp, int &ytemp, int **alpha);
     void GetLocalWinBound(int &left, int &right, int &top, int &bottom);

public:

     Contour_ywz();
     ~Contour_ywz();

     void Clear();
     int  GetLocalWinSize();
     void SetFrameInfo(CxImage *frame, int *l);
     void SetNewLabel(int *l, int size);
     void SetLocalWinSize(int newWinSize);
     double GetBeta();

     /************************************************************************/
     /* Clear all local windows and create new ones.                         */
     /************************************************************************/
     void GetLocalClassifier();

     // Notice: the size of the array "label" must be the same as the image.
     void GetCutResult(int *label);
     void GetAlphaResult(CxImage &res);
     void GetFGProb(float *P, CxImage& trimap, int& pointNum);

     //test
     void Test();
     void GetOutline();
     void TestLocalWinPosition();
};


#endif