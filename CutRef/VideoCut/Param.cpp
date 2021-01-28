#include "Param.h"
#include "../GMM/gmm.h"
#include <fstream>

using namespace std;

void GetData_ywz(CxImage &image, int x, int y, double res[])
{
    RGBQUAD rgb;
    rgb=image.GetPixelColor(x, y);
    res[0]=rgb.rgbRed;
    res[1]=rgb.rgbGreen;
    res[2]=rgb.rgbBlue;
}

double GetProb_GMM(int x, int y, Cluster_ywz *cluster, CxImage *image)
{
     int k;
     double prob;
     double data[3];

     GetData_ywz(*image, x, y, data);
     for(prob=0,k=0;k<cluster->clusterNum;++k)
     {
          prob+=cluster->weight[k]*
                Prob_ywz(data, cluster->mean[k], cluster->det[k], cluster->cov[k]);

          // Test
        /*  if(!(prob>=0 && prob<=1e100))
          {
               out << "Prob:   " << prob << endl;
               out << "Data: " << endl;
               out << data[0] << " " << data[1] << " " << data[2] << endl;
               out << "Cluster_ywz " << k << endl;
               out << "Det:    " << cluster->det[k] << endl;
               out << "Mean:   " << cluster->mean[k][0] << " ";
               out << cluster->mean[k][1] << " ";
               out << cluster->mean[k][2] << endl;
               out << "Weight: " << cluster->weight[k] << endl;
               out << "Cov: " << endl;
               out << cluster->cov[k][0] << " " << cluster->cov[k][1] << " " << cluster->cov[k][2] << endl;
               out << cluster->cov[k][3] << " " << cluster->cov[k][4] << " " << cluster->cov[k][5] << endl;
               out << cluster->cov[k][6] << " " << cluster->cov[k][7] << " " << cluster->cov[k][8] << endl;
               out << "---------------------------------" << endl;
          }*/
     }
     return prob;
}
