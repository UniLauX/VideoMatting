#include "../ClosedFormMatting/Matting.h"

using namespace Wml;

void Matting::Conv2(Polynomial1<REAL> &filt, int filtS, CxImage &img)
{
	int i,j;
	int h,w,tmp;
	RGBQUAD rgb;
	Polynomial1<REAL> polyR,polyG,polyB;
	
	w=img.GetWidth();
	h=img.GetHeight();
	for(i=0;i<h;++i)
	{
		polyR.SetDegree(w-1);
		polyG.SetDegree(w-1);
		polyB.SetDegree(w-1);
		for(j=0;j<w;++j)
		{
			rgb=img.GetPixelColor(j, i);
			polyR[j]=rgb.rgbRed;
			polyG[j]=rgb.rgbGreen;
			polyB[j]=rgb.rgbBlue;
		}
		polyR=polyR*filt;
		polyG=polyG*filt;
		polyB=polyB*filt;
		tmp=polyR.GetDegree()-filtS;
		for(j=filtS;j<=tmp;++j)
		{
			rgb.rgbRed=(int)polyR[j];
			rgb.rgbGreen=(int)polyG[j];
			rgb.rgbBlue=(int)polyB[j];
			img.SetPixelColor(j-filtS, i, rgb);
		}
	}
	
	for(i=0;i<w;++i)
	{
		polyR.SetDegree(h-1);
		polyG.SetDegree(h-1);
		polyB.SetDegree(h-1);
		for(j=0;j<h;++j)
		{
			rgb=img.GetPixelColor(i, j);
			polyR[j]=rgb.rgbRed;
			polyG[j]=rgb.rgbGreen;
			polyB[j]=rgb.rgbBlue;
		}
		polyR=polyR*filt;
		polyG=polyG*filt;
		polyB=polyB*filt;
		tmp=polyR.GetDegree()-filtS;
		for(j=filtS;j<=tmp;++j)
		{
			rgb.rgbRed=(int)polyR[j];
			rgb.rgbGreen=(int)polyG[j];
			rgb.rgbBlue=(int)polyB[j];
			img.SetPixelColor(i, j-filtS, rgb);
		}
	}
}


CxImage Matting::DownSmpIm(CxImage &img, int filtS)
{
	int i,j;
	int h,w;
	int h1,w1;
	CxImage image(img);
	RGBQUAD rgb;
	Polynomial1<REAL> filt;

	if(filtS==1)
	{
		filt.SetDegree(2);
		filt[0]=0.25;
		filt[1]=0.5;
		filt[2]=0.25;
	}
	else
	if(filtS==2)
	{
		filt.SetDegree(4);
		filt[0]=1.0/16;
		filt[1]=4.0/16;
		filt[2]=6.0/16;
		filt[3]=4.0/16;
		filt[4]=1.0/16;
	}

	Conv2(filt, filtS, image);

	w=image.GetWidth();
	h=image.GetHeight();
	w1=(w-filtS*2-1)/2+1;
	h1=(h-filtS*2-1)/2+1;

	CxImage result(w1, h1, 24);
	for(i=filtS;i<h-filtS;i+=2)
	{
		for(j=filtS;j<w-filtS;j+=2)
		{
			rgb=image.GetPixelColor(j, i);
			result.SetPixelColor((j-filtS)>>1, (i-filtS)>>1, rgb);
		}
	}
	return result;
}

CxImage Matting::UpSmpIm(CxImage &img, int newX, int newY, int filtS)
{
	int i,j;
	int w,h;
	int id,iu,jd,ju;
	RGBQUAD rgb;
	Polynomial1<REAL> filt;

	if(filtS==1)
	{
		filt.SetDegree(2);
		filt[0]=0.5;
		filt[1]=1;
		filt[2]=0.5;
	}
	else
	if(filtS==2)
	{
		filt.SetDegree(4);
		filt[0]=1.0/8;
		filt[1]=4.0/8;
		filt[2]=6.0/8;
		filt[3]=4.0/8;
		filt[4]=1.0/8;
	}
	
	w=img.GetWidth();
	h=img.GetHeight();
	id=(int)Math<REAL>::Floor((newY-h*2+1)/2.0);
	iu=(int)Math<REAL>::Ceil((newY-h*2+1)/2.0);
	jd=(int)Math<REAL>::Floor((newX-w*2+1)/2.0);
	ju=(int)Math<REAL>::Ceil((newX-w*2+1)/2.0);

	w=newX+2*filtS;
	h=newY+2*filtS;
	CxImage image(w, h, 24);
	for(i=id+filtS;i<h-iu-filtS;i+=2)
	{
		for(j=jd+filtS;j<w-ju-filtS;j+=2)
		{
			rgb=img.GetPixelColor((j-jd-filtS)>>1, (i-id-filtS)>>1);
			image.SetPixelColor(j, i, rgb);
		}
	}
	for(i=id+filtS-2;i>=0;i-=2)
	{
		for(j=0;j<w;++j)
		{
			rgb=image.GetPixelColor(j, id+filtS);
			image.SetPixelColor(j, i, rgb);
		}
	}
	for(i=h-iu-filtS+1;i<h;i+=2)
	{
		for(j=0;j<w;++j)
		{
			rgb=image.GetPixelColor(j, h-iu-filtS-1);
			image.SetPixelColor(j, i, rgb);
		}
	}
	for(j=jd+filtS-2;j>=0;j-=2)
	{
		for(i=0;i<h;++i)
		{
			rgb=image.GetPixelColor(jd+filtS, i);
			image.SetPixelColor(j, i, rgb);
		}
	}
	for(j=w-ju-filtS+1;j<w;j+=2)
	{
		for(i=0;i<h;++i)
		{
			rgb=image.GetPixelColor(w-ju-filtS-1, i);
			image.SetPixelColor(j, i, rgb);
		}
	}
	Conv2(filt, filtS, image);

	CxImage result(newX, newY, 24);
	for(i=filtS;i<h-filtS;++i)
	{
		for(j=filtS;j<w-filtS;++j)
		{
			rgb=image.GetPixelColor(j, i);
			result.SetPixelColor(j-filtS, i-filtS, rgb);
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////


void Matting::Conv2(Polynomial1<REAL> &filt, int filtS, GMatrix<REAL> &mat)
{
	int i,j;
	int w,h;
	int tmp;
	Polynomial1<REAL> poly;

	w=mat.GetColumns();
	h=mat.GetRows();
	for(i=0;i<h;++i)
	{
		poly.SetDegree(w-1);
		for(j=0;j<w;++j)
		{
			poly[j]=mat[i][j];
		}
		poly=poly*filt;
		tmp=poly.GetDegree()-filtS;
		for(j=filtS;j<=tmp;++j)
		{
			mat[i][j-filtS]=poly[j];
		}
	}
	
	for(i=0;i<w;++i)
	{
		poly.SetDegree(h-1);
		for(j=0;j<h;++j)
		{
			poly[j]=mat[j][i];
		}
		poly=poly*filt;
		tmp=poly.GetDegree()-filtS;
		for(j=filtS;j<=tmp;++j)
		{
			mat[j-filtS][i]=poly[j];
		}
	}
}

void Matting::DownSmpIm(Wml::GMatrix<REAL> &mat, int filtS, Wml::GMatrix<REAL> &res)
{
	int i,j;
	int h,w;
	int h1,w1;
	REAL data;
	Polynomial1<REAL> filt;
	GMatrix<REAL> M(mat);
	
	if(filtS==1)
	{
		filt.SetDegree(2);
		filt[0]=0.25;
		filt[1]=0.5;
		filt[2]=0.25;
	}
	else
	if(filtS==2)
	{
		filt.SetDegree(4);
		filt[0]=1.0/16;
		filt[1]=4.0/16;
		filt[2]=6.0/16;
		filt[3]=4.0/16;
		filt[4]=1.0/16;
	}
	
	Conv2(filt, filtS, M);
	
	w=M.GetColumns();
	h=M.GetRows();
	w1=(w-filtS*2-1)/2+1;
	h1=(h-filtS*2-1)/2+1;

	res.SetSize(h1, w1);
	for(i=filtS;i<h-filtS;i+=2)
	{
		for(j=filtS;j<w-filtS;j+=2)
		{
			data=M[i][j];
			res[(i-filtS)>>1][(j-filtS)>>1]=data;
		}
	}
}

void Matting::UpSmpIm(GMatrix<REAL> &mat, int newX, int newY, int filtS, GMatrix<REAL> &res)
{
	int i,j;
	int width,height;
	int w,h,tmp;
	int id,iu,jd,ju;
	REAL data;
	GMatrix<REAL> R;
	Polynomial1<REAL> filt;

	if(filtS==1)
	{
		filt.SetDegree(2);
		filt[0]=0.5;
		filt[1]=1;
		filt[2]=0.5;
	}
	else
	if(filtS==2)
	{
		filt.SetDegree(4);
		filt[0]=1.0/8;
		filt[1]=4.0/8;
		filt[2]=6.0/8;
		filt[3]=4.0/8;
		filt[4]=1.0/8;
	}
	
	width=mat.GetColumns();
	height=mat.GetRows();
	id=(int)Math<REAL>::Floor((newY-height*2+1)/2.0);
	iu=(int)Math<REAL>::Ceil((newY-height*2+1)/2.0);
	jd=(int)Math<REAL>::Floor((newX-width*2+1)/2.0);
	ju=(int)Math<REAL>::Ceil((newX-width*2+1)/2.0);

	w=newX+2*filtS;
	h=newY+2*filtS;
	R.SetSize(h, w);
	for(i=id+filtS;i<h-iu-filtS;i+=2)
	{
		for(j=jd+filtS;j<w-ju-filtS;j+=2)
		{
			data=mat[(i-id-filtS)>>1][(j-jd-filtS)>>1];
			R[i][j]=data;
		}
	}
	for(i=id+filtS-2;i>=0;i-=2)
	{
		tmp=id+filtS;
		for(j=0;j<w;++j)
		{
			data=R[tmp][j];
			R[i][j]=data;
		}
	}
	for(i=h-iu-filtS+1;i<h;i+=2)
	{
		tmp=h-iu-filtS-1;
		for(j=0;j<w;++j)
		{
			data=R[tmp][j];
			R[i][j]=data;
		}
	}
	for(j=jd+filtS-2;j>=0;j-=2)
	{
		tmp=jd+filtS;
		for(i=0;i<h;++i)
		{
			data=R[i][tmp];
			R[i][j]=data;
		}
	}
	for(j=w-ju-filtS+1;j<w;j+=2)
	{
		tmp=w-ju-filtS-1;
		for(i=0;i<h;++i)
		{
			data=R[i][tmp];
			R[i][j]=data;
		}
	}
	Conv2(filt, filtS, R);

	res.SetSize(newY, newX);
	for(i=filtS;i<h-filtS;++i)
	{
		for(j=filtS;j<w-filtS;++j)
		{
			data=R[i][j];
			res[i-filtS][j-filtS]=data;
		}
	}
}