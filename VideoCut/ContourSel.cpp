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

void Contour::GetSelectWin(VPOINTS &selWins, CxImage &scrib)
{
     int i,j,k;
     int x,y;
     int s1,s2;
     int cx,cy;
     int x1,y1;
     int x2,y2;
     POINT index;
     BYTE rgb;

     k=localWinSize >> 1;
     s1=(int)winCenters.size();
     for(i=0;i<s1;++i)
     {
          s2=(int)winCenters[i].size();
          for(j=0;j<s2;++j)
          {
               cx=winCenters[i][j].x;
               cy=winCenters[i][j].y;
               x1=cx-k;  x1=x1<0 ? 0:x1;
               y1=cy-k;  y1=y1<0 ? 0:y1;
               x2=cx+k;  x2=x2>width ? width:x2;
               y2=cy+k;  y2=y2>height ? height:y2;
               for(y=y1;y<y2;++y)
               {
                    for(x=x1;x<x2;++x)
                    {
                         rgb=scrib.GetPixelGray(x, y);
                         if(rgb!=128)
                         {
                              index.x=i;
                              index.y=j;
                              selWins.push_back(index);
                              break;
                         }
                    }
                    if(x!=x2)
                         break;
               }
          }
     }
}

// Before call this, we should get local windows and update the label.
void Contour::UpdateLocalClassifier(VPOINTS &selWins, const TRANS_CLASSIFIER_PARAM &param)
{
     int i,j,size;
     int pos(0);
     POINT point;
     int index;
     //double *F,*B,*P;
     std::map<std::pair<int, int>, double> F, B, P;
     vector<LocalClassifier *>::iterator iter;

     size=(int)selWins.size();
     iter=localWins.begin();
     boundPts.clear();
     Pf.clear();
     Pb.clear();
	 if (boundMask==NULL)
		 boundMask = new char[pixelNum];
	 
     memset(boundMask, 0, sizeof(char)*pixelNum);
     //F=new double[pixelNum];
     //B=new double[pixelNum];
     //P=new double[pixelNum];
     //memset(F, 0, sizeof(double)*pixelNum);
     //memset(B, 0, sizeof(double)*pixelNum);
     //memset(P, 0, sizeof(double)*pixelNum);

     for(i=0;i<size;++i)
     {
          point=selWins[i];
          while(pos<point.x)
          {
               ++pos;
               ++iter;
          }
          (*iter)[point.y].InitModel(param.mode);
          (*iter)[point.y].AddProbValues(F, B, P);
     }

     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               index=i*width+j;
               //if(F[index]!=0 || B[index]!=0)
	       if(F[make_pair(j,i)]!=0||B[make_pair(j,i)]!=0)
               {
				   point.x=j;
				   point.y=i;
				   boundPts.push_back(point);
				   //Pf.push_back(F[index]/P[index]);
				   //Pb.push_back(B[index]/P[index]);
				   Pf.push_back(F[make_pair(j,i)]/P[make_pair(j,i)]);
				   Pb.push_back(B[make_pair(j,i)]/P[make_pair(j,i)]);
				   boundMask[index]=1;
               }
          }
     }
     //delete [] F;
     //delete [] B;
     //delete [] P;
}
// result: alpha values
void Contour::InitCurrentFrame(CxImage &result, CxImage &scrib, 
                               TRANS_CLASSIFIER_PARAM &param)
{
     int i,j;
     int index;
     RGBQUAD rgb;
     VPOINTS selWins;
     memset(label, 0, sizeof(int)*pixelNum);
     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               index=i*width+j;
               rgb=result.GetPixelColor(j, i);
               label[index]=(rgb.rgbRed==255?1:0);
          }
     }
     //
     GetLocalClassifier(param);
     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               index=i*width+j;
               rgb=scrib.GetPixelColor(j, i);
               if(rgb.rgbRed==0)
                    label[index]=0;
               else
               if(rgb.rgbRed==255)
                    label[index]=1;
          }
     }
     GetSelectWin(selWins, scrib);
     UpdateLocalClassifier(selWins, param);
}

//void Contour::UpdateLocal(CxImage &result, CxImage &scrib, 
//                          TRANS_CLASSIFIER_PARAM &param, unsigned char *label)
//{
//     InitCurrentFrame(result, scrib, param);
//     DoCut(label);
//}

/************************************************************************/
/* Use them for local grabcut                                           */
/************************************************************************/
int Contour::GetLocalPixels(CxImage &alpha, CxImage &scrib, VPOINTS &pixels, CxImage& localTrimap)
{
     int i,j,k,s,size;
     int x1,y1,x2,y2;
     POINT point;
     RGBQUAD rgb;
     double *F;
     VPOINTS selWins;

     localTrimap.Create(scrib.GetWidth(), scrib.GetHeight(), 8);
     localTrimap.SetGrayPalette();
     localTrimap.Clear();
     memset(label, 0, sizeof(unsigned char)*pixelNum);
     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               rgb=alpha.GetPixelColor(j, i);
               label[i*width+j]=(rgb.rgbRed==255?1:0);
          }
     }
     winCenters.clear();
     GetAllWinowCenters(winCenters);
     BYTE rgbScrib;
     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               rgbScrib=scrib.GetPixelGray(j, i);
               if(rgbScrib==0)
                    label[i*width+j]=0;
               else
               if(rgbScrib==255)
                    label[i*width+j]=1;
          }
     }

     CxImage temptest;
     temptest.Copy(*image);
     for (int i=0; i<winCenters.size(); ++i)
     {
	     for (int j=0; j<winCenters[i].size(); ++j)
	     {
		     int x = winCenters[i][j].x;
		     int y = winCenters[i][j].y;
		     temptest.SetPixelColor(x, y, RGB(255,255,255));
	     }
     }
     temptest.Save("E:/temptest.jpg", CXIMAGE_FORMAT_JPG);

     GetSelectWin(selWins, scrib);
     pixels.clear();
     F=new double[pixelNum];
     memset(F, 0, sizeof(double)*pixelNum);
     s=localWinSize>>1;
     size=(int)selWins.size();
     for(i=0;i<size;++i)
     {
          point=selWins[i];
          point=winCenters[point.x][point.y];
          x1=point.x-s;  x1=x1<0 ? 0:x1;
          y1=point.y-s;  y1=y1<0 ? 0:y1;
          x2=point.x+s;  x2=x2>width ? width:x2;
          y2=point.y+s;  y2=y2>height ? height:y2;
          for(j=y1;j<y2;++j)
          {
               for(k=x1;k<x2;++k)
                    F[j*width+k]=1;
          }
     }

     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               if(F[i*width+j]!=0)
               {
                    point.x=j;
                    point.y=i;
                    pixels.push_back(point);
		    localTrimap.SetPixelIndex(j,i,128);
               }
          }
     }
     delete [] F;
     if(pixels.size()<50)
          return -1;
     else
          return 0;
}

int Contour::GetLocalPixels(unsigned char* _label, CxImage& scrib, VPOINTS& pixels, CxImage& localTrimap)
{
	double st,e,d;
	st = clock();
	int i,j,k,s,size;
	int x1,y1,x2,y2;
	POINT point;
	RGBQUAD rgb;
	double *F;
	VPOINTS selWins;
	localTrimap.Create(scrib.GetWidth(), scrib.GetHeight(), 8);
	localTrimap.SetGrayPalette();
	localTrimap.Clear();
	memcpy(label, _label, sizeof(unsigned char)*pixelNum);
	e=clock();
	d = (e-st)/CLK_TCK;
	std::cout<<"pretime: "<<d<<std::endl;

	winCenters.clear();
	double startt, endt, dt;
	startt = clock();
	GetAllWinowCenters(winCenters);
	endt = clock();
	dt = (endt - startt)/CLK_TCK;
	std::cout<<" time: "<<dt<<std::endl;

	startt = clock();
	BYTE rgbScrib;
	for(i=0;i<height;++i)
	{
		for(j=0;j<width;++j)
		{
			rgbScrib=scrib.GetPixelGray(j, i);
			if(rgbScrib==0)
				label[i*width+j]=0;
			else
				if(rgbScrib==255)
					label[i*width+j]=1;
		}
	}
	endt = clock();
	dt = (endt - startt)/CLK_TCK;
	std::cout<<"time2: "<<dt<<std::endl;
	GetSelectWin(selWins, scrib);
	pixels.clear();
	startt = clock();
	F=new double[pixelNum];
	memset(F, 0, sizeof(double)*pixelNum);
	s=localWinSize>>1;
	size=(int)selWins.size();
	for(i=0;i<size;++i)
	{
		point=selWins[i];
		point=winCenters[point.x][point.y];
		x1=point.x-s;  x1=x1<0 ? 0:x1;
		y1=point.y-s;  y1=y1<0 ? 0:y1;
		x2=point.x+s;  x2=x2>width ? width:x2;
		y2=point.y+s;  y2=y2>height ? height:y2;
		for(j=y1;j<y2;++j)
		{
			for(k=x1;k<x2;++k)
				F[j*width+k]=1;
		}
	}
	for(i=0;i<height;++i)
	{
		for(j=0;j<width;++j)
		{
			if(F[i*width+j]!=0)
			{
				point.x=j;
				point.y=i;
				pixels.push_back(point);
				localTrimap.SetPixelIndex(j,i,128);
			}
		}
	}
	e = clock();
	d = (e-st)/CLK_TCK;
	std::cout<<"d time:"<<d<<std::endl;
	delete [] F;
	if(pixels.size()<50)
		return -1;
	else
		return 0;
}

int Contour::GetLocalPixels(unsigned char* _label, UserScribble& scrib, VPOINTS& pixels, CxImage& localTrimap)
{
	int i,j,k,s,size;
	int x1,y1,x2,y2;
	POINT point;
	RGBQUAD rgb;
	double *F;
	VPOINTS selWins;
	localTrimap.Create(this->image->GetWidth(), this->image->GetHeight(), 8);
	localTrimap.SetGrayPalette();
	localTrimap.Clear();
	//memcpy(label, _label, sizeof(unsigned char)*pixelNum);
	winCenters.clear();
	CxImage scribim;
	scribim.Create(this->image->GetWidth(), this->image->GetHeight(),8);
	scribim.SetGrayPalette();
	scribim.Clear(128);
	for (int i=0; i<scrib.forgptsize(); ++i)
	{
		int x = scrib.ForgPt(i).x;
		int y = scrib.ForgPt(i).y;
		scribim.SetPixelIndex(x, y, 255);
		label[y*width+x] = 1;
	}
	for (int i=0; i<scrib.backgptsize(); ++i)
	{
		int x = scrib.BackgPt(i).x;
		int y = scrib.BackgPt(i).y;
		scribim.SetPixelIndex(x, y, 0);
		label[y*width+x] = 0;
	}

	GetAllWinowCenters(winCenters);

	GetSelectWin(selWins, scribim);
	pixels.clear();
	F=new double[pixelNum];
	memset(F, 0, sizeof(double)*pixelNum);
	s=localWinSize>>1;
	size=(int)selWins.size();
	for(i=0;i<size;++i)
	{
		point=selWins[i];
		point=winCenters[point.x][point.y];
		x1=point.x-s;  x1=x1<0 ? 0:x1;
		y1=point.y-s;  y1=y1<0 ? 0:y1;
		x2=point.x+s;  x2=x2>width ? width:x2;
		y2=point.y+s;  y2=y2>height ? height:y2;
		for(j=y1;j<y2;++j)
		{
			for(k=x1;k<x2;++k)
				F[j*width+k]=1;
		}
	}
	for(i=0;i<height;++i)
	{
		for(j=0;j<width;++j)
		{
			if(F[i*width+j]!=0)
			{
				point.x=j;
				point.y=i;
				pixels.push_back(point);
				localTrimap.SetPixelIndex(j,i,128);
			}
		}
	}
	delete [] F;
	if(pixels.size()<50)
		return -1;
	else
		return 0;
}

void Contour::smoothLabel(unsigned char *label,unsigned char* newlabel,int width,int height)
{
	int i,j;
	int index=0;
	int count;
	int xtemp;
	int ytemp;
	//memcpy(newlabel,label,sizeof(unsigned char)*width*height);
	for (i=0;i<width;++i)
		for (j=0;j<height;++j)
		{
			count=0;
			index=j*width+i;
			if (label[index])
			{
				for (xtemp=i-3;xtemp<i+3;++xtemp)
				{
					for (ytemp=j-3;ytemp<j+3;++ytemp)
					{
						index=ytemp*width+xtemp;
						if (index>=0&&index<width*height)
						{
							if (label[index])
								count++;							
						}
					}
				}
				if (count>30)
					for (xtemp=i-3;xtemp<i+3;++xtemp)
						for (ytemp=j-3;ytemp<j+3;++ytemp)
						{
							index=ytemp*width+xtemp;
							if (index>=0&&index<width*height)
									newlabel[index]=1;					

						}
				}
		}

}

void Contour::TestSelPixels(VPOINTS &pixels)
{
     int i;
     int size;
     POINT point;
     RGBQUAD rgb;
     CxImage pic;

     pic.Copy(*image);
     size=(int)pixels.size();
     for(i=0;i<size;++i)
     {
          point=pixels[i];
          rgb.rgbRed=255;
          rgb.rgbGreen=255;
          rgb.rgbBlue=255;
          pic.SetPixelColor(point.x, point.y, rgb);
     }
     pic.Save("D:\\Result\\Test\\Sel.jpg", CXIMAGE_FORMAT_JPG);
}
