#ifndef PARAM_H
#define PARAM_H

#include "../gmm/gmm.h"
#include "../include/CxImage/ximage.h"

double GetProb_GMM(int x, int y, Cluster_ywz *cluster, CxImage *image);
void   GetData_ywz(CxImage &image, int x, int y, double res[]);

#endif