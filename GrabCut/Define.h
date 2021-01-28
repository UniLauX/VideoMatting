#ifndef DEFINE_H
#define DEFINE_H

#include <Wml/WmlMathLib.h>

typedef struct Cluster
{
	
	int				   clusterNum;
	double			   (*mean)[3];
	Wml::Matrix3<double> * invCov;
	double *			   det;
	double *			   weight;
	
	Cluster(int num=5)
	{
		clusterNum=num;
		mean=new double[num][3];
		invCov=new Wml::Matrix3<double>[num];
		det=new double[num];
		weight=new double[num];
	}
	
	~Cluster()
	{
		delete [] mean;
		delete [] invCov;
		delete [] det;
		delete [] weight;
	}
	
}CLUSTER;

#endif