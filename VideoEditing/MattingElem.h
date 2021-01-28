#pragma once
#include <cximage.h>
class MattingElem
{
public:
	MattingElem(void);
	MattingElem(int _startx, int _starty, int _width, int _height)
		:startx(_startx),starty(_starty), width(_width), height(_height)
	{
		//alpha = new double[width*height];
		colorImage = new CxImage;
		trimapImage = new CxImage;
		colorImage->Create(width, height, 24);
		trimapImage->Create(width, height, 24);
	};
	~MattingElem(void){delete[] alpha; delete colorImage; delete trimapImage;};

	int GetX()
	{
		return startx;
	};
	int GetY()
	{
		return starty;
	};
	int GetWidth()
	{
		return width;
	};
	int GetHeight()
	{
		return height;
	};
	CxImage* GetImage()
	{
		return colorImage;
	}
	CxImage* GetTrimap()
	{
		return trimapImage;
	}

	CxImage* colorImage;
	CxImage* trimapImage;
	double* alpha;

private:

	int startx;
	int starty;
	int width;
	int height;

};
