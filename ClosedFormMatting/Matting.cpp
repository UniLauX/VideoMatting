#include "Matting.h"
#include <iostream>

using namespace std;
using namespace Wml;

typedef bool (*F)(REAL, REAL);

void Matting::Init()
{
	level=1;
	activeLevel=1;
	winSize=1;
	lambda=100;
	epsilon=0.0000001;
	thrAlpha=0.02;
	erodeWinSize=1;
}

Matting::Matting()
{
	Init();
}

Matting::~Matting()
{

}

void Matting::SetImage(char *imageName, char *scrbImageName)
{
	if(!image.Load(imageName) || !scrbImage.Load(scrbImageName))
	{
		cout << "Cannot open the image." << endl;
		exit(1);
	}
	SetKnownVals();
}

void Matting::SetImage(CxImage *image, CxImage *scrbImage)
{
	this->image.Copy(*image);
	this->scrbImage.Copy(*scrbImage);
     SetKnownVals();
}

void Matting::SetKnownVals()
{
	const REAL diff=0.001;
	REAL sum;
	int width,height;
	int i,j,index;
	RGBQUAD rgb1,rgb2;

	width=image.GetWidth();
	height=image.GetHeight();
	constMap.SetSize(height, width);
	constVals.SetSize(height, width);

	for(j=0;j<height;++j)
	{
		for(i=0;i<width;++i)
		{
			index=j*width+i;
			rgb1=image.GetPixelColor(i, j);
			rgb2=scrbImage.GetPixelColor(i, j);
			sum=abs(rgb1.rgbRed-rgb2.rgbRed)/255.0;
			sum+=abs(rgb1.rgbGreen-rgb2.rgbGreen)/255.0;
			sum+=abs(rgb1.rgbBlue-rgb2.rgbBlue)/255.0;
		//	if(sum>diff)
		if (rgb2.rgbRed!=128)
			{
				constMap[j][i]=1;
				constVals[j][i]=scrbImage.GetPixelGray(i, j)/255.0;	
			}
			else
			{
				constMap[j][i]=0;
				constVals[j][i]=0;
			}
		}
	}
}

void Matting::SetWinSize(int size)
{
	winSize=size;
}

void Matting::SetLevelNum(int l)
{
	level=l;
}

void Matting::SetActiveLevelNum(int l)
{
	activeLevel=l;
}

void Matting::SetThrAlpha(REAL alpha)
{
	thrAlpha=alpha;
}

void Matting::SetEpsilon(REAL e)
{
	epsilon=e;
}

void Matting::SetErodeWinSize(int size)
{
	erodeWinSize=size;
}

REAL *Matting::SolveAlpha(CxImage &image, GMatrix<REAL> &known, GMatrix<REAL> &knownVals)
{
	int width,height,size;
	REAL *alpha;

	int i,j,index;
	taucs_ccs_matrix L;
	double *bs;
	int flag;
	void *F=NULL;
	char* options[]={"taucs.factor.LLT=true", NULL};
	void* opt_arg[]={NULL};
	
	width=image.GetWidth();
	height=image.GetHeight();
	size=width*height;
	
	alpha=new REAL[size];

	InitLaplacian(image, L);
	GetLaplacian(image, L);	

	bs=new double[size];
	for(i=0;i<height;++i)
	{
		for(j=0;j<width;++j)
		{
			index=i*width+j;
			bs[index]=lambda*known[i][j]*knownVals[i][j];
			if(known[i][j]==1)
				L.values.d[L.colptr[index]]+=lambda;
		}
	}
	
	taucs_logfile("stdout");
	flag=taucs_linsolve(&L, &F, 1, alpha, bs, options, opt_arg);
	
	if (flag!=TAUCS_SUCCESS)
		cout << "Error" << endl;
	
	delete [] bs;
	taucs_linsolve(NULL, &F, 0, NULL, NULL, NULL, NULL);
	DeleteLaplacian(L);

	for(i=0;i<size;++i)
	{
		if(alpha[i]>1)
			alpha[i]=1;
		else
		if(alpha[i]<0)
			alpha[i]=0;
	}
	return alpha;
}

int Round(REAL x)
{
	int a;
	
	a=(int)floor(x);
	if(fabs(a-x)<0.5)
		return a;
	else
		return (int)ceil(x);
}

void Matting::RoundMat(Wml::GMatrix<REAL> &mat)
{
	int i,j;
	int w,h;
	
	w=mat.GetColumns();
	h=mat.GetRows();
	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{
			mat[i][j]=Round(mat[i][j]);
		}
	}
}

REAL *ErodeAlpha(REAL *alpha, int w, int h, REAL thrAlpha, int erodeWin, F TestFunc)
{
	int w1,h1;
	int x,y;
	int i,j,index;
	int tmp1,tmp2;
	int flag;
	REAL *result;

	w1=w-erodeWin;
	h1=h-erodeWin;
	result=new REAL[w*h];
	memset(result, 0, sizeof(REAL)*w*h);
	for(y=erodeWin;y<h1;++y)
	{
		for(x=erodeWin;x<w1;++x)
		{
			flag=1;
			tmp1=y+erodeWin;
			tmp2=x+erodeWin;
			for(i=y-erodeWin;i<=tmp1 && flag;++i)
			{
				for(j=x-erodeWin;j<=tmp2 && flag;++j)
				{
					index=i*w+j;
					if(!TestFunc(alpha[index], thrAlpha))
						flag=0;
				}
			}
			for(i=y-erodeWin;i<=tmp1;++i)
			{
				for(j=x-erodeWin;j<=tmp2;++j)
				{
					index=i*w+j;
					result[index]=flag;
				}
			}
		}
	}

	return result;
}

bool TestFunc1(REAL alpha, REAL thrAlpha)
{
	return alpha>=(1-thrAlpha);
}

bool TestFunc2(REAL alpha, REAL thrAlpha)
{
	return alpha<=thrAlpha;
}

void Matting::UpDateConstMap(GMatrix<REAL> &constMap, 
							 REAL *alpha, REAL thrAlpha, int erodeWin)
{
	int i,j,index;
	int width,height;
	REAL *r1,*r2;

	width=constMap.GetColumns();
	height=constMap.GetRows();
	r1=ErodeAlpha(alpha, width, height, thrAlpha, erodeWin, TestFunc1);
	r2=ErodeAlpha(alpha, width, height, thrAlpha, erodeWin, TestFunc2);
	for(i=0;i<height;++i)
	{
		for(j=0;j<width;++j)
		{
			index=i*width+j;
			constMap[i][j]=min(constMap[i][j]+r1[index]+r2[index], 1);
		}
	}
	delete [] r1;
	delete [] r2;
}

REAL *Matting::SolveAlphaC2F(CxImage &image, 
							 Wml::GMatrix<REAL> &constMap, 
							 Wml::GMatrix<REAL> &constVals, 
							 int level,
							 int activeLevelNum)
{
	static int filtS=2;
	int w,h;
	int i,j,index;
	REAL *alpha=NULL;
	REAL *sAlpha=NULL;
	REAL *tAlpha;
	CxImage sI;
	GMatrix<REAL> sConstMap;
	GMatrix<REAL> sConstVals;
	
	w=image.GetWidth();
	h=image.GetHeight();
	activeLevelNum=max(activeLevelNum, 1);
	tAlpha=new REAL[w*h];
	memset(tAlpha, 0, sizeof(REAL)*w*h);

	if(level>1)
	{
		sI=DownSmpIm(image, filtS);
		DownSmpIm(constMap, filtS, sConstMap);
		DownSmpIm(constVals, filtS, sConstVals);
		RoundMat(sConstMap);
		RoundMat(sConstVals);

/*		TestMatValues(constMap, "ConstMap.bmp");
		TestMatValues(constVals, "ConstVals.bmp");
		TestMatValues(sConstMap, "sConstMap.bmp");
		TestMatValues(sConstVals, "sConstVals.bmp");
		sI.Save("sI.bmp", CXIMAGE_FORMAT_BMP);
		cout << "sI" << endl;
		getchar();
*/
		sAlpha=SolveAlphaC2F(sI, sConstMap, sConstVals, 
							 level-1, min(level-1, activeLevelNum));
		
//		sI.Save("sI.bmp", CXIMAGE_FORMAT_BMP);
//		TestAlpha(sAlpha, sI.GetWidth(), sI.GetHeight(), "alpha_before.bmp");

		alpha=UpSampleAlphaUsingImg(sAlpha, sI, image, filtS);

//		TestAlpha(alpha, w, h, "alpha_after.bmp");
//		cout << "Level: " << level << endl;
//		cout << "Press" << endl;
//		getchar();

		for(i=0;i<h;++i)
		{
			for(j=0;j<w;++j)
			{
				index=i*w+j;
				tAlpha[index]=alpha[index]*(1-constMap[i][j])+constVals[i][j];
			}
		}
		UpDateConstMap(constMap, alpha, thrAlpha, erodeWinSize);

		for(i=0;i<h;++i)
		{
			for(j=0;j<w;++j)
			{
				index=i*w+j;
				constVals[i][j]=Round(tAlpha[index])*constMap[i][j];
			}
		}
	}
	if(activeLevelNum>=level)
	{
		delete [] alpha;
		alpha=SolveAlpha(image, constMap, constVals);
	}
	delete [] sAlpha;
	delete [] tAlpha;
	return alpha;
}

REAL *Matting::DoMatting()
{
	REAL *alpha;
	alpha=SolveAlphaC2F(image, constMap, constVals, level, activeLevel);
	return alpha;
}

void Matting::GetResult(CxImage &image)
{
     int i,j;
     int index;
     int w,h;
     REAL tmp;	
     RGBQUAD rgb;

     REAL *alpha=DoMatting();
     h=this->image.GetHeight();
     w=this->image.GetWidth();
     image.Copy(this->image);
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

     delete [] alpha;
}
int Matting::GetWinSize()
{
	return erodeWinSize;
}

int Matting::GetLevelNum()
{
	return level;
}

int Matting::GetActiveLevelNum()
{
	return activeLevel;
}

double Matting::GetThrAlpha()
{
	return thrAlpha;
}

double Matting::GetEpsilon()
{

	return epsilon;
}