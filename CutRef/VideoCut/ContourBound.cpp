#include "Contour.h"
#include <iostream>
#include <fstream>
#include <omp.h>
#include <atlstr.h>

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

void Contour_ywz::GetBound(int *flag, vector<POINT> &points)
{
     int i,j,k;
     int x,y;
     int index1,index2;
     POINT point;
     int direct[4][2]={{1,0},{0,1},{-1,0},{0,-1}};

     for(i=0;i<height;++i)
     {
          for(j=0;j<width;++j)
          {
               index1=i*width+j;
               if(label[index1])
               {			
                    for(k=0;k<4;++k)
                    {	
                         y=i+direct[k][0];
                         x=j+direct[k][1];
                         index2=y*width+x;
                         if(x>=0 && x<width && y>=0 && y<height)
                         {
                              if(label[index1]!=label[index2])
                              {
                                   point.x=j;
                                   point.y=i;
                                   points.push_back(point);
                                   flag[index1]=1;
                                   break;
                              }
                         }
                    }

               }

          }
     }

	 for (int i=0; i<width; ++i)
	 {
		 if (label[0*width+i])
		 {
			 POINT p;
			 p.x = i;
			 p.y = 0;
			 points.push_back(p);
		 }
		 if (label[(height-1)*width+i])
		 {
			 POINT p;
			 p.x = i;
			 p.y = height-1;
			 points.push_back(p);
		 }
	 }
	 for (int j=0; j<height; ++j)
	 {
		 if (label[j*width+0])
		 {
			 POINT p;
			 p.x = 0;
			 p.y = j;
			 points.push_back(p);
		 }
		 if (label[j*width+width-1])
		 {
			 POINT p;
			 p.x = width-1;
			 p.y = j;
			 points.push_back(p);
		 }
	 }


}

//void Contour_ywz::GetOrderedBound(const POINT &sPoint, int **alpha, 
//                              int *flag, VPOINTS &points, int num)
//{
//     //convert one demision to array 
//     POINT point;
//     int x0,y0;//first point
//     int x1(-2),y1(-2);//second point
//     int xc(-1),yc(-1);//current border point 
//     int xp(-1),yp(-1);//previous border point
//     int dir(7);
//     int count(0);
//     int mm = num;
//     int cc(0);
//     int xtemp,ytemp;
//
//     //find the start point
//     x0=sPoint.x;
//     y0=sPoint.y;
//     xc=sPoint.x;
//     yc=sPoint.y;
//
//     while(mm)
//     {
//          --mm;
//          count++;
//          point.x=xc;
//          point.y=yc;
//          if(flag[yc*width+xc])
//          {
//               flag[yc*width+xc]=0;
//               cc++;
//               points.push_back(point);
//          }
//          if(dir & 1)
//          {
//               dir=(dir+6) & 7;
//          }
//          else
//               dir=(dir+7) & 7;
//
//          for(int p=0;p<8;++p)
//          {
//
//               if(GetNeighbourValue(xc, yc, dir, xtemp, ytemp, alpha))
//               {
//                    xp=xc;
//                    yp=yc;
//                    xc=xtemp;
//                    yc=ytemp;
//                    if (count==1)
//                    {
//                         x1=xc;
//                         y1=yc;
//                    }
//                    break;
//               }
//               else
//               {
//                    dir++;
//                    dir &=7;
//               }
//          }
//     }
//}

void Contour_ywz::GetOrderedBound(const POINT &sPoint, int **alpha, 
							  int *flag, VPOINTS &points, int num)
{
	POINT point;
	int x0,y0;//first point
	int x1(-2),y1(-2);//second point
	int xc(-1),yc(-1);//current border point 
	int xp(-1),yp(-1);//previous border point
	int dir(7);
	int count(0);
	int mm = num;
	int cc(0);
	int xtemp,ytemp;
	RGBQUAD r={255,0,0,0};
	//find the start point
	x0=sPoint.x;
	y0=sPoint.y;
	xc=sPoint.x;
	yc=sPoint.y;
	do
	{
		--mm;
		count++;
		point.x=xc;
		point.y=yc;
		if (flag[yc*width+xc])
		{
			flag[yc*width+xc]=0;
			cc++;
			points.push_back(point);
		}
		if (dir & 1)
		{
			dir=(dir+6) & 7;
		}
		else
			dir=(dir+7) & 7;
		int p;
		for ( p=0;p<8;++p)
		{
			if (GetNeighbourValue(xc,yc,dir,xtemp,ytemp,alpha))
			{
				xp=xc;
				yp=yc;
				xc=xtemp;
				yc=ytemp;
				if (count==1)
				{
					x1=xc;
					y1=yc;
				}
				break;
			}
			else
			{
				dir++;
				dir &=7;
			}
		}
	} while (mm);
}
int Contour_ywz::GetNeighbourValue(int x, int y, int dir, 
                               int &xtemp, int &ytemp, int **alpha)
{
     static int direction[8][2]={{1,0},{1,1},{0,1},{-1,1},
                                 {-1,0},{-1,-1},{0,-1},{1,-1}};

     if(!(x>=0 && x<width && y>=0 && y<height))
          return 0;

     xtemp=x+direction[dir][0];
     ytemp=y+direction[dir][1];
     if(xtemp>=0 && xtemp<width && ytemp>=0 && ytemp<height)
     {
	     if (alpha[xtemp][ytemp]==1)
		     return 1;
	     else
		     return 0;
     }
     else
	     return 0;
    
}

void Contour_ywz::GetWindowCenter(const VPOINTS &bound, VPOINTS &centers)
{
     vector<POINT>::const_iterator ib;
     POINT firstPoint(bound[0]);
     POINT currentPoint(firstPoint);
     double dis=(1.0/2)*localWinSize;               //1/3 overlap

     centers.push_back(firstPoint);
     for(ib=bound.begin()+1;ib!=bound.end();++ib)
     {
          if((dis-GetDistance(currentPoint, *ib))<0.5)
          {
               currentPoint=*ib;
               centers.push_back(currentPoint);
          }
     }
}
void Contour_ywz::GetWindowCenter_jin(const VPOINTS &bound, VPOINTS &centers, vector<VPOINTS>& cb)
{

	if (!bound.size())
	{
		centers.clear();
		return;
	}

	vector<POINT>::const_iterator ib;
	POINT firstPoint(bound[0]);
	POINT currentPoint(firstPoint);
	double dis=(2.0/3)*localWinSize;               //1/3 overlap

	for(ib=bound.begin()+1;ib!=bound.end();++ib)
	{
		if((dis-GetDistance(currentPoint, *ib))<0.5)
		{
			currentPoint=*ib;
			centers.push_back(currentPoint);
		}
	}
	if (centers.size()<4)
		return;

	int count(0);
	vector<POINT> c1;
	vector<POINT> c2;
	vector<POINT>::iterator ic;
	vector<POINT>::iterator ic2;

	ib=bound.begin();
	ic=centers.begin();
	ic2=ic+1;
	while (ic!=centers.end() && ic2!=centers.end())
	{
		while ((*ib).x!=(*ic2).x || (*ib).y!=(*ic2).y)
		{
			if (IsInWindow(*ib,*ic,localWinSize))
			{
				c1.push_back(*ib);
			}
			if (IsInWindow(*ib,*ic2,localWinSize))
			{
				c2.push_back(*ib);
			}
			++ib;
		}
		++count;
		if (count & 1)
		{
			ic=ic2+1;
			cb.push_back(c1);
			c1.clear();
		}
		else
		{
			ic2=ic+1;
			cb.push_back(c2);
			c2.clear();
		}
	}

	if (ic==centers.end())
	{
		while (ib!=bound.end())
		{
			if (IsInWindow(*ib,*ic2,localWinSize))
			{
				c2.push_back(*ib);
			}
			if (IsInWindow(*ib,*(centers.begin()),localWinSize))
			{
				cb[0].push_back(*ib);
			}
			++ib;
		}
		cb.push_back(c2);
	}
	else
	{
		while (ib!=bound.end())
		{
			if (IsInWindow(*ib,*ic,localWinSize))
			{
				c1.push_back(*ib);
			}
			if (IsInWindow(*ib,*(centers.begin()),localWinSize))
			{
				cb[0].push_back(*ib);
			}
			++ib;
		}
		cb.push_back(c1);
	}

}
void Contour_ywz::GetALLWindowCenters(vector<VPOINTS> &vec)
{
     int i,j;
     int index;
     int *flag;
     int **alpha;
     vector<POINT>::iterator ibp;
     vector<POINT> boundPoints;
     vector<POINT> vOrder;
     vector<POINT> vCenter;
	vector<vector<POINT>> cb;  
     flag=new int[pixelNum];
     alpha=new int *[width];
     for(i=0;i<width;++i)
     {
          alpha[i]=new int[height];
          for(j=0;j<height;++j)
               alpha[i][j]=label[j*width+i];
     }

     memset(flag, 0, sizeof(int)*pixelNum);
     GetBound(flag, boundPoints);
	 static int count=0;
	 CString path = "H:/test/bound_get";
	 path.AppendFormat("%d", count);
	 path.Append(".png");
	 count++;
	 CxImage boundimage;
	 boundimage.Create(width, height, 24);
     for(ibp=boundPoints.begin();ibp!=boundPoints.end();++ibp)
     {
          index=(*ibp).y*width+(*ibp).x;
		  //boundimage.SetPixelColor((*ibp).x, (*ibp).y, RGB(255,0,0));
          if(flag[index])
          {
               vOrder.clear();
               vCenter.clear();
			   cb.clear();
               GetOrderedBound(*ibp, alpha, flag, vOrder, boundPoints.size());
               if(vOrder.size())
               {
                    GetWindowCenter_jin(vOrder, vCenter, cb);
					if (vCenter.size()>=2)
					{
						vec.push_back(vCenter);
					}
               }
          }
     }
	 for (int i=0; i<vec.size(); ++i)
	 {
		 for(int j=0; j<vec[i].size(); ++j)
			 boundimage.SetPixelColor(vec[i][j].x, vec[i][j].y, RGB(255,0,0));
	 }
	 boundimage.Save(path.GetBuffer(), CXIMAGE_FORMAT_PNG);
     for(i=0;i<width;++i)
          delete [] alpha[i];
     delete [] alpha;
     delete [] flag;
}

//////////////////////////////////////////////////////////////////////////
// Test
//////////////////////////////////////////////////////////////////////////

void Contour_ywz::GetOutline()
{
     RGBQUAD b={255,0,0,0};
     RGBQUAD r={255,255,255,0};
     CxImage outline;
     vector<vector<POINT>> wp;
     vector<vector<POINT>>::iterator iwp;
     vector<POINT>::iterator iiwp;

     outline.Copy(*image);
     outline.Clear(0);
     GetALLWindowCenters(wp);
     for (iwp=wp.begin();iwp!=wp.end();++iwp)
     {
          for (iiwp=(*iwp).begin();iiwp!=(*iwp).end();++iiwp)
          {
               outline.SetPixelColor((*iiwp).x, (*iiwp).y, r);
          }
     }
     outline.Save("test/outlineNew.jpg", CXIMAGE_FORMAT_JPG);
}
