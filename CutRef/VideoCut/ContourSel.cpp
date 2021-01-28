#include "Contour.h"
#include <time.h>
#include <iostream>

using namespace std;


static void DrawRectangle(CxImage &image, int x1, int y1, int x2, int y2)
{
     RGBQUAD rgb;

     rgb.rgbRed=255;
     rgb.rgbGreen=0;
     rgb.rgbBlue=0;

     image.DrawLine(x1, x2, y1, y1, rgb);
     image.DrawLine(x2, x2, y1, y2, rgb);
     image.DrawLine(x1, x2, y2, y2, rgb);
     image.DrawLine(x1, x1, y1, y2, rgb);
}
