#include "../ClosedFormMatting/Matting.h"

using namespace Wml;

#define ZOOM 1000

void Matting::InitLaplacian(CxImage &image, taucs_ccs_matrix &L)
{
	int width,height,size;
	int winWidth;
	int left,right,up,a;
	int winPixelNum;
	int i,j,index,allPixelNum;

	width=image.GetWidth();
	height=image.GetHeight();
	size=width*height;
	allPixelNum=0;
	winWidth=(winSize<<1)+1;
	L.m=size;
	L.n=size;
	L.flags=(TAUCS_DOUBLE | TAUCS_SYMMETRIC | TAUCS_LOWER);
	L.colptr=new int[size+1];
	memset(L.colptr, 0, sizeof(int)*(size+1));
	for(i=0;i<height;++i)
	{
		for(j=0;j<width;++j)
		{
			index=i*width+j;
			left=max(0, j-winWidth+1);
			right=min(width-1, j+winWidth-1);
			up=min(height-1, i+winWidth-1);
			a=right-left+1;
			winPixelNum=a*(up-i)+right-j+1;
			allPixelNum+=winPixelNum;
			L.colptr[index+1]=L.colptr[index]+winPixelNum;
		}
	}
	L.rowind=new int[allPixelNum];
	L.values.d=new taucs_double[allPixelNum];
	memset(L.values.d, 0, sizeof(taucs_double)*(allPixelNum));
}

void Matting::DeleteLaplacian(taucs_ccs_matrix &L)
{
	delete [] L.colptr;
	delete [] L.rowind;
	delete [] L.values.d;
}

void Matting::GetLaplacian(CxImage &image, taucs_ccs_matrix &L)
{
	int width,height;
	int nebSize;
	int winWidth;
	int regionWidth;
	int x,y,index;
	int x1,y1,x2,y2;
	int i,j,tmp1,tmp2;
	int w,h;
	REAL tmp;
	RGBQUAD rgb;
	Vector3<REAL> mean;
	GMatrix<REAL> winColor;
	GMatrix<REAL> vals;
	GMatrix<REAL> TmpMat;
	Matrix3<REAL> invCov;

	width=image.GetWidth();
	height=image.GetHeight();
	winWidth=(winSize<<1)+1;
	nebSize=winWidth*winWidth;
	regionWidth=(winSize<<2)+1;
	w=width-winSize;
	h=height-winSize;
	tmp=1.0/nebSize;
	winColor.SetSize(nebSize, 3);
	vals.SetSize(nebSize, nebSize);

	for(y=winSize;y<h;++y)
	{
		for(x=winSize;x<w;++x)
		{
			index=0;
			mean[0]=mean[1]=mean[2]=0;
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
					mean[0]+=winColor[index][0];
					mean[1]+=winColor[index][1];
					mean[2]+=winColor[index][2];
					++index;
				}
			}
			mean[0]*=tmp;
			mean[1]*=tmp;
			mean[2]*=tmp;

			invCov.MakeTensorProduct(mean, mean);
			TmpMat=winColor.TransposeTimes(winColor);
			for(i=0;i<3;++i)
			{
				for(j=0;j<3;++j)
					invCov[i][j]=(TmpMat[i][j]*tmp-invCov[i][j])*ZOOM;

				invCov[i][i]+=epsilon*tmp*ZOOM;
			}
			invCov=invCov.Inverse();
			for(i=0;i<3;++i)
			{
				for(j=0;j<3;++j)
					TmpMat[i][j]=invCov[i][j]*ZOOM;
			}
		
			for(i=0;i<nebSize;++i)
			{
				for(j=0;j<3;++j)
					winColor[i][j]-=mean[j];
			}
//			vals=(winColor*TmpMat).TimesTranspose(winColor);        //ÓÐÎÊÌâ
			vals=winColor*TmpMat*(winColor.Transpose());
			for(i=0;i<nebSize;++i)
			{
				for(j=0;j<nebSize;++j)
					vals[i][j]=(1+vals[i][j])*tmp;
			}
			for(i=0;i<nebSize;++i)
			{
				x1=x+(i%winWidth)-winSize;
				y1=y+i/winWidth-winSize;
				int left=max(0, x1-winWidth+1);
				int right=min(width-1, x1+winWidth-1);
				int up=min(height-1, y1+winWidth-1);
				int a=right-left+1;

				for(j=i;j<nebSize;++j)
				{
					x2=x+(j%winWidth)-winSize;
					y2=y+j/winWidth-winSize;
					index=a*(y2-y1)+x2-x1;
					index+=L.colptr[y1*width+x1];
					L.rowind[index]=y2*width+x2;
					if(i==j)
						L.values.d[index]+=1-vals[i][j];
					else
						L.values.d[index]+=-vals[i][j];
				}
			}
		}
	}
}