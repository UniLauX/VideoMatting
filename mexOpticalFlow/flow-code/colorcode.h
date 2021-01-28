#pragma once

typedef unsigned char uchar;

void setcols(int r, int g, int b, int k);
void makecolorwheel();

void computeColor(float fx, float fy, uchar *pix);
