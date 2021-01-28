#include "../include/Wml/WmlLinearSystem.h"
#include "../ClosedFormMatting/Matting.h"

using namespace Wml;

void Matting::GetLinearCoeff(REAL *alpha, 
					    CxImage &image, 
					    GMatrix<REAL> &A_R,
					    GMatrix<REAL> &A_G,
					    GMatrix<REAL> &A_B,
					    GMatrix<REAL> &B)
{
	int i,j;
	int x,y,index;
	int w,h;
	int width,height;
	int tmp1,tmp2;
	int nebSize;
	REAL tmp;
	REAL X[4];
	RGBQUAD rgb;
	GMatrix<REAL> G;
	GMatrix<REAL> MatA,MatB;
	GMatrix<REAL> winColor;

	nebSize=(winSize<<1)+1;
	nebSize*=nebSize;
	tmp=sqrt(epsilon);
	width=image.GetWidth();
	height=image.GetHeight();
	w=width-winSize;
	h=height-winSize;
	winColor.SetSize(nebSize, 3);
	A_R.SetSize(height, width);
	A_G.SetSize(height, width);
	A_B.SetSize(height, width);
	B.SetSize(height, width);

	for(y=winSize;y<h;++y)
	{
		for(x=winSize;x<w;++x)
		{
			G.SetSize(nebSize+3, 4);
			MatB.SetSize(nebSize+3, 1);

			index=0;
			tmp1=y+winSize;
			tmp2=x+winSize;
			for(i=y-winSize;i<=tmp1;++i)
			{
				for(j=x-winSize;j<=tmp2;++j)
				{
					rgb=image.GetPixelColor(j, i);
					winColor[index][0]=rgb.rgbRed/255.0;
					winColor[index][1]=rgb.rgbGreen/255.0;
					winColor[index][2]=rgb.rgbBlue/255.0;
					MatB[index][0]=alpha[i*width+j];
					++index;
				}
			}
			for(i=0;i<nebSize;++i)
			{
				G[i][3]=1;
				for(j=0;j<3;++j)
					G[i][j]=winColor[i][j];
			}
			for(i=0;i<3;++i)
			{
				G[i+nebSize][i]=tmp;
				MatB[i+nebSize][0]=0;
			}

			MatA=G.TransposeTimes(G);
			MatB=G.TransposeTimes(MatB);			
			LinearSystem<REAL>::Solve(MatA, (REAL *)(MatB), X);
			A_R[y][x]=X[0];
			A_G[y][x]=X[1];
			A_B[y][x]=X[2];
			B[y][x]=X[3];
		}
	}
	
	for(i=0;i<winSize;++i)
	{
		for(j=0;j<w;++j)
		{
			A_R[i][j]=A_R[winSize][j];
			A_G[i][j]=A_G[winSize][j];
			A_B[i][j]=A_B[winSize][j];
			B[i][j]=B[winSize][j];
		}
	}
	for(i=h;i<height;++i)
	{
		tmp1=h-1;
		for(j=0;j<width;++j)
		{
			A_R[i][j]=A_R[tmp1][j];
			A_G[i][j]=A_G[tmp1][j];
			A_B[i][j]=A_B[tmp1][j];
			B[i][j]=B[tmp1][j];
		}
	}
	for(j=0;j<winSize;++j)
	{
		for(i=0;i<height;++i)
		{
			A_R[i][j]=A_R[i][winSize];
			A_G[i][j]=A_G[i][winSize];
			A_B[i][j]=A_B[i][winSize];
			B[i][j]=B[i][winSize];
		}
	}
	for(j=w;j<width;++j)
	{
		tmp1=w-1;
		for(i=0;i<height;++i)
		{
			A_R[i][j]=A_R[i][tmp1];
			A_G[i][j]=A_G[i][tmp1];
			A_B[i][j]=A_B[i][tmp1];
			B[i][j]=B[i][tmp1];
		}
	}
}

REAL *Matting::UpSampleAlphaUsingImg(REAL *alpha, CxImage &I, CxImage &bI, int filtS)
{
	int i,j,index;
	int w,h;
	REAL *bAlpha;
	RGBQUAD rgb;
	GMatrix<REAL> A_R;
	GMatrix<REAL> A_G;
	GMatrix<REAL> A_B;
	GMatrix<REAL> B;
	GMatrix<REAL> r_A_R;
	GMatrix<REAL> r_A_G;
	GMatrix<REAL> r_A_B;
	GMatrix<REAL> r_B;
	
	w=bI.GetWidth();
	h=bI.GetHeight();
	bAlpha=new REAL[w*h];
	memset(bAlpha, 0, sizeof(REAL)*w*h);
	GetLinearCoeff(alpha, I, A_R, A_G, A_B, B);
/*	//
	TestAlpha(alpha, I.GetWidth(), I.GetHeight(), "Test_alpha.bmp");
	I.Save("I.bmp", CXIMAGE_FORMAT_BMP);
	TestMatValues(B, "Coeff_B.bmp");
	TestMatValues(A_R, "Coeff_A_R.bmp");
	TestMatValues(A_G, "Coeff_A_G.bmp");
	TestMatValues(A_B, "Coeff_A_B.bmp");
	getchar();
*/	//
	UpSmpIm(A_R, w, h, filtS, r_A_R);
	UpSmpIm(A_G, w, h, filtS, r_A_G);
	UpSmpIm(A_B, w, h, filtS, r_A_B);
	UpSmpIm(B, w, h, filtS, r_B);

	for(i=0;i<h;++i)
	{
		for(j=0;j<w;++j)
		{
			index=i*w+j;
			rgb=bI.GetPixelColor(j, i);
			bAlpha[index]=r_B[i][j]+
						  r_A_R[i][j]*rgb.rgbRed/255.0+
						  r_A_G[i][j]*rgb.rgbGreen/255.0+
						  r_A_B[i][j]*rgb.rgbBlue/255.0;

			if(bAlpha[index]>1)
				bAlpha[index]=1;
			else
			if(bAlpha[index]<0)
				bAlpha[index]=0;
		}
	}

	return bAlpha;
}