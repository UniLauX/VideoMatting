//#include "../VideoEditing/stdafx.h"
#include "GMM.h"
#include <iostream>
void initGMM(vector< vector<  colorNode  > > &vec,GMMNode node[COMNUM])
{
	int total=0;
	for (size_t k=0;k<vec.size();++k)
	{
		total+=(int)vec[k].size();
	}

	double sum[3];
	double avg[3];

	for (size_t i=0;i<vec.size();++i)
	{
		//dMatrix sum(3,1,0.0);
		//dMatrix avg(3,1,0.0);

		sum[0]=0;
		sum[1]=0;
		sum[2]=0;

		avg[0]=0;
		avg[1]=0;
		avg[2]=0;

		int num=0;

		for (size_t j=0;j<vec[i].size();++j)
		{
			sum[0]+=(vec[i][j].colorinfo)(0,0);
			sum[1]+=(vec[i][j].colorinfo)(1,0);
			sum[2]+=(vec[i][j].colorinfo)(2,0);
			//sum+=vec[i][j].colorinfo;

			num++;
		}


		node[i].count=vec[i].size();
		double invnum=1.0/num;
		//avg=sum*invnum;
		avg[0]=sum[0]*invnum;
		avg[1]=sum[1]*invnum;
		avg[2]=sum[2]*invnum;

		//node[i].u=avg;//mean color
		node[i].u.resize(3,1);
		node[i].u(0,0)=avg[0];
		node[i].u(1,0)=avg[1];
		node[i].u(2,0)=avg[2];

		double tempMax[3];
		dMatrix conviance(3,3,0.0);
		for (size_t j=0;j<vec[i].size();++j)
		{
			tempMax[0]=vec[i][j].colorinfo(0,0)-avg[0];
			tempMax[1]=vec[i][j].colorinfo(1,0)-avg[1];
			tempMax[2]=vec[i][j].colorinfo(2,0)-avg[2];

			conviance(0,0)+=tempMax[0]*tempMax[0];
			conviance(0,1)+=tempMax[0]*tempMax[1];
			conviance(0,2)+=tempMax[0]*tempMax[2];

			conviance(1,0)+=tempMax[1]*tempMax[0];
			conviance(1,1)+=tempMax[1]*tempMax[1];
			conviance(1,2)+=tempMax[1]*tempMax[2];

			conviance(2,0)+=tempMax[2]*tempMax[0];
			conviance(2,1)+=tempMax[2]*tempMax[1];
			conviance(2,2)+=tempMax[2]*tempMax[2];
			//conviance+=(vec[i][j].colorinfo-avg)*(~(vec[i][j].colorinfo-avg));
		}
		conviance*=invnum;
		node[i].inverseConviance=!conviance;//inverse of conviance
		node[i].det=conviance.det();//determinant of conviance
		node[i].pi=((double)num)/total;//weight
	}
}
void calGMMPara(GMMNode &gmm,vector<pixelNode> &vec,int total,CxImage &image)
{
	double sum[3];
	sum[0]=0;
	sum[1]=0;
	sum[2]=0;

	gmm.count=vec.size();
	if (vec.size()==0)
	{
		gmm.pi=0;
	}
	else
	{
		RGBQUAD rgb;
		double temp[3];

		for (int i=0;i<vec.size();++i)
		{
			
			rgb=image.GetPixelColor(vec[i].x,vec[i].y);
			sum[0]+=(double)rgb.rgbRed;
			sum[1]+=(double)rgb.rgbGreen;
			sum[2]+=(double)rgb.rgbBlue;

		}


		double inv=1.0/(double)vec.size();
		gmm.u(0,0)=sum[0]*inv;
		gmm.u(1,0)=sum[1]*inv;
		gmm.u(2,0)=sum[2]*inv;


		double inv2=1.0/total;
		gmm.pi=vec.size()*inv2;
		dMatrix conviance(3,3,0.0);
		for (int i=0;i<vec.size();i++)
		{
			rgb=image.GetPixelColor(vec[i].x,vec[i].y);
			
			temp[0]=(double)rgb.rgbRed;
			temp[1]=(double)rgb.rgbGreen;
			temp[2]=(double)rgb.rgbBlue;
           
			temp[0]=temp[0]-gmm.u(0,0);
			temp[1]=temp[1]-gmm.u(1,0);
			temp[2]=temp[2]-gmm.u(2,0);

			//conviance+=(temp-gmm.u)*(~(temp-gmm.u));

			conviance(0,0)+=temp[0]*temp[0];
			conviance(0,1)+=temp[0]*temp[1];
			conviance(0,2)+=temp[0]*temp[2];

			conviance(1,0)+=temp[1]*temp[0];
			conviance(1,1)+=temp[1]*temp[1];
			conviance(1,2)+=temp[1]*temp[2];

			conviance(2,0)+=temp[2]*temp[0];
			conviance(2,1)+=temp[2]*temp[1];
			conviance(2,2)+=temp[2]*temp[2];

		}
		conviance*=inv;
		gmm.det=conviance.det();
		gmm.inverseConviance=!conviance;

	}

}