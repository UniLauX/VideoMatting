#ifndef CONVOLVE_H
#define CONVOLVE_H


#define MAX_WINDOW_WIDTH 21

#include "cv.h"
#include <cmath>

struct ConvolveKernel{
	double data[MAX_WINDOW_WIDTH];
	int Width;
};

void GenerateKernel(double sigma,ConvolveKernel & GaussKernel,ConvolveKernel & GaussDeriv);
double ConvolveX(double * Image,int Cx,int Cy,const ConvolveKernel& Gauss,const ConvolveKernel & GaussDeriv,int Width);
double ConvolveY(double * Image,int Cx,int Cy,const ConvolveKernel& Gauss,const ConvolveKernel & GaussDeriv,int Width);
double ConvolveX(unsigned char * Image,int Cx,int Cy,const ConvolveKernel& Gauss,const ConvolveKernel & GaussDeriv,int Width);
double ConvolveY(unsigned char * Image,int Cx,int Cy,const ConvolveKernel& Gauss,const ConvolveKernel & GaussDeriv,int Width);
double ConvolveY(unsigned char * Image,int Cx,int Cy,int Width, int Height);
double ConvolveX(unsigned char * Image,int Cx,int Cy,int Width, int Height);
double ConvolveY(double * Image,int Cx,int Cy,int Width);
double ConvolveX(double * Image,int Cx,int Cy,int Width);
#endif