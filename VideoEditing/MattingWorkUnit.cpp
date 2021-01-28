#include "stdafx.h"
#include "MattingWorkUnit.h"
#include <vector>
#include "MattingElem.h"
#include "../ClosedFormMatting/Matting.h"
#include "MattingElemGenerator.h"
#include <iostream>
#include "ImageConvert.h"
#include "ClosedFormMatting.h"
//#include "SparseMatrix_ListType.h"
MattingWorkUnit::MattingWorkUnit(void)
{
	imgbuf = NULL;
	trimapbuf = NULL;
	alphabuf = NULL;
}

MattingWorkUnit::MattingWorkUnit(ImageBuffer* _imgbuf, ImageBuffer* _trimapbuf, ImageBuffer* _alphabuf, ImageBuffer* _resultbuf)
{
	imgbuf = _imgbuf;
	trimapbuf = _trimapbuf;
	alphabuf = _alphabuf;
	resultbuf = _resultbuf;
}
MattingWorkUnit::~MattingWorkUnit(void)
{
}

void MattingWorkUnit::MattingFrame(CxImage* image, CxImage* trimap, CxImage* result, CxImage* fgimg)
{
	std::vector<MattingElem*> vecmat;
	MattingElemGenerator maemge;
	std::cout<<"before generate matt elem"<<std::endl;
	maemge.GenerateMattingElem(vecmat, trimap, image);
	result->Clear();

	for (int i=0; i<vecmat.size(); ++i)
	{
		std::cout<<"emel: "<<i<<std::endl;
		int x = vecmat[i]->GetX();
		int y = vecmat[i]->GetY();
		int width = vecmat[i]->GetWidth();
		int height = vecmat[i]->GetHeight();

			/*trimap->DrawLine(x, x+width, y, y, RGB(255,0,0));
			trimap->DrawLine(x, x, y, y+height,RGB(255,0,0));
			trimap->DrawLine(x, x+width,  y+height, y+height, RGB(255,0,0) );
			trimap->DrawLine(x+width, x+width, y, y+height, RGB(255,0,0));

			image->DrawLine(x, x+width, y, y, RGB(255,0,0));
			image->DrawLine(x, x, y, y+height,RGB(255,0,0));
			image->DrawLine(x, x+width,  y+height, y+height, RGB(255,0,0) );
			image->DrawLine(x+width, x+width, y, y+height, RGB(255,0,0));
			CString path = "H:/test/";
			path.AppendFormat("%d.jpg", i);
			vecmat[i]->GetImage()->Save(path.GetBuffer(), CXIMAGE_FORMAT_JPG);
			path = "H:/test/";
			path.AppendFormat("%d.bmp", i);
			vecmat[i]->GetTrimap()->Save(path.GetBuffer(), CXIMAGE_FORMAT_BMP);*/
		Matting matt;
		matt.SetImage(vecmat[i]->GetImage(), vecmat[i]->GetTrimap());
		vecmat[i]->alpha = matt.DoMatting();
	}
	//image->Save("H:/test/image_temp.jpg", CXIMAGE_FORMAT_JPG);
	//trimap->Save("H:/test/image_trimap.bmp", CXIMAGE_FORMAT_BMP);
	for (int i=0; i<vecmat.size(); ++i)
	{
		int x = vecmat[i]->GetX();
		int y = vecmat[i]->GetY();
		int width = vecmat[i]->GetWidth();
		int height = vecmat[i]->GetHeight();
		for (int jtemp = 0; jtemp<height; ++jtemp)
		{
			for (int itemp = 0; itemp<width; ++itemp)
			{
				RGBQUAD rgb;
				rgb.rgbRed = rgb.rgbGreen = rgb.rgbBlue = (BYTE)(vecmat[i]->alpha[jtemp*width+itemp]*255);
				result->SetPixelColor(itemp+x, jtemp+y, rgb);
				RGBQUAD rgb2 = image->GetPixelColor(itemp+x, jtemp+y);
				rgb2.rgbRed *=vecmat[i]->alpha[jtemp*width+itemp];
				rgb2.rgbGreen *= vecmat[i]->alpha[jtemp*width+itemp];
				rgb2.rgbBlue *= vecmat[i]->alpha[jtemp*width+itemp];
				fgimg->SetPixelColor(itemp+x, jtemp+y, rgb2);
			}
		}
	}
	for (int i=0; i<vecmat.size(); ++i)
		delete vecmat[i];
}

void MattingWorkUnit::MattingSequence()
{
	std::cout<<"in mat sequence"<<std::endl;
	do 
	{
		std::cout<<"Frame: "<< imgbuf->GetFramePos()<<std::endl;
		CxImage* img;
		CxImage* trimap;
		img = imgbuf->GetImage();
		trimap = trimapbuf->GetImage();
		CxImage result;
		std::cout<<" before copy"<<std::endl;
		result.Copy(*trimap);
		CxImage fgimg;
		fgimg.Copy(*(imgbuf->GetImage()));
		fgimg.Clear();
		std::cout<<"before mattingFrame"<<std::endl;
		MattingFrame(img, trimap, &result, &fgimg);
		std::cout<<"after mattingFrame"<<std::endl;
		alphabuf->SetPos(imgbuf->GetFramePos());
		result.Save(alphabuf->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_PNG);
		fgimg.Save(resultbuf->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_JPG);
		std::cout<<alphabuf->GetFrameName()<<std::endl;
	} while (imgbuf->Forward()&&trimapbuf->Forward()&&resultbuf->Forward());
}



void MattingWorkUnit::SolveFB(CxImage& image, CxImage& alpha, CxImage& fImg, CxImage& bImg)
{
	ZFloatImage _src, _alpha, _fimg, _bimg;
	CxToZFloatImg(image, _src);
	CxToZFloatImg(alpha, _alpha);
	_fimg.CreateAndInit(_src.GetWidth(), _src.GetHeight(), 4);
	_bimg.CreateAndInit(_src.GetWidth(), _src.GetHeight(), 4);
	MattingAlgorithm::CClosedFormMatting cmfb;
	cmfb.SolveFB(_src, _alpha, _fimg, _bimg);
	FloatToCxImg(_fimg, fImg);
	FloatToCxImg(_bimg, bImg);
}