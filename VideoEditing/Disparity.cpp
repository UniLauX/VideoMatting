#include "StdAfx.h"
#include "Disparity.h"

CDisparity::CDisparity(/*CDocument* pDoc*/void)
//: CAlgorithm(pDoc)
{
}

CDisparity::~CDisparity(void)
{
}

ZFloatImage CDisparity::LoadDspMap(int iWidth, int iHeight, const CString& DspFileName)
{
	FILE* pFile = fopen((LPCTSTR)DspFileName, "rb");
	if(pFile)
	{
		ZFloatImage DspMap(iWidth, iHeight);
		fread(DspMap.GetMap(), sizeof(BYTE), DspMap.GetSize(), pFile);
		fclose(pFile);
		return DspMap;
	}
	else
		return ZFloatImage();
}

bool CDisparity::SaveDspMap(ZFloatImage& DspMap, const CString& DspFileName)
{
	FILE* pFile = fopen((LPCTSTR)DspFileName, "wb+");
	if(pFile)
	{
		fwrite(DspMap.GetMap(), sizeof(BYTE), DspMap.GetSize(), pFile);
		fclose(pFile);
		return true;
	}
	else
		return false;
}

ZByteImage CDisparity::DspMap2ByteImg(ZFloatImage& DspMap, float fDspMin, float fDspMax)
{
	int iWidth = DspMap.GetWidth();
	int iHeight = DspMap.GetHeight();

	ZByteImage DspImg;
	DspImg.Create(iWidth, iHeight);
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			float DspVal = DspMap.at(x, y);
			DspVal = (std::min)((std::max)(DspVal, fDspMin), fDspMax);

			int GryVal = int((DspVal - fDspMin) / (fDspMax - fDspMin) * 255 + 0.5);
			DspImg.at(x, y) = (std::max)(0, (std::min)(255, GryVal));
		}
	}

	return DspImg;
}

CxImage CDisparity::DspMap2CxImg(ZFloatImage& DspMap, float fDspMin, float fDspMax)
{
	int iWidth = DspMap.GetWidth();
	int iHeight = DspMap.GetHeight();

	CxImage DspXImg;
	DspXImg.Create(iWidth, iHeight, 8);
	DspXImg.SetGrayPalette();
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			float DspVal = DspMap.at(x, y);
			DspVal = (std::min)((std::max)(DspVal, fDspMin), fDspMax);

			int GryVal = int((DspVal - fDspMin) / (fDspMax - fDspMin) * 255 + 0.5);
			DspXImg.SetPixelIndex(x, iHeight - 1 - y, (std::max)(0, (std::min)(255, GryVal)));
		}
	}

	return DspXImg;
}

ZFloatImage CDisparity::ByteImg2DspMap(ZByteImage& DspImg, float fDspMin, float fDspMax)
{
	int iWidth = DspImg.GetWidth();
	int iHeight = DspImg.GetHeight();

	ZFloatImage DspMap;
	DspMap.Create(iWidth, iHeight);
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			DspMap.at(x, y) = (fDspMax - fDspMin) * float(DspImg.at(x, y)) / 255 + fDspMin;
		}
	}

	return DspMap;
}

ZFloatImage CDisparity::CxImg2DspMap(CxImage& DspXImg, float fDspMin, float fDspMax)
{
	int iWidth = DspXImg.GetWidth();
	int iHeight = DspXImg.GetHeight();

	ZFloatImage DspMap;
	DspMap.Create(iWidth, iHeight);
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			RGBQUAD color = DspXImg.GetPixelColor(x, iHeight - 1 - y);
			DspMap.at(x, y) = (fDspMax - fDspMin) * float(color.rgbRed) / 255 + fDspMin;
		}
	}

	return DspMap;
}

//bool CMorphology::Execute(void)
//{
//}