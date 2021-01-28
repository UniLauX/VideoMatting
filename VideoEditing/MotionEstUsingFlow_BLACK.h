#pragma once
#include "../VideoCut/LocalClassifier.h"
class MotionEstUsingFlow_BLACK
{
public:
	MotionEstUsingFlow_BLACK(void);
	~MotionEstUsingFlow_BLACK(void);

	void PredictMotion(char* uflowfilename, char* vflowfilename, unsigned char* premask, 
		unsigned char* predictedmask, int width, int height, VVPOINTS &newCenters,
		const VVPOINTS &localWins,int winSize);

	void smoothLabel(unsigned char *label,unsigned char* newlabel,int width,int height);
	void CheckMemory();
};
