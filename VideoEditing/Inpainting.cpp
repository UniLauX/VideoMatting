#include "StdAfx.h"
#include "Inpainting.h"
#include "Morphology.h"

#include <vector>
#include <queue>

Wml::Vector2<int> CInpainting::m_NbOfSt[8]
 = {Wml::Vector2<int>(1, 0),
	Wml::Vector2<int>(1, 1),
	Wml::Vector2<int>(0, 1),
	Wml::Vector2<int>(-1, 1),
	Wml::Vector2<int>(-1, 0),
	Wml::Vector2<int>(-1, -1),
	Wml::Vector2<int>(0, -1),
	Wml::Vector2<int>(1, -1)};

CInpainting::CInpainting(void)
{
}

CInpainting::~CInpainting(void)
{
}

ZByteImage CInpainting::ExemplarBasedInpainting(ZByteImage& SrcImg, ZByteImage& IptMsk, int W)
{
	printf("Exemplar-based Inpainting...\n");

	int iWidth = SrcImg.GetWidth();
	int iHeight = SrcImg.GetHeight();

	Wml::GMatrix<Wml::Vector3<int> > ImgRGB(iWidth, iHeight);

	switch(SrcImg.GetChannel())
	{
	case 1:
		{
			for(int x = 0; x < iWidth; x++)
			{
				for(int y = 0; y < iHeight; y++)
				{
					unsigned char GryVal = SrcImg.GetPixel(x, y);
					ImgRGB(x, y) = Wml::Vector3<int>(GryVal, GryVal, GryVal);
				}
			}
		}
		break;
	case 3:

	case 4:
		{
			for(int x = 0; x < iWidth; x++)
			{
				for(int y = 0; y < iHeight; y++)
				{
					ImgRGB(x, y) = Wml::Vector3<int>(SrcImg.GetPixel(x, y, 0), SrcImg.GetPixel(x, y, 1), SrcImg.GetPixel(x, y, 2));
				}
			}
		}
		break;
	default:
		break;
	}

	Wml::GMatrix<Wml::Vector2f> IspMap(iWidth, iHeight);

	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			if(!IptMsk.at(x, y))
			{
				Wml::Vector3f ImgRGBGdtX;
				Wml::Vector3f ImgRGBGdtY;

				if(x > 0 && !IptMsk.at(x - 1, y) && x < iWidth - 1 && !IptMsk.at(x + 1, y))
				{
					ImgRGBGdtX[0] = float(ImgRGB(x + 1, y)[0] - ImgRGB(x - 1, y)[0]) / 2.0f;
					ImgRGBGdtX[1] = float(ImgRGB(x + 1, y)[1] - ImgRGB(x - 1, y)[1]) / 2.0f;
					ImgRGBGdtX[2] = float(ImgRGB(x + 1, y)[2] - ImgRGB(x - 1, y)[2]) / 2.0f;
				}
				else if(x > 0 && !IptMsk.at(x - 1, y))
				{
					ImgRGBGdtX[0] = ImgRGB(x, y)[0] - ImgRGB(x - 1, y)[0];
					ImgRGBGdtX[1] = ImgRGB(x, y)[1] - ImgRGB(x - 1, y)[1];
					ImgRGBGdtX[2] = ImgRGB(x, y)[2] - ImgRGB(x - 1, y)[2];
				}
				else if(x < iWidth - 1 && !IptMsk.at(x + 1, y))
				{
					ImgRGBGdtX[0] = ImgRGB(x + 1, y)[0] - ImgRGB(x, y)[0];
					ImgRGBGdtX[1] = ImgRGB(x + 1, y)[1] - ImgRGB(x, y)[1];
					ImgRGBGdtX[2] = ImgRGB(x + 1, y)[2] - ImgRGB(x, y)[2];
				}
				else
					ImgRGBGdtX = Wml::Vector3f(0.0f, 0.0f, 0.0f);

				if(y > 0 && !IptMsk.at(x, y - 1) && y < iHeight - 1 && !IptMsk.at(x, y + 1))
				{
					ImgRGBGdtY[0] = float(ImgRGB(x, y + 1)[0] - ImgRGB(x, y - 1)[0]) / 2.0f;
					ImgRGBGdtY[1] = float(ImgRGB(x, y + 1)[1] - ImgRGB(x, y - 1)[1]) / 2.0f;
					ImgRGBGdtY[2] = float(ImgRGB(x, y + 1)[2] - ImgRGB(x, y - 1)[2]) / 2.0f;
				}
				else if(y > 0 && !IptMsk.at(x, y - 1))
				{
					ImgRGBGdtY[0] = ImgRGB(x, y)[0] - ImgRGB(x, y - 1)[0];
					ImgRGBGdtY[1] = ImgRGB(x, y)[1] - ImgRGB(x, y - 1)[1];
					ImgRGBGdtY[2] = ImgRGB(x, y)[2] - ImgRGB(x, y - 1)[2];
				}
				else if(y < iHeight - 1 && !IptMsk.at(x, y + 1))
				{
					ImgRGBGdtY[0] = ImgRGB(x, y + 1)[0] - ImgRGB(x, y)[0];
					ImgRGBGdtY[1] = ImgRGB(x, y + 1)[1] - ImgRGB(x, y)[1];
					ImgRGBGdtY[2] = ImgRGB(x, y + 1)[2] - ImgRGB(x, y)[2];
				}
				else
					ImgRGBGdtY = Wml::Vector3f(0.0f, 0.0f, 0.0f);

				IspMap(x, y).Y() = (ImgRGBGdtX[0] + ImgRGBGdtX[1] + ImgRGBGdtX[2]) / 3.0f / 255.0f;
				IspMap(x, y).X() = - (ImgRGBGdtY[0] + ImgRGBGdtY[1] + ImgRGBGdtY[2]) / 3.0f / 255.0f;
			}
		}
	}

	Wml::GMatrixf CfdMap(iWidth, iHeight);
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			CfdMap(x, y) = !IptMsk.at(x, y);
		}
	}

	//Wml::GMatrixf DtMap(iWidth, iHeight);
	////memset(DtMap[0], - 1.0e-3, sizeof(float) * iWidth * iHeight);
	//for(int x = 0; x < iWidth; x++)
	//{
	//	for(int y = 0; y < iHeight; y++)
	//	{
	//		DtMap(x, y) = - 1.0e-3;
	//	}
	//}

	int iFilPtNum = 0;

	ZByteImage RegFilMap(iWidth, iHeight);
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			RegFilMap.at(x, y) = (IptMsk.at(x, y) != 0);
			iFilPtNum += (IptMsk.at(x, y) != 0);
		}
	}

	while(iFilPtNum)
	{
		printf("%d Pixels to Fill...\n", iFilPtNum);

		std::vector<Wml::Vector2<int> > FilFrt;

		for(int x = 0; x < iWidth; x++)
		{
			for(int y = 0; y < iHeight; y++)
			{
				if(!RegFilMap.at(x, y) && (x > 0 && RegFilMap.at(x - 1, y)
					|| x < iWidth - 1 && RegFilMap.at(x + 1, y)
					|| y > 0 && RegFilMap.at(x, y - 1)
					|| y < iHeight - 1 && RegFilMap.at(x, y + 1)
					|| x > 0 && y > 0 && RegFilMap.at(x - 1, y - 1)
					|| x < iWidth - 1 && y > 0 && RegFilMap.at(x + 1, y - 1)
					|| x > 0 && y < iHeight - 1 && RegFilMap.at(x - 1, y + 1)
					|| x < iWidth - 1 && y < iHeight - 1 && RegFilMap.at(x + 1, y + 1)))
					FilFrt.push_back(Wml::Vector2<int>(x, y));
			}
		}

		//std::vector<Wml::Vector2f> N;
		std::vector<float> DtMap(FilFrt.size());

		for(int i = 0; i < FilFrt.size(); i++)
		{
			//Wml::Vector2f Nrm;

			//if(FilFrt[i].X() == 0)
			//	Nrm.X() = RgFilMp.at(FilFrt[i].X() + 1, FilFrt[i].Y()) - RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y());
			//else if(FilFrt[i].X() == iWidth - 1)
			//	Nrm.X() = RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y()) - RgFilMp.at(FilFrt[i].X() - 1, FilFrt[i].Y());
			//else
			//	Nrm.X() = float(RgFilMp.at(FilFrt[i].X() + 1, FilFrt[i].Y()) - RgFilMp.at(FilFrt[i].X() - 1, FilFrt[i].Y())) / 2.0;

			//if(FilFrt[i].Y() == 0)
			//	Nrm.Y() = RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y() + 1) - RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y());
			//else if(FilFrt[i].Y() == iHeight - 1)
			//	Nrm.Y() = RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y()) - RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y() - 1);
			//else
			//	Nrm.Y() = float(RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y() + 1) - RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y() - 1)) / 2.0;

			//Nrm = - Nrm;
			//Nrm.Normalize();
			Wml::Vector2f Nrm = BoundaryNormal(FilFrt[i], RegFilMap);
			Nrm = - Nrm;

			//N.push_back(Nrm);
			//DtMap(FilFrt[i].X(), FilFrt[i].Y()) = fabs(IspMp(FilFrt[i].X(), FilFrt[i].Y()).X() * Nrm.X() + IspMp(FilFrt[i].X(), FilFrt[i].Y()).Y() * Nrm.Y()) + 1.0e-3;
			DtMap[i] = fabs(IspMap(FilFrt[i].X(), FilFrt[i].Y()).X() * Nrm.X() + IspMap(FilFrt[i].X(), FilFrt[i].Y()).Y() * Nrm.Y()) + 1.0e-3;
		}

		for(int i = 0; i < FilFrt.size(); i++)
		{
			Wml::Vector4<int> WndRct;
			WndRct[0] = (std::max)(- W, - FilFrt[i].X());
			WndRct[1] = (std::min)(W, iWidth - 1 - FilFrt[i].X());
			WndRct[2] = (std::max)(- W, - FilFrt[i].Y());
			WndRct[3] = (std::min)(W, iHeight - 1 - FilFrt[i].Y());

			float C = 0.0;
			for(int wx = WndRct[0]; wx <= WndRct[1]; wx++)
			{
				for(int wy = WndRct[2]; wy <= WndRct[3]; wy++)
				{
					if(!RegFilMap.at(FilFrt[i].X() + wx, FilFrt[i].Y() + wy))
						C += CfdMap(FilFrt[i].X() + wx, FilFrt[i].Y() + wy);
				}
			}
			C /= float((WndRct[1] - WndRct[0] + 1) * (WndRct[3] - WndRct[2] + 1));

			CfdMap(FilFrt[i].X(), FilFrt[i].Y()) = C;
		}
			
		Wml::Vector2<int> IptPt(-1, -1);
		float MxP = - FLT_MAX;
		for(int i = 0; i < FilFrt.size(); i++)
		{
			//float P = DtMap(FilFrt[i].X(), FilFrt[i].Y()) * CfdMp(FilFrt[i].X(), FilFrt[i].Y());
			float P = DtMap[i] * CfdMap(FilFrt[i].X(), FilFrt[i].Y());

			if(P > MxP)
			{
				MxP = P;
				IptPt = FilFrt[i];
			}
		}

		Wml::Vector4<int> WndRect;
		WndRect[0] = (std::max)(- W, - IptPt.X());
		WndRect[1] = (std::min)(W, iWidth - 1 - IptPt.X());
		WndRect[2] = (std::max)(- W, - IptPt.Y());
		WndRect[3] = (std::min)(W, iHeight - 1 - IptPt.Y());

		Wml::Vector2<int> EptPt(-1, -1);
		int MinErr = INT_MAX;

		for(int x = - WndRect[0]; x < iWidth - WndRect[1]; x++)
		{
			for(int y = - WndRect[2]; y < iHeight - WndRect[3]; y++)
			{
				bool bVldEpt = true;

				for(int wx = WndRect[0]; wx <= WndRect[1]; wx++)
				{
					for(int wy = WndRect[2]; wy <= WndRect[3]; wy++)
					{
						if(RegFilMap.at(x + wx, y + wy))
						{
							bVldEpt = false;
							break;
						}
					}

					if(!bVldEpt)
						break;
				}

				if(!bVldEpt)
					continue;

				int SumSqErr = 0;
				for(int wx = WndRect[0]; wx <= WndRect[1]; wx++)
				{
					for(int wy = WndRect[2]; wy <= WndRect[3]; wy++)
					{
						if(!RegFilMap.at(IptPt.X() + wx, IptPt.Y() + wy))
							SumSqErr += (ImgRGB(IptPt.X() + wx, IptPt.Y() + wy) - ImgRGB(x + wx, y + wy)).SquaredLength();
					}
				}

				if(SumSqErr < MinErr)
				{
					MinErr = SumSqErr;
					EptPt = Wml::Vector2<int>(x, y);
				}
			}
		}

		if(EptPt == Wml::Vector2<int>(-1, -1))
			return ZByteImage();

		for(int wx = WndRect[0]; wx <= WndRect[1]; wx++)
		{
			for(int wy = WndRect[2]; wy <= WndRect[3]; wy++)
			{
				if(RegFilMap.at(IptPt.X() + wx, IptPt.Y() + wy))
				{
					RegFilMap.at(IptPt.X() + wx, IptPt.Y() + wy) = 0;
					CfdMap(IptPt.X() + wx, IptPt.Y() + wy) = CfdMap(IptPt.X(), IptPt.Y());
					IspMap(IptPt.X() + wx, IptPt.Y() + wy) = IspMap(EptPt.X() + wx, EptPt.Y() + wy);
					ImgRGB(IptPt.X() + wx, IptPt.Y() + wy) = ImgRGB(EptPt.X() + wx, EptPt.Y() + wy);
					iFilPtNum--;
				}
			}
		}
	}

	ZByteImage DstImg = SrcImg;

	switch(DstImg.GetChannel())
	{
	case 1:
		{
			for(int x = 0; x < iWidth; x++)
			{
				for(int y = 0; y < iHeight; y++)
				{
					if(IptMsk.at(x, y))
						DstImg.at(x, y) = ImgRGB(x, y)[0];
				}
			}
		}
		break;
	case 3:
		{
			for(int x = 0; x < iWidth; x++)
			{
				for(int y = 0; y < iHeight; y++)
				{
					if(IptMsk.at(x, y))
					{
						DstImg.at(x, y, 0) = ImgRGB(x, y)[0];
						DstImg.at(x, y, 1) = ImgRGB(x, y)[1];
						DstImg.at(x, y, 2) = ImgRGB(x, y)[2];
					}
				}
			}
		}
		break;
	case 4:
		{
			for(int x = 0; x < iWidth; x++)
			{
				for(int y = 0; y < iHeight; y++)
				{
					if(IptMsk.at(x, y))
					{
						DstImg.at(x, y, 0) = ImgRGB(x, y)[0];
						DstImg.at(x, y, 1) = ImgRGB(x, y)[1];
						DstImg.at(x, y, 2) = ImgRGB(x, y)[2];
						DstImg.at(x, y, 3) = 255;
					}
				}
			}
		}
		break;
	default:
		break;
	}

	return DstImg;
}

ZByteImage CInpainting::ExemplarBasedInpainting(ZByteImage& SrcImg, int W)
{
	VERIFY(SrcImg.GetChannel() == 4);

	int iWidth = SrcImg.GetWidth();
	int iHeight = SrcImg.GetHeight();

	ZByteImage IptMsk(iWidth, iHeight);
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			IptMsk.at(x, y) = SrcImg.GetPixel(x, y, 3) < 255;
		}
	}

	ZByteImage DstImg = ExemplarBasedInpainting(SrcImg, IptMsk, W);
	if(DstImg.GetMap() && DstImg.GetChannel() == 4)
	{
		for(int x = 0; x < iWidth; x++)
		{
			for(int y = 0; y < iHeight; y++)
			{
				DstImg.at(x, y, 3) = 255;
			}
		}
	}

	return DstImg;
}

ZFloatImage CInpainting::DepthInpainting(ZFloatImage& SrcDsp, ZIntImage& IptMsk)
{
	int iWidth = SrcDsp.GetWidth();
	int iHeight = SrcDsp.GetHeight();

	if(SrcDsp.GetMap() && IptMsk.GetMap() && iWidth == IptMsk.GetWidth() && iHeight == IptMsk.GetHeight())
	{
		ZFloatImage DstDsp = SrcDsp;

		ZByteImage RegFilMap(iWidth, iHeight);
		for(int x = 0; x < iWidth; x++)
		{
			for(int y = 0; y < iHeight; y++)
			{
				RegFilMap.at(x, y) = IptMsk.at(x, y) == 0;
			}
		}
		ZByteImage QuePtMsk = CMorphology::Dilation(RegFilMap, 1);

		std::queue<Wml::Vector2<int> > FilPtQue;
		for(int x = 0; x < iWidth; x++)
		{
			for(int y = 0; y < iHeight; y++)
			{
				if(QuePtMsk.at(x, y) && !RegFilMap.at(x, y))
				{
					if(IptMsk.at(x, y) > 0)
						FilPtQue.push(Wml::Vector2<int>(x, y));
					else
						QuePtMsk.at(x, y) = 0;
				}
			}
		}

		while(!FilPtQue.empty())
		{
			Wml::Vector2<int> P = FilPtQue.front();
			Wml::Vector2f BdNrm = BoundaryNormal(P, RegFilMap);
			float MinNrmErr = FLT_MAX;
			Wml::Vector2<int> NbPtFil(-1, -1);

			for(int i = 0; i < 8; i++)
			{
				Wml::Vector2<int> NbPt = P + m_NbOfSt[i];
				if(NbPt.X() >= 0 && NbPt.X() < iWidth && NbPt.Y() >= 0 && NbPt.Y() < iHeight)
				{
					if(RegFilMap.at(NbPt.X(), NbPt.Y()))
					{
						Wml::Vector2f Nrm(P.X() - NbPt.X(), P.Y() - NbPt.Y());
						Nrm.Normalize();
						float NrmErr = fabs(BdNrm.Kross(Nrm));
						if(NrmErr < MinNrmErr)
						{
							MinNrmErr = NrmErr;
							NbPtFil = NbPt;
						}
					}
					else
					{
						if(IptMsk.at(NbPt.X(), NbPt.Y()) > 0 && !QuePtMsk.at(NbPt.X(), NbPt.Y()))
						{
							QuePtMsk.at(NbPt.X(), NbPt.Y()) = 1;
							FilPtQue.push(NbPt);
						}
					}
				}
			}

			if(NbPtFil != Wml::Vector2<int>(-1, -1))
			{
				DstDsp.at(P.X(), P.Y()) = DstDsp.at(NbPtFil.X(), NbPtFil.Y());
				RegFilMap.at(P.X(), P.Y()) = 1;
			}

			FilPtQue.pop();
		}

		return DstDsp;
	}
	else
		return ZFloatImage();
}

ZFloatImage CInpainting::OptFlowInpainting(ZFloatImage& SrcOptFlow, ZIntImage& IptMsk)
{
	int iWidth = SrcOptFlow.GetWidth();
	int iHeight = SrcOptFlow.GetHeight();

	if(SrcOptFlow.GetMap() && SrcOptFlow.GetChannel() == 2 && IptMsk.GetMap() && iWidth == IptMsk.GetWidth() && iHeight == IptMsk.GetHeight())
	{
		ZFloatImage DstOptFlow = SrcOptFlow;

		ZByteImage RegFilMap(iWidth, iHeight);
		for(int x = 0; x < iWidth; x++)
		{
			for(int y = 0; y < iHeight; y++)
			{
				RegFilMap.at(x, y) = IptMsk.at(x, y) == 0;
			}
		}
		ZByteImage QuePtMsk = CMorphology::Dilation(RegFilMap, 1);

		std::queue<Wml::Vector2<int> > FilPtQue;
		for(int x = 0; x < iWidth; x++)
		{
			for(int y = 0; y < iHeight; y++)
			{
				if(QuePtMsk.at(x, y) && !RegFilMap.at(x, y))
				{
					if(IptMsk.at(x, y) > 0)
						FilPtQue.push(Wml::Vector2<int>(x, y));
					else
						QuePtMsk.at(x, y) = 0;
				}
			}
		}

		while(!FilPtQue.empty())
		{
			Wml::Vector2<int> P = FilPtQue.front();
			Wml::Vector2f BdNrm = BoundaryNormal(P, RegFilMap);
			float MinNrmErr = FLT_MAX;
			Wml::Vector2<int> NbPtFil(-1, -1);

			for(int i = 0; i < 8; i++)
			{
				Wml::Vector2<int> NbPt = P + m_NbOfSt[i];
				if(NbPt.X() >= 0 && NbPt.X() < iWidth && NbPt.Y() >= 0 && NbPt.Y() < iHeight)
				{
					if(RegFilMap.at(NbPt.X(), NbPt.Y()))
					{
						Wml::Vector2f Nrm(P.X() - NbPt.X(), P.Y() - NbPt.Y());
						Nrm.Normalize();
						float NrmErr = fabs(BdNrm.Kross(Nrm));
						if(NrmErr < MinNrmErr)
						{
							MinNrmErr = NrmErr;
							NbPtFil = NbPt;
						}
					}
					else
					{
						if(IptMsk.at(NbPt.X(), NbPt.Y()) > 0 && !QuePtMsk.at(NbPt.X(), NbPt.Y()))
						{
							QuePtMsk.at(NbPt.X(), NbPt.Y()) = 1;
							FilPtQue.push(NbPt);
						}
					}
				}
			}

			if(NbPtFil != Wml::Vector2<int>(-1, -1))
			{
				DstOptFlow.at(P.X(), P.Y(), 0) = DstOptFlow.at(NbPtFil.X(), NbPtFil.Y(), 0);
				DstOptFlow.at(P.X(), P.Y(), 1) = DstOptFlow.at(NbPtFil.X(), NbPtFil.Y(), 1);
				RegFilMap.at(P.X(), P.Y()) = 1;
			}

			FilPtQue.pop();
		}

		return DstOptFlow;
	}
	else
		return ZFloatImage();
}

Wml::Vector2f CInpainting::BoundaryNormal(const Wml::Vector2<int>& P, ZByteImage& RegFilMap)
{
	int iWidth = RegFilMap.GetWidth();
	int iHeight = RegFilMap.GetHeight();

	Wml::Vector2f Nrm;

	if(P.X() == 0)
		Nrm.X() = float(RegFilMap.at(P.X() + 1, P.Y())) - RegFilMap.at(P.X(), P.Y());
	else if(P.X() == iWidth - 1)
		Nrm.X() = float(RegFilMap.at(P.X(), P.Y())) - RegFilMap.at(P.X() - 1, P.Y());
	else
		Nrm.X() = (float(RegFilMap.at(P.X() + 1, P.Y())) - RegFilMap.at(P.X() - 1, P.Y())) / 2;

	if(P.Y() == 0)
		Nrm.Y() = float(RegFilMap.at(P.X(), P.Y() + 1)) - RegFilMap.at(P.X(), P.Y());
	else if(P.Y() == iHeight - 1)
		Nrm.Y() = float(RegFilMap.at(P.X(), P.Y())) - RegFilMap.at(P.X(), P.Y() - 1);
	else
		Nrm.Y() = (float(RegFilMap.at(P.X(), P.Y() + 1)) - RegFilMap.at(P.X(), P.Y() - 1)) / 2;

	Nrm.Normalize();

	return Nrm;
}

bool CInpainting::StereoInpainting(ZByteImage& DstImg, ZFloatImage& DstDsp, ZByteImage& SrcImg, ZFloatImage& SrcDsp, ZByteImage& IptMsk, float fDspMin, float fDspMax, int W, float fDspWt1, float fDspWt2)
{
	printf("Exemplar-based Inpainting...\n");

	int iWidth = SrcImg.GetWidth();
	int iHeight = SrcImg.GetHeight();

	Wml::GMatrix<Wml::Vector3<int> > ImgRGB(iWidth, iHeight);
	DstDsp = SrcDsp;

	switch(SrcImg.GetChannel())
	{
	case 1:
		{
			for(int x = 0; x < iWidth; x++)
			{
				for(int y = 0; y < iHeight; y++)
				{
					unsigned char GryVal = SrcImg.GetPixel(x, y);
					ImgRGB(x, y) = Wml::Vector3<int>(GryVal, GryVal, GryVal);
				}
			}
		}
		break;
	case 3:

	case 4:
		{
			for(int x = 0; x < iWidth; x++)
			{
				for(int y = 0; y < iHeight; y++)
				{
					ImgRGB(x, y) = Wml::Vector3<int>(SrcImg.GetPixel(x, y, 0), SrcImg.GetPixel(x, y, 1), SrcImg.GetPixel(x, y, 2));
				}
			}
		}
		break;
	default:
		break;
	}

	Wml::GMatrix<Wml::Vector2f> IspMap(iWidth, iHeight);

	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			if(!IptMsk.at(x, y))
			{
				Wml::Vector3f ImgRGBGdtX;
				Wml::Vector3f ImgRGBGdtY;
				Wml::Vector2f DspGdt;

				if(x > 0 && !IptMsk.at(x - 1, y) && x < iWidth - 1 && !IptMsk.at(x + 1, y))
				{
					ImgRGBGdtX[0] = float(ImgRGB(x + 1, y)[0] - ImgRGB(x - 1, y)[0]) / 2.0f;
					ImgRGBGdtX[1] = float(ImgRGB(x + 1, y)[1] - ImgRGB(x - 1, y)[1]) / 2.0f;
					ImgRGBGdtX[2] = float(ImgRGB(x + 1, y)[2] - ImgRGB(x - 1, y)[2]) / 2.0f;
					DspGdt.X() = float(SrcDsp.at(x + 1, y) - SrcDsp.at(x - 1, y)) / 2.0f;
				}
				else if(x > 0 && !IptMsk.at(x - 1, y))
				{
					ImgRGBGdtX[0] = ImgRGB(x, y)[0] - ImgRGB(x - 1, y)[0];
					ImgRGBGdtX[1] = ImgRGB(x, y)[1] - ImgRGB(x - 1, y)[1];
					ImgRGBGdtX[2] = ImgRGB(x, y)[2] - ImgRGB(x - 1, y)[2];
					DspGdt.X() = SrcDsp.at(x, y) - SrcDsp.at(x - 1, y);
				}
				else if(x < iWidth - 1 && !IptMsk.at(x + 1, y))
				{
					ImgRGBGdtX[0] = ImgRGB(x + 1, y)[0] - ImgRGB(x, y)[0];
					ImgRGBGdtX[1] = ImgRGB(x + 1, y)[1] - ImgRGB(x, y)[1];
					ImgRGBGdtX[2] = ImgRGB(x + 1, y)[2] - ImgRGB(x, y)[2];
					DspGdt.X() = SrcDsp.at(x + 1, y) - SrcDsp.at(x, y);
				}
				else
				{
					ImgRGBGdtX = Wml::Vector3f(0.0f, 0.0f, 0.0f);
					DspGdt.X() = 0.0f;
				}

				if(y > 0 && !IptMsk.at(x, y - 1) && y < iHeight - 1 && !IptMsk.at(x, y + 1))
				{
					ImgRGBGdtY[0] = float(ImgRGB(x, y + 1)[0] - ImgRGB(x, y - 1)[0]) / 2.0f;
					ImgRGBGdtY[1] = float(ImgRGB(x, y + 1)[1] - ImgRGB(x, y - 1)[1]) / 2.0f;
					ImgRGBGdtY[2] = float(ImgRGB(x, y + 1)[2] - ImgRGB(x, y - 1)[2]) / 2.0f;
					DspGdt.Y() = float(SrcDsp.at(x, y + 1) - SrcDsp.at(x, y - 1)) / 2.0f;
				}
				else if(y > 0 && !IptMsk.at(x, y - 1))
				{
					ImgRGBGdtY[0] = ImgRGB(x, y)[0] - ImgRGB(x, y - 1)[0];
					ImgRGBGdtY[1] = ImgRGB(x, y)[1] - ImgRGB(x, y - 1)[1];
					ImgRGBGdtY[2] = ImgRGB(x, y)[2] - ImgRGB(x, y - 1)[2];
					DspGdt.Y() = SrcDsp.at(x, y) - SrcDsp.at(x, y - 1);
				}
				else if(y < iHeight - 1 && !IptMsk.at(x, y + 1))
				{
					ImgRGBGdtY[0] = ImgRGB(x, y + 1)[0] - ImgRGB(x, y)[0];
					ImgRGBGdtY[1] = ImgRGB(x, y + 1)[1] - ImgRGB(x, y)[1];
					ImgRGBGdtY[2] = ImgRGB(x, y + 1)[2] - ImgRGB(x, y)[2];
					DspGdt.Y() = SrcDsp.at(x, y + 1) - SrcDsp.at(x, y);
				}
				else
				{
					ImgRGBGdtY = Wml::Vector3f(0.0f, 0.0f, 0.0f);
					DspGdt.Y() = 0.0f;
				}

				IspMap(x, y).Y() = (ImgRGBGdtX[0] + ImgRGBGdtX[1] + ImgRGBGdtX[2])
								 + fDspWt1 * DspGdt.X() / (fDspMax - fDspMin);
				IspMap(x, y).X() = - ((ImgRGBGdtY[0] + ImgRGBGdtY[1] + ImgRGBGdtY[2])
								 + fDspWt1 * DspGdt.Y() / (fDspMax - fDspMin));
			}
		}
	}

	Wml::GMatrixf CfdMap(iWidth, iHeight);
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			CfdMap(x, y) = !IptMsk.at(x, y);
		}
	}

	//Wml::GMatrixf DtMap(iWidth, iHeight);
	////memset(DtMap[0], - 1.0e-3, sizeof(float) * iWidth * iHeight);
	//for(int x = 0; x < iWidth; x++)
	//{
	//	for(int y = 0; y < iHeight; y++)
	//	{
	//		DtMap(x, y) = - 1.0e-3;
	//	}
	//}

	int iFilPtNum = 0;

	ZByteImage RegFilMap(iWidth, iHeight);
	for(int x = 0; x < iWidth; x++)
	{
		for(int y = 0; y < iHeight; y++)
		{
			RegFilMap.at(x, y) = (IptMsk.at(x, y) != 0);
			iFilPtNum += (IptMsk.at(x, y) != 0);
		}
	}

	while(iFilPtNum)
	{
		printf("%d pixels to fill...\n", iFilPtNum);

		std::vector<Wml::Vector2<int> > FilFrt;

		for(int x = 0; x < iWidth; x++)
		{
			for(int y = 0; y < iHeight; y++)
			{
				if(!RegFilMap.at(x, y) && (x > 0 && RegFilMap.at(x - 1, y)
					|| x < iWidth - 1 && RegFilMap.at(x + 1, y)
					|| y > 0 && RegFilMap.at(x, y - 1)
					|| y < iHeight - 1 && RegFilMap.at(x, y + 1)
					|| x > 0 && y > 0 && RegFilMap.at(x - 1, y - 1)
					|| x < iWidth - 1 && y > 0 && RegFilMap.at(x + 1, y - 1)
					|| x > 0 && y < iHeight - 1 && RegFilMap.at(x - 1, y + 1)
					|| x < iWidth - 1 && y < iHeight - 1 && RegFilMap.at(x + 1, y + 1)))
					FilFrt.push_back(Wml::Vector2<int>(x, y));
			}
		}

		//std::vector<Wml::Vector2f> N;
		std::vector<float> DtMap(FilFrt.size());

		for(int i = 0; i < FilFrt.size(); i++)
		{
			//Wml::Vector2f Nrm;

			//if(FilFrt[i].X() == 0)
			//	Nrm.X() = RgFilMp.at(FilFrt[i].X() + 1, FilFrt[i].Y()) - RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y());
			//else if(FilFrt[i].X() == iWidth - 1)
			//	Nrm.X() = RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y()) - RgFilMp.at(FilFrt[i].X() - 1, FilFrt[i].Y());
			//else
			//	Nrm.X() = float(RgFilMp.at(FilFrt[i].X() + 1, FilFrt[i].Y()) - RgFilMp.at(FilFrt[i].X() - 1, FilFrt[i].Y())) / 2.0;

			//if(FilFrt[i].Y() == 0)
			//	Nrm.Y() = RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y() + 1) - RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y());
			//else if(FilFrt[i].Y() == iHeight - 1)
			//	Nrm.Y() = RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y()) - RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y() - 1);
			//else
			//	Nrm.Y() = float(RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y() + 1) - RgFilMp.at(FilFrt[i].X(), FilFrt[i].Y() - 1)) / 2.0;

			//Nrm = - Nrm;
			//Nrm.Normalize();
			//N.push_back(Nrm);

			Wml::Vector2f Nrm = BoundaryNormal(FilFrt[i], RegFilMap);
			Nrm = - Nrm;

			//DtMap[i] = fabs(IspMap(FilFrt[i].X(), FilFrt[i].Y()).X() * Nrm.X() + IspMap(FilFrt[i].X(), FilFrt[i].Y()).Y() * Nrm.Y())
			//		 * exp((fDspMin - DstDsp.at(FilFrt[i].X(), FilFrt[i].Y())) / (fDspMax - fDspMin)) + 1.0e-3;
			DtMap[i] = fabs(IspMap(FilFrt[i].X(), FilFrt[i].Y()).X() * Nrm.X() + IspMap(FilFrt[i].X(), FilFrt[i].Y()).Y() * Nrm.Y()) + 1.0e-3;
		}

		for(int i = 0; i < FilFrt.size(); i++)
		{
			Wml::Vector4<int> WndRect;
			WndRect[0] = (std::max)(- W, - FilFrt[i].X());
			WndRect[1] = (std::min)(W, iWidth - 1 - FilFrt[i].X());
			WndRect[2] = (std::max)(- W, - FilFrt[i].Y());
			WndRect[3] = (std::min)(W, iHeight - 1 - FilFrt[i].Y());

			float C = 0.0;
			for(int wx = WndRect[0]; wx <= WndRect[1]; wx++)
			{
				for(int wy = WndRect[2]; wy <= WndRect[3]; wy++)
				{
					if(!RegFilMap.at(FilFrt[i].X() + wx, FilFrt[i].Y() + wy))
						C += CfdMap(FilFrt[i].X() + wx, FilFrt[i].Y() + wy);
				}
			}
			C /= float((WndRect[1] - WndRect[0] + 1) * (WndRect[3] - WndRect[2] + 1));

			CfdMap(FilFrt[i].X(), FilFrt[i].Y()) = C;
		}

		Wml::Vector2<int> IptPt(-1, -1);
		float MxP = - FLT_MAX;
		for(int i = 0; i < FilFrt.size(); i++)
		{
			//float P = DtMap(FilFrt[i].X(), FilFrt[i].Y()) * CfdMap(FilFrt[i].X(), FilFrt[i].Y());
			float P = DtMap[i] * CfdMap(FilFrt[i].X(), FilFrt[i].Y());

			if(P > MxP)
			{
				MxP = P;
				IptPt = FilFrt[i];
			}
		}

		Wml::Vector4<int> WndRect;
		WndRect[0] = (std::max)(- W, - IptPt.X());
		WndRect[1] = (std::min)(W, iWidth - 1 - IptPt.X());
		WndRect[2] = (std::max)(- W, - IptPt.Y());
		WndRect[3] = (std::min)(W, iHeight - 1 - IptPt.Y());

		Wml::Vector2<int> EptPt(-1, -1);
		float MinErr = FLT_MAX;

		for(int x = - WndRect[0]; x < iWidth - WndRect[1]; x++)
		{
			for(int y = - WndRect[2]; y < iHeight - WndRect[3]; y++)
			{
				bool bVldEpt = true;

				for(int wx = WndRect[0]; wx <= WndRect[1]; wx++)
				{
					for(int wy = WndRect[2]; wy <= WndRect[3]; wy++)
					{
						if(RegFilMap.at(x + wx, y + wy))
						{
							bVldEpt = false;
							break;
						}
					}

					if(!bVldEpt)
						break;
				}

				if(!bVldEpt)
					continue;

				float SumSqErr = 0.0;
				for(int wx = WndRect[0]; wx <= WndRect[1]; wx++)
				{
					for(int wy = WndRect[2]; wy <= WndRect[3]; wy++)
					{
						if(!RegFilMap.at(IptPt.X() + wx, IptPt.Y() + wy))
						{
							Wml::Vector3<int> RGBDif = ImgRGB(IptPt.X() + wx, IptPt.Y() + wy) - ImgRGB(x + wx, y + wy);
							SumSqErr += float(abs(RGBDif[0]) + abs(RGBDif[1]) + abs(RGBDif[2]));
							SumSqErr += fDspWt1 * fabs(DstDsp.at(IptPt.X() + wx, IptPt.Y() + wy) - DstDsp.at(x + wx, y + wy)) / (fDspMax - fDspMin);
							SumSqErr += fDspWt2 * exp((fDspMin - DstDsp.at(x + wx, y + wy)) / (fDspMax - fDspMin));
						}
					}
				}

				if(SumSqErr < MinErr)
				{
					MinErr = SumSqErr;
					EptPt = Wml::Vector2<int>(x, y);
				}
			}
		}

		if(EptPt == Wml::Vector2<int>(-1, -1))
			return false;

		for(int wx = WndRect[0]; wx <= WndRect[1]; wx++)
		{
			for(int wy = WndRect[2]; wy <= WndRect[3]; wy++)
			{
				if(RegFilMap.at(IptPt.X() + wx, IptPt.Y() + wy))
				{
					RegFilMap.at(IptPt.X() + wx, IptPt.Y() + wy) = 0;
					CfdMap(IptPt.X() + wx, IptPt.Y() + wy) = CfdMap(IptPt.X(), IptPt.Y());
					IspMap(IptPt.X() + wx, IptPt.Y() + wy) = IspMap(EptPt.X() + wx, EptPt.Y() + wy);
					ImgRGB(IptPt.X() + wx, IptPt.Y() + wy) = ImgRGB(EptPt.X() + wx, EptPt.Y() + wy);
					DstDsp.at(IptPt.X() + wx, IptPt.Y() + wy) = DstDsp.at(EptPt.X() + wx, EptPt.Y() + wy);
					iFilPtNum--;
				}
			}
		}
	}

	DstImg = SrcImg;

	switch(DstImg.GetChannel())
	{
	case 1:
		{
			for(int x = 0; x < iWidth; x++)
			{
				for(int y = 0; y < iHeight; y++)
				{
					if(IptMsk.at(x, y))
						DstImg.at(x, y) = ImgRGB(x, y)[0];
				}
			}
		}
		break;
	case 3:
		{
			for(int x = 0; x < iWidth; x++)
			{
				for(int y = 0; y < iHeight; y++)
				{
					if(IptMsk.at(x, y))
					{
						DstImg.at(x, y, 0) = ImgRGB(x, y)[0];
						DstImg.at(x, y, 1) = ImgRGB(x, y)[1];
						DstImg.at(x, y, 2) = ImgRGB(x, y)[2];
					}
				}
			}
		}
	case 4:
		{
			for(int x = 0; x < iWidth; x++)
			{
				for(int y = 0; y < iHeight; y++)
				{
					if(IptMsk.at(x, y))
					{
						DstImg.at(x, y, 0) = ImgRGB(x, y)[0];
						DstImg.at(x, y, 1) = ImgRGB(x, y)[1];
						DstImg.at(x, y, 2) = ImgRGB(x, y)[2];
						DstImg.at(x, y, 3) = 255;
					}
				}
			}
		}
		break;
	default:
		break;
	}

	return true;
}
