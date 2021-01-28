#pragma once
#include <cximage.h>
#include "ImageBuffer.h"
class FlowCalculator_Black
{
public:
	FlowCalculator_Black();
	~FlowCalculator_Black(void);
	void CalOpticalFlow(CxImage* fistimg, CxImage* secimg, float* u, float* v);
	void SetLevel(int _level){level = _level;}
	static const int minwidth = 200;

private:
	CxImage* currImg;
	CxImage* nextImg;
	ImageBuffer* imgbuffer;
	int level;
};
