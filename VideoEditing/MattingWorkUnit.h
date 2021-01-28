#pragma once
#include "ImageBuffer.h"
#include <cximage.h>
#include "ZImage.h"

class MattingWorkUnit
{
public:
	MattingWorkUnit(void);
	MattingWorkUnit(ImageBuffer* _imgbuf, ImageBuffer* _trimapbuf, ImageBuffer* _alphabuf, ImageBuffer* resultbuf);
	~MattingWorkUnit(void);
	void MattingSequence();
	void MattingFrame(CxImage* image, CxImage* trimap, CxImage* result, CxImage* fgimg);
	void SolveFB(CxImage& image, CxImage& alpha, CxImage& fImg, CxImage& bImg);
private:
	ImageBuffer* imgbuf;
	ImageBuffer* trimapbuf;
	ImageBuffer* alphabuf;
	ImageBuffer* resultbuf;

};
