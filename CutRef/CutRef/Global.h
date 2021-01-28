//#ifndef GLOBAL_H
//#define GLOBAL_H
#pragma once


#include "../include/CxImage/ximage.h"
#include "../include/MRF/mrf.h"
#include "../include/MRF/GCoptimization.h"
#include <vector>

using std::vector;


inline int Near(float a)
{
    return (int)(a+0.5);
}

inline float Norm_2(RGBQUAD &r1, RGBQUAD &r2)
{
    return sqrt((float)(r1.rgbRed-r2.rgbRed)*(r1.rgbRed-r2.rgbRed)+
        (r1.rgbGreen-r2.rgbGreen)*(r1.rgbGreen-r2.rgbGreen)+
        (r1.rgbBlue-r2.rgbBlue)*(r1.rgbBlue-r2.rgbBlue));
}

float *LoadData(const char *name, int &w, int &h);
void   SaveData(const char *name, int w, int h, const float *d);

void NormalizeDataCost(MRF::CostVal *dc, int w, int h, int disRange);
void GetCues(CxImage &A, MRF::CostVal *hCue, MRF::CostVal *vCue);
void SolveMRF(int w, int h, MRF::CostVal *D, float smooth, 
              MRF::CostVal *hCues, MRF::CostVal *vCues, int dis, int times, int *res);

void OpticalFlow(CxImage &from, CxImage &to, float *u, float *v);

void GetResult(int w, int h, const int *data, CxImage &res);
void GetResult(int w, int h, float Max, const float *data, CxImage &res);

bool LoadImage(const char *dir, const char *ext, int num, CxImage &image);
bool LoadImage(const char* dir, const char* ext, CxImage &image);
void ImageToGray(CxImage &img, int *gray);
void ImageToLabel(CxImage &image, int *label);

struct OptData
{
    int   num;
    float *data;
    float *r;

    OptData():data(NULL),r(NULL){}

    void Create(int n)
    {
        Destroy();
        data=new float[n];
        r=new float[n];
    }

    void Destroy()
    {
        delete [] data;
        delete [] r;
    }
};

void CheckMemory();

//#endif