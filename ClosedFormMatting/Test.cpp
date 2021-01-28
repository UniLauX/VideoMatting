#include "Matting.h"
#include <fstream>

using namespace std;
using namespace Wml;

static ofstream out("out.txt");

void Matting::Test(CxImage &image, int newX, int newY, char *name)
{
	int i,j;
	int w,h;
	RGBQUAD rgb;
	GMatrix<REAL> R,G,B;
	GMatrix<REAL> res_R,res_G,res_B;

	w=image.GetWidth();
	h=image.GetHeight();
	R.SetSize(h, w);
	G.SetSize(h, w);
	B.SetSize(h, w);

	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{
			rgb=image.GetPixelColor(j, i);
			R[i][j]=rgb.rgbRed/255.0;
			G[i][j]=rgb.rgbGreen/255.0;
			B[i][j]=rgb.rgbBlue/255.0;
		}
	}
	UpSmpIm(R, newX, newY, 1, res_R);
	UpSmpIm(G, newX, newY, 1, res_G);
	UpSmpIm(B, newX, newY, 1, res_B);
	
	CxImage I(newX, newY, 24);
	for(i=0;i<newY;++i)
	{
		for(j=0;j<newX;++j)
		{
			rgb.rgbRed=(int)(res_R[i][j]*255);
			rgb.rgbGreen=(int)(res_G[i][j]*255);
			rgb.rgbBlue=(int)(res_B[i][j]*255);
			I.SetPixelColor(j, i, rgb);
		}
	}
	I.Save(name, CXIMAGE_FORMAT_BMP);
}

void Matting::TestSolveAlpha(char *name)
{
	REAL *alpha=DoMatting();
	TestAlpha(alpha, image.GetWidth(), image.GetHeight(), name);
	delete [] alpha;
}

void Matting::TestMatValues(GMatrix<REAL> &mat, char *name)
{
	int w,h;
	int i,j;
	REAL tmp;

	w=mat.GetColumns();
	h=mat.GetRows();

	RGBQUAD rgb;
	CxImage image(w, h, 24);
	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{
			tmp=mat[i][j]*255;
			if(tmp>255)
				tmp=255;
			if(tmp<0)
				tmp=0;
			rgb.rgbRed=(int)tmp;
			rgb.rgbGreen=(int)tmp;
			rgb.rgbBlue=(int)tmp;
			image.SetPixelColor(j, i, rgb);
		}
	}

	image.Save(name, CXIMAGE_FORMAT_BMP);
}

void Matting::TestMatValues(Wml::GMatrix<REAL> &mat)
{
	int i,j;
	int w,h;

	w=mat.GetColumns();
	h=mat.GetRows();
	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{
			out << mat[i][j] << " ";
		}
		out << endl;
	}
	out << "-------------------------" << endl;
}

void Matting::TestAlpha(REAL *alpha, int w, int h, char *name)
{
	int i,j,index;
	REAL tmp;
	
	RGBQUAD rgb;
	CxImage image(w, h, 24);
	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{
			index=i*w+j;
			tmp=alpha[index]*255;
			rgb.rgbRed=(int)tmp;
			rgb.rgbGreen=(int)tmp;
			rgb.rgbBlue=(int)tmp;
			image.SetPixelColor(j, i, rgb);
		}
	}
	
	image.Save(name, CXIMAGE_FORMAT_BMP);
}

void Matting::TestLinearCoeff(CxImage &image, CxImage &alpha)
{
	int w,h;
	int i,j,index;
	REAL *alp;
	RGBQUAD rgb;
	GMatrix<REAL> A_R;
	GMatrix<REAL> A_G;
	GMatrix<REAL> A_B;
	GMatrix<REAL> B;

	w=image.GetWidth();
	h=image.GetHeight();
	alp=new REAL[w*h];

	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{
			index=i*w+j;
			rgb=alpha.GetPixelColor(j, i);
			alp[index]=rgb.rgbRed/255.0;
		}
	}

	GetLinearCoeff(alp, image, A_R, A_G, A_B, B);
	TestMatValues(A_R, "A_R.bmp");
	TestMatValues(A_G, "A_G.bmp");
	TestMatValues(A_B, "A_B.bmp");
	TestMatValues(B, "B.bmp");

	delete [] alp;
}

void Matting::TestErode(CxImage &image, char *name)
{
	int w,h;
	int x,y;
	int i,j;
	int tmp1,tmp2;
	int width,height;
	int erodeWin=2;
	int flag;
	RGBQUAD rgb;
	CxImage alpha(image);
	
	width=image.GetWidth();
	height=image.GetHeight();
	w=width-erodeWin;
	h=height-erodeWin;
	for(y=0;y<height;++y)
	{
		for(x=0;x<width;++x)
		{
			rgb=image.GetPixelColor(x, y);
			if(rgb.rgbRed/255.0>thrAlpha)
			{
				rgb.rgbRed=255;
				rgb.rgbGreen=255;
				rgb.rgbBlue=255;
			}
			else
			{
				rgb.rgbRed=0;
				rgb.rgbGreen=0;
				rgb.rgbBlue=0;
			}
			alpha.SetPixelColor(x, y, rgb);
		}
	}

	CxImage res(alpha);
	for(y=erodeWin;y<h;++y)
	{
		for(x=erodeWin;x<w;++x)
		{
			flag=1;
			tmp1=y+erodeWin;
			tmp2=x+erodeWin;
			for(i=y-erodeWin;i<=tmp1 && flag;++i)
			{
				for(j=x-erodeWin;j<=tmp2 && flag;++j)
				{
					rgb=alpha.GetPixelColor(j, i);
					if(rgb.rgbBlue==0)
						flag=0;
				}
			}
			rgb.rgbRed=255*flag;
			rgb.rgbGreen=rgb.rgbRed;
			rgb.rgbBlue=rgb.rgbRed;
			for(i=y-erodeWin;i<tmp1;++i)
			{
				for(j=x-erodeWin;j<tmp2;++j)
				{
					res.SetPixelColor(j, i, rgb);
				}
			}
		}
	}

	res.Save(name, CXIMAGE_FORMAT_BMP);
}

typedef bool (*F)(REAL, REAL);
REAL *ErodeAlpha(REAL *alpha, int w, int h, REAL thrAlpha, int erodeWin, F TestFunc);
bool TestFunc1(REAL alpha, REAL thrAlpha);
bool TestFunc2(REAL alpha, REAL thrAlpha);

void Matting::TestErodeAlpha(CxImage &image, char *name)
{
	int i,j,index;
	int w,h;
	int erodeWin=1;
	REAL thrAlpha=0.05;
	REAL *alpha,*r;
	RGBQUAD rgb;

	w=image.GetWidth();
	h=image.GetHeight();
	alpha=new REAL[w*h];
	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{
			index=i*w+j;
			rgb=image.GetPixelColor(j, i);
			alpha[index]=rgb.rgbRed/255.0;
		}
	}
	
	r=ErodeAlpha(alpha, w, h, thrAlpha, erodeWin, TestFunc1);
	TestAlpha(r, w, h, name);

	delete [] alpha;
	delete [] r;
}

//////////////////////////////////////////////////////////////////////////

void Running_Teddy(Matting &matting)
{
     matting.SetImage("Pic/7.bmp", "Pic/7_s.bmp");
     matting.SetLevelNum(2);
     matting.SetActiveLevelNum(2);
}

void Running_Fire(Matting &matting)
{
     matting.SetImage("Pic/6.bmp", "Pic/6_s.bmp");
     matting.SetLevelNum(2);
     matting.SetActiveLevelNum(1);
     matting.SetErodeWinSize(1);
}

void Running_Peacock(Matting &matting)
{
     matting.SetImage("Pic/3.bmp", "Pic/3_s.bmp");
     matting.SetLevelNum(4);
     matting.SetActiveLevelNum(2);
     matting.SetErodeWinSize(1);
     matting.SetThrAlpha(0.025);
}

void Running_Rabbit(Matting &matting)
{
     matting.SetImage("Pic/9.bmp", "Pic/9_s.bmp");
     matting.SetLevelNum(4);
     matting.SetActiveLevelNum(4);
     matting.SetErodeWinSize(1);
     matting.SetThrAlpha(0.12);
     matting.SetEpsilon(0.00001);
}