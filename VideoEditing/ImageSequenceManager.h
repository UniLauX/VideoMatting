#ifndef IMAGESEQUENCEMANAGER_H
#define IMAGESEQUENCEMANAGER_H

class CxImage;
class CVideoEditingView;

#include "ImageBuffer.h"


class ImageSequenceManager
{
public:
	ImageSequenceManager();
	~ImageSequenceManager();
	void setStartIndex(int index);
	void setMaxnum();
	ImageBuffer * getImageBuffer(int framePos);
	void loadImage(int index, int num);
	void setPathName(CString& pathname);
	void setExtName(CString& ext);

private:
	int maxnum;
	int start;
	int end;
	int stFrameIndex; //index of start frame
	int enFrameIndex; //index + 1 of end frame

	//int transsub;
	ImageBuffer ** ImageSequence;
	

public:
	CString pathname;
	CString extName;
	int startVedioIndex;
};
#endif