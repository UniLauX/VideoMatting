// colorcode.cpp
//
// Color encoding of flow vectors
// adapted from the color circle idea described at
//   http://members.shaw.ca/quadibloc/other/colint.htm
//
// Daniel Scharstein, 4/2007
// added tick marks and out-of-range coding 6/05/07
#include "stdafx.h"
#include "colorcode.h"
#include <stdlib.h>
#include <math.h>

int ncols = 0;
#define MAXCOLS 60
int colorwheel[MAXCOLS][3];


void setcols(int r, int g, int b, int k)
{
    colorwheel[k][0] = r;
    colorwheel[k][1] = g;
    colorwheel[k][2] = b;
}

void makecolorwheel()
{
    // relative lengths of color transitions:
    // these are chosen based on perceptual similarity
    // (e.g. one can distinguish more shades between red and yellow 
    //  than between yellow and green)
    int RY = 15;
    int YG = 6;
    int GC = 4;
    int CB = 11;
    int BM = 13;
    int MR = 6;
    ncols = RY + YG + GC + CB + BM + MR;
    //printf("ncols = %d\n", ncols);
    if (ncols > MAXCOLS)
	exit(1);
    int i;
    int k = 0;
    for (i = 0; i < RY; i++) setcols(255,	   255*i/RY,	 0,	       k++);
    for (i = 0; i < YG; i++) setcols(255-255*i/YG, 255,		 0,	       k++);
    for (i = 0; i < GC; i++) setcols(0,		   255,		 255*i/GC,     k++);
    for (i = 0; i < CB; i++) setcols(0,		   255-255*i/CB, 255,	       k++);
    for (i = 0; i < BM; i++) setcols(255*i/BM,	   0,		 255,	       k++);
    for (i = 0; i < MR; i++) setcols(255,	   0,		 255-255*i/MR, k++);
}
#define M_PI       3.14159265358979323846
void computeColor(double fx, double fy, uchar *pix)
{
    if (ncols == 0)
	makecolorwheel();

    double rad = sqrt(fx * fx + fy * fy);
    double a = atan2(-fy, -fx) / M_PI;
    double fk = (a + 1.0) / 2.0 * (ncols-1);
    int k0 = (int)fk;
    int k1 = (k0 + 1) % ncols;
    double f = fk - k0;
    //f = 0; // uncomment to see original color wheel
    for (int b = 0; b < 3; b++) {
	double col0 = colorwheel[k0][b] / 255.0;
	double col1 = colorwheel[k1][b] / 255.0;
	double col = (1 - f) * col0 + f * col1;
	if (rad <= 1)
	    col = 1 - rad * (1 - col); // increase saturation with radius
	else
	    col *= .75; // out of range
	pix[2 - b] = (int)(255.0 * col);
    }
}
