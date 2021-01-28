#include "FlowCalculator_LiuCe.h"
#include "../mexOpticalFlow/mex/OpticalFlow.h"
#include "colorcode.h"
#include <atlstr.h>
FlowCalculator_LiuCe::FlowCalculator_LiuCe(void)
{
}

FlowCalculator_LiuCe::~FlowCalculator_LiuCe(void)
{
}
void FlowCalculator_LiuCe::OpticalFlow(CxImage& pFrm, CxImage& pRfFrm, float*u, float* v,double alpha , double Rt, int MnWd , int ItrNm_OtFwPDE, int ItrNm_InFwPDE, int ItrNm_CG)
{
	//printf("Optical Flow: Frame %d - %d\n", pFrm->frameno, pRfFrm->frameno);

	int iWidth = pFrm.GetWidth();
	int iHeight = pFrm.GetHeight();

	Wml::GMatrix<Wml::Vector3f> ImgRGB(iWidth, iHeight);
	Wml::GMatrix<bool> VsbMp(iWidth, iHeight);

	//CxImage Img;
	//Img.Load(pFrm->ImgFileName().c_str());
	DImage Im1(iWidth, iHeight, 3);
	double* pImgDt = Im1.data();
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			RGBQUAD c = pFrm.GetPixelColor(x, iHeight - 1 - y);
			pImgDt[y * iWidth * 3 + x * 3] = double(c.rgbRed) / 255.0;
			pImgDt[y * iWidth * 3 + x * 3 + 1] = double(c.rgbGreen) / 255.0;
			pImgDt[y * iWidth * 3 + x * 3 + 2] = double(c.rgbBlue) / 255.0;
		}
	}

	
	DImage Im2(iWidth, iHeight, 3);
	pImgDt = Im2.data();
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			RGBQUAD c = pRfFrm.GetPixelColor(x, iHeight - 1 - y);
			pImgDt[y * iWidth * 3 + x * 3] = double(c.rgbRed) / 255.0;
			pImgDt[y * iWidth * 3 + x * 3 + 1] = double(c.rgbGreen) / 255.0;
			pImgDt[y * iWidth * 3 + x * 3 + 2] = double(c.rgbBlue) / 255.0;
		}
	}

	DImage V_x, V_y, WrpI2;
	OpticalFlow::Coarse2FineFlow(V_x, V_y, WrpI2, Im1, Im2, alpha, Rt, MnWd, ItrNm_OtFwPDE, ItrNm_InFwPDE, ItrNm_CG);

	//Wml::GMatrix<Wml::Vector2d> OpFwMp(iWidth, iHeight);
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			double v_x = V_x.data()[y * iWidth + x];
			double v_y = V_y.data()[y * iWidth + x];
			//OpFwMp(x, y) = Wml::Vector2d(v_x, v_y);
			u[(iHeight-1-y)*iWidth+x]=v_x;
			v[(iHeight-1-y)*iWidth+x]=v_y;
		}
	}

	////////////////////////////////////////////////////////////////////////////
	////testing
	static int count=0;
	{
		CxImage OpFwImg(iWidth, iHeight, 24);
		for(int y = 0; y < iHeight; y++)
		{
			for(int x = 0; x < iWidth; x++)
			{
				Wml::Vector3<uchar> color;
				computeColor(u[y*iWidth+x],v[y*iWidth+x], &color[0]);
				OpFwImg.SetPixelColor(x, y, RGB(color[0], color[1], color[2]));
			}
		}
		CString OpFwImgNm = "H:/test/";
		OpFwImgNm.AppendFormat("flow_%d.png", count);
		count++;
		OpFwImg.Save((LPCTSTR)OpFwImgNm, CXIMAGE_FORMAT_PNG);
	}
	////////////////////////////////////////////////////////////////////////////

	//return OpFwMp;


}
