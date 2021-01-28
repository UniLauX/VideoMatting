#pragma once
#include "../grab/GMM.h"
#include <cximage.h>
#include "../grab/Grabcut.h"

typedef struct window
{
	int centerX;
	int centerY;
	int win_size;
}Local_Win;
class RefineWithTrimap
{
public:
	RefineWithTrimap(void);
	~RefineWithTrimap(void);

	void  Refine(CxImage& srcimg, CxImage& labelimg, CxImage& trimapimg);//refine image
	void Refinewindow(CxImage& srcimg, CxImage& labelimg, CxImage& trimapimg, Local_Win win);
	double calposs( GMMNode& gmm,RGBQUAD rgb);
	double distance(int x1,int y1,int x2,int y2);
	double  coldistance(RGBQUAD c1,RGBQUAD c2);
	double calN(double distance, double coldis, double beta);
	double calBeta(CxImage& colimage, int x1, int x2, int y1, int y2);
	void calculateNweight(NWeight** nweight, int x1, int x2, int y1, int y2, CxImage& srcimg, double beta);
	int m_winsize;
	CxImage tempimg;
};
