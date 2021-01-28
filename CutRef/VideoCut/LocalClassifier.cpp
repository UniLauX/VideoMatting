#include "LocalClassifier.h"
#include "Param.h"

#include <fstream>
#include <iostream>

using namespace std;

static ofstream out("D:\\Result\\Prob.txt"); 

#define EPS      1e-50

CLASSIFIERPARAM LocalClassifier_ywz::param;

LocalClassifier_ywz::LocalClassifier_ywz()
{
     Init();
}

LocalClassifier_ywz::~LocalClassifier_ywz()
{
     Clear();
}

void LocalClassifier_ywz::Init()
{
     cx=0;
     cy=0;
     winSize=0;
     ptsNum=0;

     bgCluster=NULL;
     fgCluster=NULL;
     Pc=NULL;
     Pf=NULL;
     Pb=NULL;

     valid=true;
}

void LocalClassifier_ywz::Clear()
{
     delete bgCluster;
     delete fgCluster;
     delete [] Pc;
     delete [] Pf;
     delete [] Pb;
     bgCluster=NULL;
     fgCluster=NULL;
     Pc=NULL;
     Pf=NULL;
     Pb=NULL;
}

void LocalClassifier_ywz::SetParam(int x, int y, int *l, CxImage *frame)
{
     int i;
     int w,h;

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
}

void LocalClassifier_ywz::InitParam(int x, int y, int winSize, int *l, CxImage *pic)
{
     Init();
     this->winSize=winSize;
     SetParam(x, y, l, pic);

     // Alloc pixel info memory
     bgCluster=new Cluster_ywz(3);	
     fgCluster=new Cluster_ywz(3);
     Pc=new double[ptsNum];
     Pf=new double[ptsNum];
     Pb=new double[ptsNum];
}

void LocalClassifier_ywz::InitClassifier()
{
     int i,j;
     int w1,w2;
     int idx1,idx2;
     double data[3];

     int fgPixelsNum(0);
     int bgPixelsNum(0);
     double *fgPixels;
     double *bgPixels;

     w1=image->GetWidth(); w2=y2-y1;
     bgPixels=new double[ptsNum*3];
     fgPixels=new double[ptsNum*3];

     for(i=y1,idx2=0;i<y2;++i)
     {
          for(j=x1;j<x2;++j,++idx2)
          {
               idx1=i*w1+j;
               GetData_ywz(*image, j, i, data);
               if(label[idx1]==1)
               {
                    idx1=3*fgPixelsNum;
                    fgPixels[idx1]=data[0];
                    fgPixels[idx1+1]=data[1];
                    fgPixels[idx1+2]=data[2];
                    ++fgPixelsNum;
               }
               else
               {
                    idx1=3*bgPixelsNum;
                    bgPixels[idx1]=data[0];
                    bgPixels[idx1+1]=data[1];
                    bgPixels[idx1+2]=data[2];
                    ++bgPixelsNum;	
               }
          }
     }
     if(fgPixelsNum/(float)(winSize*winSize)<0.05f ||
        bgPixelsNum/(float)(winSize*winSize)<0.05f)
     {
          valid=false;
          delete [] bgPixels;
          delete [] fgPixels;
          return;
     }

     GMM_ywz::times=1;
     GMM_ywz::Train(bgPixels, bgPixelsNum, *bgCluster);
     GMM_ywz::Train(fgPixels, fgPixelsNum, *fgCluster);

     delete [] bgPixels;
     delete [] fgPixels;
}

void LocalClassifier_ywz::GetColorProb()
{
     int i,j,w;
     int index;
     double F,B;
     double s1,s2;
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
          }
     }
}

void LocalClassifier_ywz::GetModelProb()
{
     int i,j,w;
     int index;

     if(!valid)
          return;

     w=image->GetWidth();
     for(i=y1,index=0;i<y2;++i)
     {
          for(j=x1;j<x2;++j,++index)
          {
               Pf[index]=Pc[index];
               Pb[index]=1-Pc[index];
          }
     }
}

void LocalClassifier_ywz::InitModel()
{    
     InitClassifier();
	 GetModelParam();
}

void LocalClassifier_ywz::GetModelParam()
{
     double a;
     a=(winSize-param.eMin)/((1-param.fcutoff)*(1-param.fcutoff));
     GetColorProb();
     GetModelProb();
}

void LocalClassifier_ywz::AddProbValues(double *F, double *B, double *P)
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
               F[idx]+=Pf[index]*x;
               B[idx]+=Pb[index]*x;
               P[idx]+=x;
          }
     }
}

////////////////////////////////////////////

void LocalClassifier_ywz::Test(CxImage *image)
{
     int i,j;
     int idx;
     double a;
     RGBQUAD rgb;

     //InitClassifier();
     GetColorProb();

     a=(winSize-param.eMin)/((1-param.fcutoff)*(1-param.fcutoff));
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

void LocalClassifier_ywz::TestPos(CxImage *image)
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
