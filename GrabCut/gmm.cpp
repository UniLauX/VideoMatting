#include "../VideoEditing/stdafx.h"
#include "GMM.h"
#include <math.h>
#include <string.h>
#include "../Vision/Others/OpenCV10/Include/cv.h"

#include <time.h>
#include "../include/CxImage/ximage.h"
#include <iostream>

using namespace std;

#define EPSILON 1e-10
#define TESTNAME "test10.jpg"

int GMM::times=5;

void Inverse(const double mat[], double result[])
{
    int i;
    double Det;

    result[0]=mat[4]*mat[8]-mat[5]*mat[7];
    result[1]=mat[2]*mat[7]-mat[1]*mat[8];
    result[2]=mat[1]*mat[5]-mat[2]*mat[4];
    result[3]=mat[5]*mat[6]-mat[3]*mat[8];
    result[4]=mat[0]*mat[8]-mat[2]*mat[6];
    result[5]=mat[2]*mat[3]-mat[0]*mat[5];
    result[6]=mat[3]*mat[7]-mat[4]*mat[6];
    result[7]=mat[1]*mat[6]-mat[0]*mat[7];
    result[8]=mat[0]*mat[4]-mat[1]*mat[3];

    Det=mat[0]*result[0]+mat[1]*result[3]+mat[2]*result[6];
    Det=1.0/(Det+EPSILON);
    for(i=0;i<9;++i)
        result[i]*=Det;
}

inline double Determinant(double mat[])
{
    double fCo00=mat[4]*mat[8]-mat[5]*mat[7];
    double fCo10=mat[5]*mat[6]-mat[3]*mat[8];
    double fCo20=mat[3]*mat[7]-mat[4]*mat[6];
    double fDet =mat[0]*fCo00+mat[1]*fCo10+mat[2]*fCo20;

    return fDet+EPSILON;
}

double Prob(double *data, double *mean, double det, double *segma)
{
    double a[3];
    double b[3];
    double c;
    double inv[9];

    Inverse(segma, inv);
    a[0]=data[0]-mean[0];
    a[1]=data[1]-mean[1];
    a[2]=data[2]-mean[2];
    b[0]=a[0]*inv[0]+a[1]*inv[3]+a[2]*inv[6];
    b[1]=a[0]*inv[1]+a[1]*inv[4]+a[2]*inv[7];
    b[2]=a[0]*inv[2]+a[1]*inv[5]+a[2]*inv[8];

    c=a[0]*b[0]+a[1]*b[1]+a[2]*b[2];

	return exp(-0.5*c)/sqrt(fabs(det)+EPSILON);
}

double Prob(double *data, double *mean, double *segma)
{
    double a[3];
    double b[3];
    double c;
    double inv[9];

    Inverse(segma, inv);
    a[0]=data[0]-mean[0];
    a[1]=data[1]-mean[1];
    a[2]=data[2]-mean[2];
    b[0]=a[0]*inv[0]+a[1]*inv[3]+a[2]*inv[6];
    b[1]=a[0]*inv[1]+a[1]*inv[4]+a[2]*inv[7];
    b[2]=a[0]*inv[2]+a[1]*inv[5]+a[2]*inv[8];

    c=a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
    if(c<0)
        return log(EPSILON);  // Some errors.
    else
        return -0.5*c;
}

void GMM::GetGMM(double data[], int dataNum, int clusterNum, 
                 double mean[][3], double pi[], double det[], double *segma[9])
{

    int t;
    int *label;

    label=new int[dataNum];
    for(t=0;t<times;++t)
    {
        GetLabel(data, dataNum, clusterNum, mean, pi, det, segma, label);
        NewGMM(data, dataNum, clusterNum, mean, pi, det, segma, label);
    }
    delete [] label;
}

void GMM::InitGMM(double data[], int dataNum, int clusterNum, 
                  double mean[][3], double pi[], double det[], double *segma[9])
{
    int i;
    int idx;
    CvPoint3D32f *pt;
    CvMat *points=cvCreateMat(dataNum, 1, CV_32FC3);
    CvMat *clusters=cvCreateMat(dataNum, 1, CV_32SC1);

    for(i=0;i<dataNum;++i)
    {
        idx=i*3;
        pt=(CvPoint3D32f *)points->data.fl+i;
        pt->x=(float)data[idx];
        pt->y=(float)data[idx+1];
        pt->z=(float)data[idx+2];
    }
    cvKMeans2(points, clusterNum, clusters,
              cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0));

    NewGMM(data, dataNum, clusterNum, mean, pi, det, segma, clusters->data.i);
    cvReleaseMat(&points);
    cvReleaseMat(&clusters);
}

void GMM::GetLabel(double data[], int dataNum, int clusterNum, 
                   double mean[][3], double pi[], double det[], double *segma[9],
                   int label[])
{
    int i,k;
    double tmp,Max;

    #pragma omp parallel for private(Max,k,tmp)
    for(i=0;i<dataNum;++i)
    {
        Max=1e100;
        for(k=0;k<clusterNum;++k)
        {
            tmp=log(pi[k]*det[k]+EPSILON)*Prob(&data[i*3], mean[k], segma[k]);
            if(tmp<Max)
            {
                Max=tmp;
                label[i]=k;
            }
        }
    }
}

void GMM::NewGMM(double data[], int dataNum, int clusterNum, 
                 double mean[][3], double pi[], double det[], double *segma[9],
                 int label[])
{
    int i,k;
    int idx;
    double a,b,c;
    int *num;
	double T(1);

    num=new int[clusterNum];
    memset(num, 0, sizeof(int)*clusterNum);
    for(i=0;i<clusterNum;++i)
    {
        mean[i][0]=0;
        mean[i][1]=0;
        mean[i][2]=0;
        memset(segma[i], 0, sizeof(double)*9);
    }
    for(i=0;i<dataNum;++i)
    {
        idx=i*3;
        k=label[i];
        num[k]++;
        mean[k][0]+=data[idx];
        mean[k][1]+=data[idx+1];
        mean[k][2]+=data[idx+2];
    }
    for(i=0;i<clusterNum;++i)
    {
        a=1.0/((float)num[i]+EPSILON);
        mean[i][0]*=a;
        mean[i][1]*=a;
        mean[i][2]*=a;
    }
    for(i=0;i<dataNum;++i)
    {
        idx=i*3;
        k=label[i];
        a=data[idx]-mean[k][0];
        b=data[idx+1]-mean[k][1];
        c=data[idx+2]-mean[k][2];
        segma[k][0]+=a*a+T; segma[k][1]+=a*b; segma[k][2]+=a*c;
        segma[k][3]+=b*a; segma[k][4]+=b*b+T; segma[k][5]+=b*c;
        segma[k][6]+=c*a; segma[k][7]+=c*b; segma[k][8]+=c*c+T;
    }
    for(k=0;k<clusterNum;++k)
    {
        for(i=0;i<9;++i)
            segma[k][i]/=((float)num[k]+EPSILON);
        pi[k]=(dataNum==0)?0:num[k]/(float)dataNum;
        det[k]=Determinant(segma[k]);
    }

    delete [] num;
}

/************************************************************************/
/* Test                                                                 */
/************************************************************************/
void GMM::Print(int clusterNum, double mean[][3], double pi[], 
                double det[], double *segma[9])
{
    int k;
    double inv[9];

    for(k=0;k<clusterNum;++k)
    {
        cout << "Mean: " << endl;
        cout << mean[k][0] << " " << mean[k][1] << " " << mean[k][2] << endl;
        cout << "Pi: " << pi[k] << endl;
        cout << "Segma: " << endl;
        cout << segma[k][0] << " " << segma[k][1] << " " << segma[k][2] << endl;
        cout << segma[k][3] << " " << segma[k][4] << " " << segma[k][5] << endl;
        cout << segma[k][6] << " " << segma[k][7] << " " << segma[k][8] << endl;
        cout << "Det: " << det[k] << endl;

        Inverse(segma[k], inv);
        cout << "Inv: " << endl;
        cout << inv[0] << " " << inv[1] << " " << inv[2] << endl;
        cout << inv[3] << " " << inv[4] << " " << inv[5] << endl;
        cout << inv[6] << " " << inv[7] << " " << inv[8] << endl;
    }
}

void GMM::TestGMM()
{
    int i,j;
    int index;
    int w,h,N;
    double *data;
    RGBQUAD rgb;
    CxImage image;

    image.Load(TESTNAME);
    w=image.GetWidth();
    h=image.GetHeight();
    N=w*h;
    data=new double[N*3];

    for(i=0;i<h;++i)
    {
        for(j=0;j<w;++j)
        {
            rgb=image.GetPixelColor(j, i);
            index=(i*w+j)*3;
            data[index]=rgb.rgbRed;
            data[index+1]=rgb.rgbGreen;
            data[index+2]=rgb.rgbBlue;
        }   
    }

    double mean[5][3];
    double pi[5],det[5];
    double **segma;
    int *label;
    int clusterNum(5);

    label=new int[N];
    segma=new double *[clusterNum];
    for(i=0;i<clusterNum;++i)
        segma[i]=new double[9];

    InitGMM(data, N, clusterNum, mean, pi, det, segma);
    Print(clusterNum, mean, pi, det, segma);
    cout << "-----------------------------------------" << endl;

    double start=clock();
    GetGMM(data, N, clusterNum, mean, pi, det, segma);
    double stop=clock();
    double duration=((double)(stop-start))/CLK_TCK;
    Print(clusterNum, mean, pi, det, segma);
    GetLabel(data, N, clusterNum, mean, pi, det, segma, label);
    cout << "Time: " << duration << endl;
    image.Clear();
    for(i=0;i<h;++i)
    {
        for(j=0;j<w;++j)
        {
            index=i*w+j;
            rgb.rgbRed=(label[index]*17) % 255;
            rgb.rgbGreen=(label[index]*101) % 255;
            rgb.rgbBlue=(label[index]*153) % 255;
            image.SetPixelColor(j, i, rgb);
        }
    }
    image.Save("GMM.jpg", CXIMAGE_FORMAT_JPG);

    delete [] data;
    delete [] label;
}
