#include "../VideoEditing/stdafx.h"
#include "LocalClassifier.h"
#include "Param.h"
#include "../GrabCut/gmm.h"
#include "../GrabCut/global.h"
#include <fstream>
#include <iostream>

using namespace std;

static ofstream out("D:\\Result\\Prob.txt"); 

#define EPS      1e-50

CLASSIFIERPARAM LocalClassifier::param;

LocalClassifier::LocalClassifier()
{
     Init();
}

LocalClassifier::~LocalClassifier()
{
     Clear();
}

void LocalClassifier::Init()
{
     cx=0;
     cy=0;
     winSize=0;
     ptsNum=0;

     bgCluster=NULL;
     fgCluster=NULL;
     Pc=NULL;
     Fs=NULL;
     Pf=NULL;
     Pb=NULL;

     boundPts.clear();
     dis=NULL;
     valid=true;
}

void LocalClassifier::Clear()
{
     delete bgCluster;
     delete fgCluster;
     delete [] Pc;
     delete [] Fs;
     delete [] Pf;
     delete [] Pb;
     delete [] dis;
}

int LocalClassifier::GetWidth()
{
     return x2-x1;
}

int LocalClassifier::GetHeight()
{
     return y2-y1;
}

void LocalClassifier::SetParam(int x, int y, unsigned char *l, CxImage *frame)
{
     int i,j,k;
     int w,h;
     int idx1,idx2;
     POINT pt;
     int direct[4][2]={{1,0},{0,1},{-1,0},{0,-1}};

     cx=x;
     cy=y;
     this->image=frame;
     this->label=l;

     i=winSize >> 1;
     w=frame->GetWidth();
     h=frame->GetHeight();
     x1=cx-i;  x1=x1<0 ? 0:x1;
     y1=cy-i;  y1=y1<0 ? 0:y1;
     x2=cx+i;  x2=x2>w ? w:x2;
     y2=cy+i;  y2=y2>h ? h:y2;
     ptsNum=winSize*winSize;

     // Find bound pixels, should be optimized
     boundPts.clear();
     for(i=y1;i<y2;++i)
     {
          for(j=x1;j<x2;++j)
          {
               idx1=i*w+j;
               if (label[idx1])
               {			
                    for(k=0;k<4;++k)
                    {	
                         y=i+direct[k][0];
                         x=j+direct[k][1];
                         idx2=y*w+x;
                         if(x>=0 && x<w && y>=0 && y<h)
                         {
                              if(label[idx1]!=label[idx2])
                              {
                                   pt.x=j;
                                   pt.y=i;
                                   boundPts.push_back(pt);
                                   break;
                              }
                         }
                    }
               }
          }
     }
}

void LocalClassifier::InitParam(int x, int y, int winSize, unsigned char *l, CxImage *pic)
{
     Init();
     this->winSize=winSize;
     SetParam(x, y, l, pic);

     // Alloc pixel info memory
     bgCluster=new Cluster(3);	
     fgCluster=new Cluster(3);
     dis=new int[ptsNum];
     // Notice!!! 
     // OpenMP has been used
     // It is not safe
     Pc=new double[ptsNum];
     Fs=new double[ptsNum];
     Pf=new double[ptsNum];
     Pb=new double[ptsNum];
}

int LocalClassifier::GetMinDistance(int x, int y)
{
     int i,size;
     int x1,y1;
     POINT pt;
     int tmp;
     int Min(1000000);

     size=(int)boundPts.size();
     for(i=0;i<size;++i)
     {
          pt=boundPts[i];
          x1=pt.x-x;
          y1=pt.y-y;
          tmp=x1*x1+y1*y1;
          if(tmp<Min)
               Min=tmp;
     }
     return Min;
}

void LocalClassifier::InitClassifier(int mode, int type)
{
     int i,j;
     int w1,w2;
     int idx1,idx2;
     int dist;
     double F,B;
//以下为可替换代码,刚加入的
	 double a=(winSize-param.eMin)/((1-param.fcutoff)*(1-param.fcutoff));
	 int count=0;
	 double tmp;
	 tmp=Fc-param.fcutoff;
	 if(tmp>0)                                                            //formula(4) in paper,calculate sigma_s
		 es=param.eMin+a*tmp*tmp;      

	 else
		 es=param.eMin;

	 for(count=0;count<ptsNum;++count)
		 Fs[count]=1-exp(-dis[count]/(es*es));   

//以上按照原论文公式计算



     double data[3];

     int fgPixelsNum(0);
     int bgPixelsNum(0);
     double *fgPixels;
     double *bgPixels;

     dist=param.boundDis*param.boundDis;
     w1=image->GetWidth(); w2=y2-y1;
     bgPixels=new double[ptsNum*3];
     fgPixels=new double[ptsNum*3];

     for(i=y1,idx2=0;i<y2;++i)
     {
          for(j=x1;j<x2;++j,++idx2)
          {
               dis[idx2]=GetMinDistance(j, i);
               if(type==0 && dis[idx2]<=dist)
                    continue;
           count++;
               if(type==1)
               {
				   F=GetProb_GMM(j, i, fgCluster, image);
				   B=GetProb_GMM(j, i, bgCluster, image);
                    F=F/(F+B+EPS);
				
               }
               
               idx1=i*w1+j;
               GetData(*image, j, i, data);
               if(label[idx1]==1)
               {
                  if(type==1 && Fs[count]<param.foreThres)                           //源程序中用的不是Fs[count],而是F
				 //    if(type==1 && F<param.foreThres)  
                         continue;

                    idx1=3*fgPixelsNum;
                    fgPixels[idx1]=data[0];
                    fgPixels[idx1+1]=data[1];
                    fgPixels[idx1+2]=data[2];
                    ++fgPixelsNum;
               }
               else
               {
                 if(type==1 && Fs[count]>param.backThres)                          //同上
				   //    if(type==1 && F>param.backThres)    
                         continue;    

                    idx1=3*bgPixelsNum;
                    bgPixels[idx1]=data[0];
                    bgPixels[idx1+1]=data[1];
                    bgPixels[idx1+2]=data[2];
                    ++bgPixelsNum;	
               }
          }
     }

     // !!!Notice: bgPixelsNum and fgPixelsNum can be zero, there is a small hole.
     if(bgPixelsNum<=10 || fgPixelsNum<=10)
     {
          valid=false;
          return;
     }

     GMM::times=1;
     if((mode & 1)==1)
     {
          GMM::InitGMM(bgPixels, bgPixelsNum,bgCluster->clusterNum, bgCluster->mean, 
                       bgCluster->weight, bgCluster->det, bgCluster->cov);

          GMM::GetGMM(bgPixels, bgPixelsNum,bgCluster->clusterNum, bgCluster->mean, 
                      bgCluster->weight, bgCluster->det, bgCluster->cov);
     }

     if((mode & 2)==2)
     {
		
          GMM::InitGMM(fgPixels, fgPixelsNum,fgCluster->clusterNum, fgCluster->mean, 
                       fgCluster->weight,fgCluster->det, fgCluster->cov);

          GMM::GetGMM(fgPixels, fgPixelsNum,fgCluster->clusterNum, fgCluster->mean, 
                      fgCluster->weight,fgCluster->det, fgCluster->cov);
     }

     delete [] bgPixels;
     delete [] fgPixels;
}

// Test
void Output_Cluster(ofstream &out, Cluster *cluster)
{
     int k;

     for(k=0;k<cluster->clusterNum;++k)
     {
          out << "Cluster " << k  << ": " << endl;
          out << "Det:    " << cluster->det[k] << endl;
          out << "Weight: " << cluster->weight[k] << endl;
     }
     out << endl;
}

void LocalClassifier::GetColorProb()
{
     int i,j,w;
     int index;
     double F,B;
     double s1,s2;
     double tmp;
     double delta;

     if(!valid)
          return;
     
     delta=winSize>>1;
     delta*=delta;
     w=image->GetWidth();
     s1=s2=index=0;
     for(i=y1;i<y2;++i)
     {
          for(j=x1;j<x2;++j,++index)
          {
               F=GetProb_GMM(j, i, fgCluster, image);
               B=GetProb_GMM(j, i, bgCluster, image);
               Pc[index]=F/(F+B+EPS);

               //if(!(Pc[index]>=0 && Pc[index]<=1))
               //{
               //     out << "F: " << F << endl;
               //     out << "B: " << B << endl;
               //     out << Pc[index] << endl;
               //     out << "FG GMM:" << endl;
               //     Output_Cluster(out, fgCluster);
               //     out << "BG GMM:" << endl;
               //     Output_Cluster(out, bgCluster);
               //     out << "---------------------------" << endl;
               //}
               //
               tmp=Got_Exp(-dis[index]/delta);
               s2+=tmp;
               s1+=fabs(label[i*w+j]-Pc[index])*tmp;
          }
     }
     Fc=1-s1/s2;
}

void LocalClassifier::GetShapeParam(double a)
{
     int i;
     double tmp;

     tmp=Fc-param.fcutoff;
     if(tmp>=0)
          es=param.eMin+a*tmp*tmp;
     else
          es=param.eMin;
     
     for(i=0;i<ptsNum;++i)
     {
          tmp=dis[i]/(es*es);
          Fs[i]=1-Got_Exp(-tmp);
/*
          out << "Es:  " << es << endl;
          out << "Dis: " << dis[i] << endl;
          out << "Tmp: " << tmp << endl;
          out << "Fs:  " << Fs[i] << endl;
          out << "-------------------------------------------\n" << endl;
*/
     }
//     out << "***********************************\n" << endl;
}

void LocalClassifier::GetModelProb(bool shape)
{
     int i,j,w;
     int index;

     // Test
     if(!valid)
          return;

     w=image->GetWidth();
     if(shape)
     {
          for(i=y1,index=0;i<y2;++i)
          {
               for(j=x1;j<x2;++j,++index)
               {
                    Pf[index]=Fs[index]*label[i*w+j]+(1-Fs[index])*Pc[index];
                    Pb[index]=Fs[index]*(1-label[i*w+j])+(1-Fs[index])*(1-Pc[index]);
               }
          }
     }
     else
     {
          for(i=y1,index=0;i<y2;++i)
          {
               for(j=x1;j<x2;++j,++index)
               {
                    Pf[index]=Pc[index];
                    Pb[index]=1-Pc[index];
               }
          }
     }
}

void LocalClassifier::InitModel(int mode, bool shape, int type)
{    
	 double startT,endT,duration;
     startT=clock();
     InitClassifier(mode, type);
	 endT=clock();
	 duration=(endT-startT)/CLK_TCK;
	 //std::cout<<" init classifier time: "<< duration<<std::endl;

     startT=clock();
	 GetModelParam(shape);
     endT=clock();
     duration=(endT-startT)/CLK_TCK;
	 //std::cout<<"get model param time: "<<duration<<std::endl;
}

void LocalClassifier::GetModelParam(bool shape)
{
     double a;
     a=(winSize-param.eMin)/((1-param.fcutoff)*(1-param.fcutoff));
     GetColorProb();
     GetShapeParam(a);
     GetModelProb(shape);
}

//void LocalClassifier::AddProbValues(double *F, double *B, double *P)
void LocalClassifier::AddProbValues(std::map<std::pair<int, int>, double>& F, std::map<std::pair<int ,int>, double>& B, std::map<std::pair<int, int>, double>& P)
{
     int i,j,w;
     int index,idx;
     double x,y;
     // Test
     if(!valid)
          return;
     w=image->GetWidth();
     for(i=y1,index=0;i<y2;++i)
     {
          for(j=x1;j<x2;++j,++index)
          {
               x=j-cx;
               y=i-cy;
               x=1.0/(sqrt(x*x+y*y)+0.1);
               idx=i*w+j;
               //F[idx]+=Pf[index]*x;
			F[make_pair(j,i)] +=Pf[index]*x;
               //B[idx]+=Pb[index]*x;
			 B[make_pair(j,i)] += Pb[index]*x;
               //P[idx]+=x;
			P[make_pair(j,i)] +=x;
          }
     }
}

void LocalClassifier::AddProbValues(stdext::hash_map<int, double>& F, stdext::hash_map<int, double>& B, stdext::hash_map<int, double>& P)
{
	int i,j,w;
	int index,idx;
	double x,y;
	// Test
	if(!valid)
		return;
	w=image->GetWidth();

	for(i=y1,index=0;i<y2;++i)
	{
		for(j=x1;j<x2;++j,++index)
		{
			x=j-cx;
			y=i-cy;
			x=1.0/(sqrt(x*x+y*y)+0.1);
			idx=i*w+j;
			F[idx] +=Pf[index]*x;
			B[idx] += Pb[index]*x;
			P[idx] +=x;
			//point2 ptemp;
			//ptemp.x = j;
			//ptemp.y = i;
			//F[ptemp] += Pf[index]*x;
			//B[ptemp] += Pb[index]*x;
			//P[ptemp] += x;
		}
	}
}

void LocalClassifier::AddProbValues(double *F, double *B, double *P)
{

	int i,j,w;
	int index,idx;
	double x,y;
	// Test
	if(!valid)
		return;
	w=image->GetWidth();
	for(i=y1,index=0;i<y2;++i)
	{
		for(j=x1;j<x2;++j,++index)
		{
			x=j-cx;
			y=i-cy;
			x=1.0/(sqrt(x*x+y*y)+0.1);
			idx=i*w+j;
			F[idx] +=Pf[index]*x;
			B[idx] += Pb[index]*x;
			P[idx] +=x;
		}
	}
}


void LocalClassifier::Move(unsigned char *label, CxImage *frame, VVPOINTS &winCenters)    //三个参数均为新一帧的参数
{
    int dis,Max;
    VVPOINTS::iterator m;
    VPOINTS::iterator n,pt;

    Max=100000000;
    for(m=winCenters.begin();m!=winCenters.end();++m)
    {
        for(n=m->begin();n!=m->end();++n)
        {
            dis=(cx-n->x)*(cx-n->x)+(cy-n->y)*(cy-n->y);
 
		//	cout<<" ("<<cx<<","<<cy<<"} ";                

            if(dis<Max)      
            {
                Max=dis;
                pt=n;
            }
        }
	
    }
//	cout<<"max= "<<Max<<endl;
    SetParam(pt->x, pt->y, label, frame);    //主要根据传入参数设置cx，cy和 boundPts.
}

void LocalClassifier::DoMatting(Matting &matting, double *alpha, int *count)
{
     int i,j;
     int x,y;
     int idx;
     int index;
     RGBQUAD rgb;
     CxImage image;
     CxImage scrbImage;
     double *localAlpha;
     int boundWidth(80);

     if(!valid)
          return;

     image.Create(GetWidth(), GetHeight(), 24);
     for(i=y1,y=0;i<y2;++i,++y)
     {
          for(j=x1,x=0;j<x2;++j,++x)
          {
               rgb=this->image->GetPixelColor(j, i);
               image.SetPixelColor(x, y, rgb);
          }
     }
     scrbImage.Copy(image);
     for(i=y1,y=0,idx=0;i<y2;++i,++y)
     {
          for(j=x1,x=0;j<x2;++j,++x,++idx)
          {
               if(GetMinDistance(j, i)<=boundWidth)
                    continue;
               index=i*this->image->GetWidth()+j;
               rgb.rgbRed=(BYTE)(label[index]*255);
               rgb.rgbGreen=rgb.rgbRed;
               rgb.rgbBlue=rgb.rgbRed;
               scrbImage.SetPixelColor(x, y, rgb);
          }
     }

     matting.SetImage(&image, &scrbImage);
     localAlpha=matting.DoMatting();
     for(i=y1,idx=0;i<y2;++i)
     {
          for(j=x1;j<x2;++j,++idx)
          {
               index=i*this->image->GetWidth()+j;
               alpha[index]+=localAlpha[idx];
               count[index]++;
          }
     }
/*
     static int naindex(0);
     CString dir;
     dir.Format("%s%d%s", "D:\\Result\\Test\\Test_s", naindex, ".jpg");
     scrbImage.Save(dir, CXIMAGE_FORMAT_JPG);
     dir.Format("%s%d%s", "D:\\Result\\Test\\Test_i", naindex++, ".jpg");
     image.Save(dir, CXIMAGE_FORMAT_JPG);
*/
     delete [] localAlpha;
}

////////////////////////////////////////////

void LocalClassifier::TestFs(CxImage *image)
{
     int i,j,w;
     int index;
     RGBQUAD rgb;
/*
     InitClassifier();
     GetColorProb();
*/
     if(!valid)
          return;

     w=y2-y1;
     for(i=y1,index=0;i<y2;++i)
     {
          for(j=x1;j<x2;++j,++index)
          {
               rgb.rgbRed=Fs[index]*255;
               rgb.rgbGreen=rgb.rgbRed;
               rgb.rgbBlue=rgb.rgbRed;
               image->SetPixelColor(j, i, rgb);
          }
     }
}

void LocalClassifier::Test(CxImage *image)
{
     int i,j;
     int idx;
     double a;
     RGBQUAD rgb;

     //InitClassifier();
     GetColorProb();

     a=(winSize-param.eMin)/((1-param.fcutoff)*(1-param.fcutoff));
     GetShapeParam(a);
     GetModelProb();

     for(i=y1,idx=0;i<y2;++i)
     {
          for(j=x1;j<x2;++j,++idx)
          {
               rgb.rgbRed=(BYTE)(255*(Pf[idx]>1?1:Pf[idx]));
               rgb.rgbGreen=rgb.rgbRed;
               rgb.rgbBlue=rgb.rgbRed;
               image->SetPixelColor(j, i, rgb);
          }
     }
}

void LocalClassifier::TestPos(CxImage *image)
{
     RGBQUAD rgb;

     rgb.rgbRed=255;
     rgb.rgbGreen=0;
     rgb.rgbBlue=0;

     if(!valid)
          rgb.rgbGreen=255;

     image->DrawLine(x1, x2, y1, y1, rgb);
     image->DrawLine(x2, x2, y1, y2, rgb);
     image->DrawLine(x1, x2, y2, y2, rgb);
     image->DrawLine(x1, x1, y1, y2, rgb);
}

#include <math.h>

void LocalClassifier::TestDis(CxImage *image)
{
     int i,j,w;
     int index;
     RGBQUAD rgb;

     if(!valid)
          return;

     w=y2-y1;
     for(i=y1,index=0;i<y2;++i)
     {
          for(j=x1;j<x2;++j,++index)
          {
               rgb.rgbRed=sqrt((float)dis[index])*(510.0/winSize);
               rgb.rgbGreen=rgb.rgbRed;
               rgb.rgbBlue=rgb.rgbRed;
               image->SetPixelColor(j, i, rgb);
          }
     }
}
