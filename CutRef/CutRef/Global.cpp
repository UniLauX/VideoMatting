#include "Global.h"
#include <math.h>
#include <iostream>

extern "C"
{
#include "../RobustFlow/calflow.h"
};

using namespace std;

float *LoadData(const char *name, int &w, int &h)
{
    int    p[2];
    FILE * fp;
    float *d;

    if(fopen_s(&fp, name, "rb")!=0)
        return NULL;
    else
    {
        //fread(p, sizeof(int), 2, fp);
        //w=3840;//p[0];
        //h=2160;//p[1];
        d=new float[w*h];
        fread(d, sizeof(float), w*h, fp);
        fclose(fp);
        return d;
    }
}

void SaveData(const char *name, int w, int h, const float *d)
{
    int  p[2];
    FILE *fp;

    if(fopen_s(&fp, name, "wb")==0)
    {
        //p[0]=w;
        //p[1]=h;
     //   fwrite(p, sizeof(int), 2, fp);
        fwrite(d, sizeof(float), w*h, fp);
        fclose(fp);
    }
}

void NormalizeDataCost(MRF::CostVal *dc, int w, int h, int disRange)
{
    int i,d,idx;
    float *s;
    int n=w*h;

    s=new float[n];
    memset(s, 0, sizeof(float)*n);
    for(i=0,idx=0;i<n;++i)
    {
        for(d=0;d<disRange;++d,++idx)
        {
            if(dc[idx]>s[i])
                s[i]=dc[idx];
        }
    }
    for(i=0,idx=0;i<n;++i)
    {
        for(d=0;d<disRange;++d,++idx)
        {
            dc[idx]/=(s[i]+1e-10f);
        }
    }
    delete [] s;
}

void GetCues(CxImage &A, MRF::CostVal *hCue, MRF::CostVal *vCue)
{
    int     i,j,k;
    int     x,y,count;
    int     width;
    int     height;
    int     direct[4][2]={{1,0},{0,1},{-1,0},{0,-1}};
    RGBQUAD r1,r2;
    float   s;

    float   eps(50);
    float   lambda(0.1f);

    width=A.GetWidth();
    height=A.GetHeight();
    for(i=0;i<height;++i)
    {
        for(j=0;j<width;++j)
        {
            r1=A.GetPixelColor(j, i);
            for(k=0,count=0,s=0;k<4;++k)
            {
                y=i+direct[k][1];
                x=j+direct[k][0];
                if(x>=0 && x<width && y>=0 && y<height)
                {
                    r2=A.GetPixelColor(x, y);
                    s+=1.0f/(Norm_2(r1, r2)+eps);
                    count++;
                }
            }
            s=count/s;

            r2=A.GetPixelColor(j+1, i);
            hCue[j+i*width]=s/(Norm_2(r1, r2)+eps)*lambda;
            r2=A.GetPixelColor(j, i+1);
            vCue[j+i*width]=s/(Norm_2(r1, r2)+eps)*lambda;
        }
    }
}

void SolveMRF(int w, int h, MRF::CostVal *D, float smooth, 
              MRF::CostVal *hCues, MRF::CostVal *vCues, int dis, int times, int *res)
{
    int              i,n;
    DataCost *       dc;
    SmoothnessCost * sc;
    EnergyFunction * energy;
    MRF *            mrf;
    MRF::EnergyVal   E,Ed;
    MRF::EnergyVal   Es,Eold(-1);
    float            t,tot_t(0);

    dc=new DataCost(D);
    if(hCues==NULL || vCues==NULL)
        sc=new SmoothnessCost(1, 2, smooth);
    else
        sc=new SmoothnessCost(1, 2, smooth, hCues, vCues);

    energy=new EnergyFunction(dc, sc);

    mrf=new Swap(w, h, dis, energy);
    mrf->initialize();
    mrf->clearAnswer();
    for(int iter=0;iter<times;iter++) 
    {
        mrf->optimize(1, t);
        tot_t+=t;

        Ed=mrf->dataEnergy();
        Es=mrf->smoothnessEnergy();
        E=mrf->totalEnergy();
        printf("Energy = %f (Ed=%f, Es=%f) at start\n", E, Ed, Es);

        if(E==Eold)
            break;
        Eold=E;
    }
    for(i=0,n=w*h;i<n;++i)
        res[i]=mrf->getLabel(i);

    delete dc;
    delete sc;
    delete energy;
    delete mrf;
}

void OpticalFlow(CxImage &from, CxImage &to, float *u, float *v)
{
    CxImage image1,image2;
    float * bf1;
    float * bf2;
    int     i,j,k;
    int     w,h;

    w=from.GetWidth();
    h=from.GetHeight();
    bf1=new float[w*h];
    bf2=new float[w*h];

    for(j=0,k=0;j<h;++j)
    {
        for(i=0;i<w;++i,++k)
        {
            BYTE temp1=from.GetPixelGray(i,j);
            BYTE temp2=to.GetPixelGray(i,j);

            bf1[k]=temp1;
            bf2[k]=temp2;
        }
    }
    calFlow(bf1, bf2, u, v, 4, 1, w, h);

    delete [] bf1;
    delete [] bf2;
}

void ImageToGray(CxImage &img, int *gray)
{
    int i,j;
    int w,h;
    int t;

    w=img.GetWidth();
    h=img.GetHeight();
    for(i=0,t=0;i<h;++i)
    {
        for(j=0;j<w;++j,++t)
        {
            gray[t]=img.GetPixelGray(j, i);
        }
    }
}

void GetResult(int w, int h, const int *data, CxImage &res)
{
    int   i,j,N;
    float Max(0);
    float t;
    RGBQUAD rgb;

    N=w*h;
    res.Create(w, h, 24);
    for(i=0;i<N;++i)
    {
        if(data[i]>Max)
        {
            Max=data[i];
        }
    }
    t=255.0f/Max;
    for(i=0;i<h;++i)
    {
        for(j=0;j<w;++j)
        {
            rgb.rgbRed=(int)(data[i*w+j]*t);
            rgb.rgbGreen=rgb.rgbRed;
            rgb.rgbBlue=rgb.rgbRed;
            res.SetPixelColor(j, i, rgb);
        }
    }
    cout << "Max: " << Max << endl;
}

void GetResult(int w, int h, float Max, const float *data, CxImage &res)
{
    int   i,j,N;
    float t;
    RGBQUAD rgb;

    N=w*h;
    res.Create(w, h, 24);
    if(Max<=0)
    {
        for(i=0;i<N;++i)
        {
            if(data[i]>Max)
            {
                Max=data[i];
            }
        }
    }
    t=255.0f/Max;
    for(i=0;i<h;++i)
    {
        for(j=0;j<w;++j)
        {
            rgb.rgbRed=min(255, (int)(fabs(data[i*w+j])*t));
            rgb.rgbGreen=rgb.rgbRed;
            rgb.rgbBlue=rgb.rgbRed;
            res.SetPixelColor(j, i, rgb);
        }
    }
    cout << "Max: " << Max << endl;
}

bool LoadImage(const char *dir, const char *ext, int num, CxImage &image)
{
    char name[1000];

    sprintf_s(name, "%s/%d.%s", dir, num, ext);
    return image.Load(name);
}

bool LoadImage(const char *fileTitle, const char *ext,  CxImage &image)
{
	char name[1000];
	sprintf_s(name, "%s.%s", fileTitle, ext);

	printf("Name: %s\n", name);
	return image.Load(name);
}
void ImageToLabel(CxImage &image, int *label)
{
    int i,j;
    int w,h;

    w=image.GetWidth();
    h=image.GetHeight();
    for(i=0;i<h;++i)
    {
        for(j=0;j<w;++j)
        {
            if(image.GetPixelGray(j, i)>200)
                label[j+i*w]=1;
            else
                label[j+i*w]=0;
        }
    }
}

#include <Windows.h>

void CheckMemory()
{
    MEMORYSTATUS myMemoryStatus;
    memset(&myMemoryStatus, 0, sizeof(MEMORYSTATUS));
    myMemoryStatus.dwLength = sizeof(MEMORYSTATUS);

    GlobalMemoryStatus(&myMemoryStatus);

    const int nOneM = 1024 * 1024;
    printf("memory of P %fM, available memory of P %fM\n",
        (double)myMemoryStatus.dwTotalVirtual / nOneM, (double)myMemoryStatus.dwAvailVirtual / nOneM
        );
}
