#include "Matting.h"

void Running_Teddy(Matting &matting)
{
	matting.SetImage("Data/7.bmp", "Data/7_s.bmp");
	matting.SetLevelNum(2);
	matting.SetActiveLevelNum(2);
}

void Running_Fire(Matting &matting)
{
	matting.SetImage("Data/6.bmp", "Data/6_s.bmp");
	matting.SetLevelNum(2);
	matting.SetActiveLevelNum(1);
	matting.SetErodeWinSize(1);
}

void Running_Peacock(Matting &matting)
{
	matting.SetImage("Data/3.bmp", "Data/3_s.bmp");
	matting.SetLevelNum(4);
	matting.SetActiveLevelNum(2);
	matting.SetErodeWinSize(1);
	matting.SetThrAlpha(0.025);
}

void Running_Rabbit(Matting &matting)
{
	matting.SetImage("Data/9.bmp", "Data/9_s.bmp");
	matting.SetLevelNum(4);
	matting.SetActiveLevelNum(4);
	matting.SetErodeWinSize(1);
	matting.SetThrAlpha(0.12);
	matting.SetEpsilon(0.00001);
}

int main()
{
	Matting matting;

	Running_Rabbit(matting);
	matting.TestSolveAlpha("Out.bmp");


/*	CxImage image,I;
	image.Load("alpha_after.bmp");
//	(matting.DownSmpIm(image, 2)).Save("Test.bmp", CXIMAGE_FORMAT_BMP);

	I.Load("alpha_before.bmp");
//	(matting.UpSmpIm(I, image.GetWidth(), image.GetHeight(), 2)).Save("Test_up.bmp", CXIMAGE_FORMAT_BMP);
	matting.Test(I, image.GetWidth(), image.GetHeight(), "Up_Mat_Test.bmp");
*/
/*
	CxImage image;
	image.Load("Out.bmp");
	matting.TestErodeAlpha(image, "ErodeTest.bmp");
*/

	return 0;
}





















