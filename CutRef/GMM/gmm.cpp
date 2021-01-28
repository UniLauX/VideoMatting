#include "GMM.h"
#include <math.h>
#include <string.h>
#include "../include/opencv/cv.h"

#include <time.h>
#include "../include/CxImage/ximage.h"
#include <iostream>

using namespace std;

#define EPSILON 1e-10
#define TESTNAME "test/test10.jpg"

int GMM_ywz::times=5;

void Inverse_ywz(const double mat[], double result[])
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

double Prob_ywz(double *data, double *mean, double det, double *segma)
{
    double a[3];
    double b[3];
    double c;
    double inv[9];

    Inverse_ywz(segma, inv);
    a[0]=data[0]-mean[0];
    a[1]=data[1]-mean[1];
    a[2]=data[2]-mean[2];
    b[0]=a[0]*inv[0]+a[1]*inv[3]+a[2]*inv[6];
    b[1]=a[0]*inv[1]+a[1]*inv[4]+a[2]*inv[7];
    b[2]=a[0]*inv[2]+a[1]*inv[5]+a[2]*inv[8];

    c=a[0]*b[0]+a[1]*b[1]+a[2]*b[2];

    return exp(-0.5*c)/sqrt(fabs(det)+EPSILON);
}

float Cluster_ywz::Prob_ywz(double data[3])
{
    double a,b,c;

    a=::Prob_ywz(data, mean[0], det[0], cov[0]);
    b=::Prob_ywz(data, mean[1], det[1], cov[1]);
    c=::Prob_ywz(data, mean[2], det[2], cov[2]);

    return (float)(weight[0]*a+weight[1]*b+weight[2]*c);
}

void Cluster_ywz::Print()
{
    GMM_ywz::Print(clusterNum, mean, weight, det, cov);
}

void GMM_ywz::GetGMM(double data[], int dataNum, int clusterNum, 
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

void GMM_ywz::InitGMM(double data[], int dataNum, int clusterNum, 
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

void GMM_ywz::GetLabel(double data[], int dataNum, int clusterNum, 
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
            tmp=Prob_ywz(&data[i*3], mean[k], det[k], segma[k]);
            if(tmp<Max)
            {
                Max=tmp;
                label[i]=k;
            }
        }
    }
}

void GMM_ywz::NewGMM(double data[], int dataNum, int clusterNum, 
                 double mean[][3], double pi[], double det[], double *segma[9],
                 int label[])
{
    int i,k;
    int idx;
    double a,b,c;
    double T(1);
    int *num;

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

void GMM_ywz::Train(double data[], int n, Cluster_ywz &gmm)
{
    InitGMM(data, n, gmm.clusterNum, gmm.mean, gmm.weight, gmm.det, gmm.cov);
    GetGMM(data, n, gmm.clusterNum, gmm.mean, gmm.weight, gmm.det, gmm.cov);
}

/************************************************************************/
/* Test                                                                 */
/************************************************************************/
void GMM_ywz::Print(int clusterNum, double mean[][3], double pi[], 
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

        Inverse_ywz(segma[k], inv);
        cout << "Inv: " << endl;
        cout << inv[0] << " " << inv[1] << " " << inv[2] << endl;
        cout << inv[3] << " " << inv[4] << " " << inv[5] << endl;
        cout << inv[6] << " " << inv[7] << " " << inv[8] << endl;

        cout << "----------------------------------" << endl;
    }
}

void GMM_ywz::TestGMM()
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
    image.Save("test/GMM.jpg", CXIMAGE_FORMAT_BMP);

    delete [] data;
    delete [] label;
}
