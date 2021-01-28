#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <omp.h>
#include "Contour.h"
#include <math.h>


using namespace std;

static inline bool IsInWindow(const POINT &point, const POINT &center, int size)
{
     return ((point.x<center.x+size/2) && (point.x>center.x-size/2) &&
          (point.y<center.y+size/2) && (point.y>center.y-size/2));	
}

static inline double GetDistance(const POINT &p1, const POINT &p2)
{
     return sqrt((double)(p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
}

void Contour::GetBound(int * flag,VPOINTS& points)      //除过边界，若元素为1而四周存在0，则该元素为边界元素
{
	points.clear();   //清空元素

	int i,j,k;
	int x,y;
	int index1,index2;
	POINT point;
	int direct[4][2]={{1,0},{0,1},{-1,0},{0,-1}};
	for(j=0;j<height;++j)
	{
		for(i=0;i<width;++i)
		{
			index1=j*width+i;
			if(label[index1])
			{
				for(k=0;k<4;++k)
				{
					x=i+direct[k][0];
					y=j+direct[k][1];
					index2=y*width+x;
					if(x>=0&&x<width&&y>=0&&y<height)             //非边缘点
					{
						if(label[index2]!=1)
						{
							point.x=i;
							point.y=j;
							points.push_back(point);
							flag[index1]=1;
							break;                      
						}
					}
					else                                           //边缘点
					{
						point.x=i;
						point.y=j;
						points.push_back(point);
						flag[index1]=1;
					}

				}
			}
		}
	}


}

int Contour::GetNeighbourValue(int x, int y, int dir, 
                               int &xtemp, int &ytemp, unsigned char **alpha)
{
     static int direction[8][2]={{1,0},{1,1},{0,1},{-1,1},
     {-1,0},{-1,-1},{0,-1},{1,-1}};
	//static int direction[8][2] = {{-1,1}, {0,1}, {1,1},{1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}};

     if (!(x>=0 && x<width && y>=0 && y<height))
          return 0;

     xtemp=x+direction[dir][0];
     ytemp=y+direction[dir][1];
     if (xtemp>=0&&xtemp<width&&ytemp>=0&&ytemp<height)
     {
	     if (alpha[xtemp][ytemp]==1)
		     return 1;
	     else
		     return 0;
     }
     else
	     return 0;
    
}

void Contour::GetAllWinowCenters(vector<VPOINTS> &vec)
{   
	VPOINTS  tmpVec;
	tmpVec.clear();
	// 1. generate boundary piexels    //缺乏对于图像中行元素非4倍数的处理
	//boundPts.clear();
	 vector<POINT> boundPoints;
	vec.clear();
	int *flag;
	flag=new int[pixelNum];
	memset(flag,0,sizeof(int)*pixelNum);
	GetBound(flag,boundPoints);

	/*
	//test code for display generated contour
	if(true)
	{
		CxImage contourImg;
		contourImg.Copy(*image);
		RGBQUAD tRgb;
		tRgb.rgbRed=255;
		tRgb.rgbBlue=0;
		tRgb.rgbGreen=0;
		int bndPtSize=boundPts.size();
		cout<<"bndPtSize= "<<bndPtSize<<endl;
		for(int i=0;i<bndPtSize;++i)
		{
			int x=boundPts[i].x;
			int y=boundPts[i].y;
			contourImg.SetPixelColor(x,y,tRgb);

		}
		contourImg.Save("D://contourImg.jpg",CXIMAGE_FORMAT_JPG);
	}
   */

	//2. select windows sample from boundary pixels
	/*
	CxImage newConImg;
	newConImg.Copy(*image);
	RGBQUAD nRgb;
	nRgb.rgbRed=0;
	nRgb.rgbBlue=255;
	nRgb.rgbGreen=0;
*/
	int winSize=localWinSize;
	float centerDisThres=localWinSize*(7.0/12);

	if(boundPoints.size()==0)                              //当跟踪不到特征点时提示并退出程序
	{
		cout<<"no boundry pixels"<<endl;
    
		MessageBox(NULL,"No Boundry Pixels!","提示",0);
		tmpVec.clear();
		//return false;
	    // return ;
		exit(-1);
		 
	}
/*

*/
	vector<POINT>::iterator ibp;
	vector<POINT> orderBdyPts;
//	int count=0;

	for(ibp=boundPoints.begin();ibp!=boundPoints.end();++ibp)
	{
		int x=(*ibp).x;
		int y=(*ibp).y;
		int index=y*width+x;
		if(flag[index]==1)
		{
			orderBdyPts.clear();
			static int cnt=0;
			GetConnectedBound(*ibp,flag,orderBdyPts);
			cnt++;
		/*
			//test code for newConImg
			if(true)
			{
				int ordBdySize=orderBdyPts.size();
				//	cout<<ordBdySize<<endl;
				for(int i=0;i<ordBdySize;++i)
				{
					int x=orderBdyPts[i].x;
					int y=orderBdyPts[i].y;
					newConImg.SetPixelColor(x,y,nRgb);
				}
				newConImg.Save("D://newConImg.jpg",CXIMAGE_FORMAT_JPG);
			}

			//以上6行为测试代码
        */


			//new start code
			tmpVec.push_back(orderBdyPts[0]);
			
			POINT nextPnt;
		    VPOINTS selectedWinPts;
			selectedWinPts.push_back(orderBdyPts[0]);
			int curX = orderBdyPts[0].x;
			int curY = orderBdyPts[0].y;
			int size = orderBdyPts.size();
			for (int i = 1; i < size; ++i)
			{
				nextPnt.x = orderBdyPts[i].x;
				nextPnt.y = orderBdyPts[i].y;
				//cout<<"min distance "<<GetWinMinDis(orderBdyPts[i], selectedWinPts)<<endl;
				//cout<<"centerDisThres "<<centerDisThres<<endl;
				if (GetWinMinDis(orderBdyPts[i], selectedWinPts) >= centerDisThres)      
				{
					tmpVec.push_back(nextPnt);
					selectedWinPts.push_back(orderBdyPts[i]);
					curX = nextPnt.x;
					curY = nextPnt.y;
				}

			}
			selectedWinPts.clear();
			

		}

	}

	//cout<<"local win Num= "<<tmpVec.size()<<endl;
	vec.push_back(tmpVec);

	if(tmpVec.size()<4)
	{
		cout<<"elm num is smaller than 4"<<endl;
		return;
	}
	
    tmpVec.clear();
	
	 delete [] flag;
	//
  /*
	//test code for display adjusted wins 
	if(true){
		CxImage selConImg;
		selConImg.Copy(*image);
		int size =tmpVec.size();
		//win boundary
		int *cntImg;
		int cntIdx;
		cntImg=new int[pixelNum];
		memset(cntImg,0,sizeof(int)*pixelNum);
		int maxCnt = 0;
		//cntImg.CreateAndInit(m_width, m_height, 1, 0);  //cntImg未进行初始化


		for (int i = 0; i < size; ++i)
		{
			int centerX = tmpVec[i].x;
			int centerY = tmpVec[i].y;
			int winSize = localWinSize;
			for (int winX = 0; winX < winSize; ++winX)
			{
				for (int winY = 0; winY < winSize; ++winY)
				{
					int gX = winX+centerX-winSize/2;
					int gY = winY+centerY-winSize/2;
					cntIdx=gY*width+gX;
					if (gX >= 0 && gX < width && gY >= 0 && gY < height)
					{
						cntImg[cntIdx]++;
						maxCnt = __max(cntImg[cntIdx], maxCnt);
					}

				}
			}
		}

		int scalIdx=0;
		RGBQUAD sRgb;
		sRgb.rgbRed=255;
		sRgb.rgbBlue=0;
		sRgb.rgbGreen=0;
		float scale = 1.0/maxCnt;
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				scalIdx=y*width+x;
				if (cntImg[scalIdx] != 0)
				{

					sRgb.rgbRed=(int)scale*cntImg[cntIdx]*255;
					sRgb.rgbBlue=(int)scale*cntImg[cntIdx]*255;
					sRgb.rgbGreen=(int)scale*cntImg[cntIdx]*255;
					selConImg.SetPixelColor(x,y, sRgb);
				}
			}
		}


		selConImg.Save("D://selConImg.jpg",CXIMAGE_FORMAT_JPG);
	}
   */
}

void Contour::GetConnectedBound(const POINT& sPoint,int * flagImg,VPOINTS& points)
{
	static int direction[8][2]={{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
	points.clear();
	queue<POINT> pointQueue;
	

	POINT  curPnt,newPnt;
	pointQueue.push(sPoint);

	int index=sPoint.y*width+sPoint.x;
	flagImg[index]=0;

	while(!pointQueue.empty())
	{

		curPnt.x = pointQueue.front().x;
		curPnt.y = pointQueue.front().y;
		pointQueue.pop();
		points.push_back(curPnt);
		int dir;
		for (dir = 0; dir < 8; ++dir)
		{
			newPnt.x = curPnt.x + direction[dir][0];
			newPnt.y = curPnt.y + direction[dir][1];
			if ( IsValidCoords(newPnt.x, newPnt.y) )
			{
				int newIdx=newPnt.y*width+newPnt.x;
				if (flagImg[newIdx] == 1)
				{
					pointQueue.push(newPnt);
					flagImg[newIdx] = 0;
				}
			}
		}
	}
    
}

bool Contour::IsValidCoords(int x,int y)
{
	if(x>=0&&x<width&&y>=0&&y<height)
	{
		return true;
	}
	return false;
}

float Contour::GetWinMinDis(POINT point,VPOINTS& pointSet)
{
	float minDis= GetDistance(point, pointSet[0]);
	int pointNum = pointSet.size();
	for (int i = 1; i < pointNum; ++i)
	{
		minDis = __min(minDis, GetDistance(point, pointSet[i]));
	}
	return minDis;
}

//////////////////////////////////////////////////////////////////////////
// Test
//////////////////////////////////////////////////////////////////////////

void Contour::GetOutline()
{
     RGBQUAD b={255,0,0,0};
     RGBQUAD r={0,0,255,0};
     CxImage outline;
     vector<vector<POINT>> wp;
     vector<vector<POINT>>::iterator iwp;
     vector<POINT>::iterator iiwp;

     outline.Copy(*image);
     outline.Clear(255);
     GetAllWinowCenters(wp);
     for (iwp=wp.begin();iwp!=wp.end();++iwp)
     {
          for (iiwp=(*iwp).begin();iiwp!=(*iwp).end();++iiwp)
          {
               outline.SetPixelColor((*iiwp).x,(*iiwp).y,r,false);
          }
     }
     //     outline.Save("D:\\outlineNew.bmp",CXIMAGE_FORMAT_BMP);
}
