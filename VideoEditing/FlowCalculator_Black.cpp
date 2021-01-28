#include "FlowCalculator_Black.h"
extern "C"
#include "../Opticalflow/calflow.h"

FlowCalculator_Black::FlowCalculator_Black()
{
	level = 4;
}
FlowCalculator_Black::~FlowCalculator_Black(void)
{
}

void FlowCalculator_Black::CalOpticalFlow(CxImage* fistimg, CxImage* secimg, float* u, float* v)
{
	float* bf1;
	float* bf2;
	int w,h;
	w=fistimg->GetWidth();
	h=fistimg->GetHeight();

	bf1=(float*)malloc(sizeof(float)*w*h);
	bf2=(float*)malloc(sizeof(float)*w*h);

	int count=0;
	for (int j=0;j<h;++j)
	{
		for (int i=0;i<w;++i)
		{
			BYTE temp1=fistimg->GetPixelGray(i,j);
			BYTE temp2=secimg->GetPixelGray(i,j);
			bf1[count]=temp1;
			bf2[count]=temp2;
			count++;
		}
	}
	calFlow(bf1,bf2,u,v,0,1,level,1,w,h);
	free(bf1);
	free(bf2);
}

