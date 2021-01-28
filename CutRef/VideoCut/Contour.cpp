#include "Contour.h"
#include "../CutRef/Global.h"
#include <iostream>

using namespace std;

Contour_ywz::Contour_ywz()
{
    width=0;
    height=0;
    pixelNum=0;
    localWinSize=80;

    label=NULL;
    image=NULL;
    winCenters.clear();
}

Contour_ywz::~Contour_ywz()
{
    Clear();
}

void Contour_ywz::ClearLocalWins()
{
    list<LocalClassifier_ywz *>::iterator iter;

    for(iter=localWins.begin();iter!=localWins.end();++iter)
    {
        delete [] *iter;
    }
    localWins.clear();
}

void Contour_ywz::Clear()
{
    ClearLocalWins();
    delete [] label;
    label=NULL;
}

void Contour_ywz::SetFrameInfo(CxImage *frame, int *l)
{
    image=frame;
    width=image->GetWidth();
    height=image->GetHeight();
    pixelNum=width*height;

    Clear();
    label=new int[pixelNum];
    memcpy(label, l, sizeof(int)*pixelNum);
}

void Contour_ywz::SetNewLabel(int *l, int size)
{
    memcpy(label, l, sizeof(int)*size);
}

void Contour_ywz::SetLocalWinSize(int newWinSize)
{
    localWinSize=newWinSize;
}

int Contour_ywz::GetLocalWinSize()
{
    return localWinSize;
}

void Contour_ywz::GetLocalClassifier()
{
    int i,j;
    int m,n;
    LocalClassifier_ywz *classifier;
    list<LocalClassifier_ywz *>::iterator iter;

    winCenters.clear();
    GetALLWindowCenters(winCenters);
    m=(int)winCenters.size();
    ClearLocalWins();

    for(i=0;i<m;++i)
    {
        n=(int)winCenters[i].size();
        classifier=new LocalClassifier_ywz[n];
        for(j=0;j<n;++j)
        {
            classifier[j].InitParam(winCenters[i][j].x, winCenters[i][j].y, 
                                    localWinSize, label, image);

            classifier[j].InitModel();
        }
        localWins.push_back(classifier);
    }
}

template<typename T>
static float Prob_ywz(int w, int h, int x, int y, int win, const T *label)
{
    int i,j;
    int half(win/2);
    T   s,c;

    for(i=y-half,s=0,c=0;i<=y+half;++i)
    {
        for(j=x-half;j<=x+half;++j)
        {
            if(j>=0 && j<w && i>=0 && i<h)
            {
                c++;
                s+=label[j+i*w];
            }
        }
    }
    return (float)s/(c+1e-10f);
}

void Contour_ywz::GetProb()
{
     int i,j;
     int winNum;
     int index;
     POINT point;
     double *F,*B,*P;
     list<LocalClassifier_ywz *>::iterator iter;

     boundPts.clear();
     Pf.clear();
     Pb.clear();

     F=new double[pixelNum];
     B=new double[pixelNum];
     P=new double[pixelNum];
     memset(F, 0, sizeof(double)*pixelNum);
     memset(B, 0, sizeof(double)*pixelNum);
     memset(P, 0, sizeof(double)*pixelNum);
     for(i=0,iter=localWins.begin();iter!=localWins.end();++iter,++i)
     {
          winNum=(int)winCenters[i].size();
          for(j=0;j<winNum;++j)
          {
               (*iter)[j].AddProbValues(F, B, P);
          }
     }

     int left,right,top,bottom;
     GetLocalWinBound(left, right, top, bottom);
     for(i=bottom;i<top;++i)
     {
          for(j=left;j<right;++j)
          {
               index=i*width+j;
               if(F[index]!=0 || B[index]!=0)
               {
                    point.x=j;
                    point.y=i;
                    boundPts.push_back(point);
                    Pf.push_back(F[index]/P[index]);
                    Pb.push_back(B[index]/P[index]);
               }
          }
     }

     delete [] F;
     delete [] B;
     delete [] P;
}

void Contour_ywz::GetLocalWinBound(int &left, int &right, int &top, int &bottom)
{
    int i,j;
    int x1,y1,x2,y2;
    int winNum;
    list<LocalClassifier_ywz *>::iterator iter;

    left=width; right=0;
    top=0; bottom=height;
    for(i=0,iter=localWins.begin();iter!=localWins.end();++iter,++i)
    {
        winNum=(int)winCenters[i].size();
        for(j=0;j<winNum;++j)
        {
            (*iter)[j].GetLeftCorner(x1, y1);
            (*iter)[j].GetRightCorner(x2, y2);
            if(x1<left)
                left=x1;
            if(x2>right)
                right=x2;
            if(y2>top)
                top=y2;
            if(y1<bottom)
                bottom=y1;
        }
    }
}

void Contour_ywz::DoCut(int *label)
{
    int n(width*height);
    float *tmp=new float[n];
    MRF::CostVal *vcues=new MRF::CostVal[n];
    MRF::CostVal *hcues=new MRF::CostVal[n];
    MRF::CostVal *D=new MRF::CostVal[n*2];

    GetCues(*image, hcues, vcues);
    memset(D, 0, sizeof(MRF::CostVal)*n);
    for(int i=0,k=0;i<height;++i)
    {
        for(int j=0;j<width;++j,++k)
        {
            tmp[k]=Prob_ywz(width, height, j, i, 10, label);
            if(tmp[k]==1.0f)
            {
                D[k*2]=1.0f;
                D[k*2+1]=0.0f;
            }
        }
    }
    for(int i=0,n=(int)boundPts.size();i<n;++i)
    {
        POINT p=boundPts[i];
        int   idx=p.x+p.y*width;
        D[idx*2]=1-Pb[i]*(1-tmp[idx]);
        D[idx*2+1]=1-Pf[i]*tmp[idx];
    }
    SolveMRF(width, height, D, 1.0f, hcues, vcues, 2, 5, label);

    delete [] vcues;
    delete [] hcues;
    delete [] D;
    delete [] tmp;
}

void Contour_ywz::GetCutResult(int *label)
{
     GetProb();
     if(boundPts.size())
          DoCut(label);
}

void Contour_ywz::GetAlphaResult(CxImage &res)
{
     int i,j;
     int w,h;
     int index;
     RGBQUAD rgb;

     w=image->GetWidth();
     h=image->GetHeight();
     res.Copy(*image);
     for (i=0;i<h;++i)
     {
          for (j=0;j<w;++j)
          {
               index=i*w+j;
               rgb.rgbBlue=(BYTE)(label[index]*255);
               rgb.rgbGreen=rgb.rgbBlue;
               rgb.rgbRed=rgb.rgbBlue;
               res.SetPixelColor(j, i, rgb);
          }
     }
}

void Contour_ywz::GetFGProb(float *P, CxImage& trimap, int& pointNum)
{
    int i,size;
    POINT point;
   
    GetLocalClassifier();
    GetProb();

    memset(P, 0, sizeof(float)*pixelNum);
   pointNum =  size=(int)boundPts.size();	
    for(i=0;i<size;++i)
    {
        point=boundPts[i];
        P[point.x+point.y*width]=Pf[i];
		trimap.SetPixelIndex(point.x, point.y, 128);
    }
}

//////////////////////////////////////////////////////////////////////////
// Test
//////////////////////////////////////////////////////////////////////////

#include <fstream>

using namespace std;

void Contour_ywz::Test()
{
     int i,size;
     POINT point;
     RGBQUAD rgb;
     CxImage pic(*image);

     GetLocalClassifier();
     TestLocalWinPosition();

     GetCutResult(label);

     size=(int)boundPts.size();
     for(i=0;i<size;++i)
     {
          point=boundPts[i];
          rgb.rgbRed=(BYTE)(255*Pf[i]);
          rgb.rgbGreen=rgb.rgbRed;
          rgb.rgbBlue=rgb.rgbRed;
          pic.SetPixelColor(point.x, point.y, rgb);
     }
     pic.Save("test/All_Prob.jpg", CXIMAGE_FORMAT_JPG);
}

void Contour_ywz::TestLocalWinPosition()
{
     int i,j,n;
     CxImage pic;
     list<LocalClassifier_ywz *>::iterator iter;

	 int winsize;
     pic.Copy(*image);
     for(i=0,iter=localWins.begin();iter!=localWins.end();++iter,++i)
     {
          n=(int)winCenters[i].size();
          for(j=0;j<n;++j)
          {
               ((*iter)[j]).TestPos(&pic);
			   winsize=((*iter)[j]).GetWidth();
          }
     }
     pic.Save("test/local.jpg", CXIMAGE_FORMAT_JPG);
}
