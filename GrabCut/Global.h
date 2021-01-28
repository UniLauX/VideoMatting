#ifndef GLOBAL_H
#define GLOBAL_H

#include "../Wml/WmlMath.h"
#include "../include/CxImage/ximage.h"
#include "../include/Graphcut/graph.h"

typedef struct Cluster
{

     int				   clusterNum;
     double			       (*mean)[3];
     double **             cov;
     double *			   det;
     double *			   weight;

     Cluster(int num=5)
     {
		 clusterNum=num;
		 mean=new double[num][3];
		 det=new double[num];
		 weight=new double[num];
		 cov=new double *[num];
		 for(int i=0;i<num;++i)
			 cov[i]=new double[9];
     }

     ~Cluster()
     {
          delete [] mean;
          delete [] det;
          delete [] weight;
          for(int i=0;i<clusterNum;++i)
               delete [] cov[i];
          delete [] cov;
     }

}CLUSTER;

void    Inverse(const double mat[], double result[]);
double  Determinant(double mat[]);

inline void GetData(CxImage &image, int x, int y, double res[])
{
    RGBQUAD rgb;
    rgb=image.GetPixelColor(x, y);
    res[0]=rgb.rgbRed;
    res[1]=rgb.rgbGreen;
    res[2]=rgb.rgbBlue;
}

#endif