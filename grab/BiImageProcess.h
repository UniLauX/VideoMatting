#ifndef BIMAGE_H
#define BIMAGE_H

#include <vector>
#include <stack>
#include <list>
#include <iostream>
#include <algorithm>
#include <vector>
#include "../include/CxImage/ximage.h"
#include "../VideoEditing/KConnectedComponentLabeler.h"
#include "../VideoCut/LocalClassifier.h"

#define NMAX 1000
using namespace std;

namespace BiImageProcess
{
//typedef std::vector<POINT> VPOINTS;
//typedef std::vector<VPOINTS> VVPOINTS;

bool isValid(int x,int y,int w,int h);
 bool BSInterpolation(float **alphaSource,float **alphaDes,int oldX,int oldY,int newX,int newY);//Bcubic split interpolation
 void checkConnectivity(float **llabel, int w, int h);
 void checkConnectivity(unsigned char *label,int w,int h, int minarea=1000);
 void checkConnectivity(CxImage &label);
 void fillHoles(unsigned char *label,int w,int h, int minarea=50);
 void checkConnectivityfast(unsigned char* l, int w, int h);
 void dilate(float** label,int w,int h,int k);
 void erode(float**label,int w,int h,int k);
 void getBound(std::vector<POINT> &bound, int **label,int xStart,int yStart,int xEnd,int yEnd);
 void getBound(std::vector<POINT> &bound,unsigned char *label,int xStart,int yStart,int xEnd,int yEnd,int w,int h);
 void getBound(std::vector<POINT> &bound,CxImage* image);
 void getBound_ex(std::vector<POINT> &bound, CxImage* image);
bool neighbourNCaled(int i,int j,int w,int h,int **P);
int NeighbourMax(int i,int j,int w,int h,int **P);
 double KernelBSpline(const float x);
 void manhattan(float **label,int w,int h);
 void UpdateCoexists(int i,int j,int w,int h,int **P,int **coexist);
 void GetTrimap(unsigned char *soreceLabel,CxImage& trimap,int dis,int w,int h);
 void GetTrimap(CxImage* label,CxImage& trimap,int dis);
 float distance(int x1,int y1,int x2,int y2);
 void FindBoundBox(CxImage* label,int& left,int &top,int &right,int &bottom);

 void GetAllWindowsCenters(VVPOINTS &vec,int *label,int width,int height,int localWinSize);//label must indicate 0 or 1
  void GetBound(int *label,int *flag, VPOINTS &points,int width,int height);
  void GetOrderedBound(const POINT &sPoint, unsigned char **alpha, int *flag, VPOINTS &points, int num, int width, int height);
  int GetNeighbourValue(int x, int y, int dir,  int &xtemp, int &ytemp, unsigned char  **alpha,int width,int height);
  void GetWindowCenter(const VPOINTS &bound, VPOINTS &centers, vector<VPOINTS> &cb,int localWinSize);
  double GetDistance(const POINT &p1, const POINT &p2);
  bool IsInWindow(const POINT &point, const POINT &center, int size);
  bool checkNeighbour(int x, int y, unsigned char** mask);
}
#endif