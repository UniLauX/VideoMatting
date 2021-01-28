#include "stdafx.h"
#include "TemporalMattingManager.h"
#include <WmlMathLib.h>
#include "ZImage.h"
#include "ZCubeImage.h"
#include "CxImageFun.h"
#include "ImageConvert.h"
#include "ClosedFormMatting.h"
#include "MattingWorkUnit.h"
TemporalMattingManager::TemporalMattingManager(ImageBuffer* _srcbuf, ImageBuffer* _trimbuf, ImageBuffer* _alphabuf, ImageBuffer* _resbuf,ImageBuffer* _fgbuffer, ImageBuffer* _bgbuffer,std::string _optpath, float fTmpWeight)
{
	srcimgbuffer = _srcbuf;
	trimapbuffer = _trimbuf;	
	alphabuffer = _alphabuf;
	resbuf = _resbuf;
	FGBuffer = _fgbuffer;
	BGBuffer = _bgbuffer;	
	optfilepath = _optpath;
	m_fTmpWeight = fTmpWeight;
}

void TemporalMattingManager::PathMatting(int _startframe, int _endframe, int _step, bool bAnchorFirst, bool bAnchorEnd)
{
	srcimgbuffer->Goto(_startframe);
	m_iWidth = srcimgbuffer->GetImage()->GetWidth();
	m_iHeight = srcimgbuffer->GetImage()->GetHeight();
	pfm.Init(optfilepath,m_iWidth, m_iHeight);

	for (int i=_startframe; i/*+_step*/<_endframe;)
	{
		printf("Do matting from %d to %d\n", i, i+_step);
		int p = (i+_step<_endframe)?(i+_step):_endframe;
		int s = (p-_step>=_startframe)?(p-_step):_startframe;
		if(s==_startframe && bAnchorFirst == false)
			DoTemporalMatting(s, p);
		else
			DoTemporalMatting_Fix(s, p, true, p==_endframe && bAnchorEnd);
		//i += (_step+1)/2;
		//i+=_step;
		i=p;
	}
}
#include <iostream>
void TemporalMattingManager::DoTemporalMatting(int _startframe, int _endframe)
{
	FloatImgList srcImgList;
	ByteImgList triMapList;
	IntImgList csMapList;
	FloatImgList wMapList;
	ByteImgList flowErrList;
	printf("before generate trimap and srcimg\n");
	for (int index = _startframe; index<=_endframe; ++index)
	{
		srcimgbuffer->Goto(index);
		trimapbuffer->Goto(index);
		CxImage* image = srcimgbuffer->GetImage();
		int iWidth = image->GetWidth();
		int iHeight = image->GetHeight();
		int iChannel = 3;
		ZFloatImage* pSrcImg = new ZFloatImage(iWidth, iHeight,3);
		CxToZFloatImg(*image, *pSrcImg);
		srcImgList.push_back(pSrcImg);

		ZByteImage* ptriImg = new ZByteImage(iWidth, iHeight,3);
		CxToByteImg(*(trimapbuffer->GetImage()), *ptriImg, false);
		triMapList.push_back(ptriImg);		
	}

	printf("before generate flowerrlist\n");
	for (int index = _startframe; index<_endframe; ++index)
	{
		ZByteImage* flowerrim = CheckConfidence(index);
		flowErrList.push_back(flowerrim);
		CxImage flowerr;
		ByteToCxImg(*flowerrim, flowerr);
		std::string path = "H:/test/";
		char num[10];
		_itoa(index, num, 10);
		path += std::string("flow_")+std::string(num)+std::string(".png");
		flowerr.Save(path.c_str(), CXIMAGE_FORMAT_PNG);
	}
	printf("before preparetemporalinfo\n");
	PrepareTemporalInfo(_startframe, _endframe, srcImgList, csMapList, wMapList,flowErrList);
	for (int i=_startframe; i<_endframe; ++i)
		delete flowErrList[i-_startframe];

	MattingAlgorithm::CClosedFormMatting mattingEst;
	printf("before solve alpha multiframes\n");
	FloatImgList alphaImgList;
	alphaImgList.clear();
	mattingEst.SolveAlpha_MultiFrames(srcImgList, triMapList, csMapList, wMapList, alphaImgList);

	for (int i=_startframe; i<=_endframe; ++i)
	{
		delete srcImgList[i-_startframe];
		delete triMapList[i-_startframe];
		delete csMapList[i-_startframe];
		delete wMapList[i-_startframe];
	}

	std::cout<<"before save result"<<std::endl;
	std::cout<<_startframe<<"  "<<_endframe<<std::endl;
	for (int index = _startframe; index<=_endframe; ++index)
	{
		ZFloatImage* alphaImg = alphaImgList[index-_startframe];
		CxImage alphaim;
		FloatToCxImg(*alphaImg, alphaim);
		//char framenum[10]; 
		//_itoa(index,framenum,10);
		//std::string path = std::string("H:/")+ std::string(framenum)+std::string(".png");
		//alphaim.Save(path.c_str(), CXIMAGE_FORMAT_PNG);
		alphabuffer->SetPos(index);
		resbuf->SetPos(index);
		alphaim.Save(alphabuffer->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_PNG);
		CxImage fg;
		srcimgbuffer->Goto(index);
		ExtractForground(srcimgbuffer->GetImage(), &alphaim, &fg );
		MattingWorkUnit mwu;
		CxImage fim, bim;
		std::cout<<"before SolveFB"<<std::endl;
		mwu.SolveFB(*(srcimgbuffer->GetImage()), alphaim, fim, bim);
		std::cout<<"After SolveFB"<<std::endl;
		FGBuffer->SetPos(index);
		BGBuffer->SetPos(index);
		fim.Save(FGBuffer->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_PNG);
		bim.Save(BGBuffer->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_PNG);
		fg.SetJpegQuality(100);
		fg.Save(resbuf->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_JPG);
	}
	for (int i=_startframe; i<=_endframe; ++i)
		delete alphaImgList[i-_startframe];
}

void TemporalMattingManager::DoTemporalMatting_Fix(int _startframe, int _endframe, bool bAnchorFirst, bool bAnchorEnd)
{
	FloatImgList srcImgList;
	ByteImgList triMapList;
	IntImgList csMapList;
	FloatImgList wMapList;
	ByteImgList flowErrList;

	FloatImgList fixAlphaImgList;
	std::vector<int> fixImgList;

	printf("before generate trimap and srcimg\n");
	for (int index = _startframe; index<=_endframe; ++index)
	{
		srcimgbuffer->Goto(index);
		trimapbuffer->Goto(index);
		CxImage* image = srcimgbuffer->GetImage();
		int iWidth = image->GetWidth();
		int iHeight = image->GetHeight();
		int iChannel = 3;
		ZFloatImage* pSrcImg = new ZFloatImage(iWidth, iHeight,3);
		CxToZFloatImg(*image, *pSrcImg);
		srcImgList.push_back(pSrcImg);

		ZByteImage* ptriImg = new ZByteImage(iWidth, iHeight,3);
		CxToByteImg(*(trimapbuffer->GetImage()), *ptriImg, false);
		triMapList.push_back(ptriImg);		
	}

	printf("before generate flowerrlist\n");
	for (int index = _startframe; index<_endframe; ++index)
	{
		ZByteImage* flowerrim = CheckConfidence(index);
		flowErrList.push_back(flowerrim);
		CxImage flowerr;
		ByteToCxImg(*flowerrim, flowerr);
		std::string path = "H:/test/";
		char num[10];
		_itoa(index, num, 10);
		path += std::string("flow_")+std::string(num)+std::string(".png");
		flowerr.Save(path.c_str(), CXIMAGE_FORMAT_PNG);
	}
	printf("before preparetemporalinfo\n");
	PrepareTemporalInfo(_startframe, _endframe, srcImgList, csMapList, wMapList,flowErrList);
	for (int i=_startframe; i<_endframe; ++i)
		delete flowErrList[i-_startframe];

	printf("Prepare Soft Constraint\n");
	//Load the alpha image of first frame!!!
	if(bAnchorFirst || bAnchorEnd){
		CxImage cxFixAlphaImg;
		if(bAnchorFirst){
			alphabuffer->SetPos(_startframe);
			cxFixAlphaImg.Load(alphabuffer->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_PNG);
			if(cxFixAlphaImg.GetBits() != 0){
				ZFloatImage* pFixAlphaImg = new ZFloatImage;
				CxToZFloatImg(cxFixAlphaImg,*pFixAlphaImg);//Already Normalized to [0,1]
				fixAlphaImgList.push_back(pFixAlphaImg);
				fixImgList.push_back(0);
			}
		}
		if(bAnchorEnd){
			alphabuffer->SetPos(_endframe);
			cxFixAlphaImg.Load(alphabuffer->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_PNG);
			if(cxFixAlphaImg.GetBits() != 0){
				ZFloatImage* pFixAlphaImg = new ZFloatImage;
				CxToZFloatImg(cxFixAlphaImg,*pFixAlphaImg);//Already Normalized to [0,1]
				fixAlphaImgList.push_back(pFixAlphaImg);
				fixImgList.push_back(_endframe-_startframe);
			}
		}
	}
	
	MattingAlgorithm::CClosedFormMatting mattingEst;
	printf("before solve alpha multiframes\n");
	FloatImgList alphaImgList;
	alphaImgList.clear();
	mattingEst.SolveAlpha_MultiFrames_Fix(srcImgList, triMapList, csMapList, wMapList, alphaImgList, fixAlphaImgList, fixImgList);

	for (int i=_startframe; i<=_endframe; ++i)
	{
		delete srcImgList[i-_startframe];
		delete triMapList[i-_startframe];
		delete csMapList[i-_startframe];
		delete wMapList[i-_startframe];
	}

	std::cout<<"before save result"<<std::endl;
	std::cout<<_startframe<<"  "<<_endframe<<std::endl;
	for (int index = _startframe; index<=_endframe; ++index)
	{
		ZFloatImage* alphaImg = alphaImgList[index-_startframe];
		CxImage alphaim;
		FloatToCxImg(*alphaImg, alphaim);
		//char framenum[10]; 
		//_itoa(index,framenum,10);
		//std::string path = std::string("H:/")+ std::string(framenum)+std::string(".png");
		//alphaim.Save(path.c_str(), CXIMAGE_FORMAT_PNG);
		alphabuffer->SetPos(index);
		resbuf->SetPos(index);
		alphaim.Save(alphabuffer->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_PNG);
		CxImage fg;
		srcimgbuffer->Goto(index);
		ExtractForground(srcimgbuffer->GetImage(), &alphaim, &fg );
		MattingWorkUnit mwu;
		CxImage fim, bim;
		std::cout<<"before SolveFB"<<std::endl;
		mwu.SolveFB(*(srcimgbuffer->GetImage()), alphaim, fim, bim);
		std::cout<<"After SolveFB"<<std::endl;
		FGBuffer->SetPos(index);
		BGBuffer->SetPos(index);
		fim.Save(FGBuffer->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_PNG);
		bim.Save(BGBuffer->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_PNG);
		fg.SetJpegQuality(100);
		fg.Save(resbuf->GetFrameName().GetBuffer(), CXIMAGE_FORMAT_JPG);
	}
	for (int i=_startframe; i<=_endframe; ++i)
		delete alphaImgList[i-_startframe];

	for(int i=0; i<fixAlphaImgList.size(); ++i)
		delete fixAlphaImgList[i];
}

ZByteImage* TemporalMattingManager::CheckConfidence(int index)
{
	ZByteImage* ferrImg = new ZByteImage(m_iWidth, m_iHeight);
	float* cu = new float[m_iWidth*m_iHeight];//u offset of current frame
	float* cv = new float[m_iWidth*m_iHeight];// v offset of current frame
	memcpy(cu, pfm.Forwardflow(index)->u, sizeof(float)*m_iWidth*m_iHeight);
	memcpy(cv, pfm.Forwardflow(index)->v, sizeof(float)*m_iWidth*m_iHeight);
	float* nbu = new float[m_iWidth*m_iHeight];//backward u offset of next frame
	float* nbv = new float[m_iWidth*m_iHeight];//backward v offset of next frame
	memcpy(nbu, pfm.Backwardflow(index+1)->u, sizeof(float)*m_iWidth*m_iHeight);
	memcpy(nbv, pfm.Backwardflow(index+1)->v, sizeof(float)*m_iWidth*m_iHeight);

	int iCount = 0;
	for(int j=0; j<m_iHeight; ++j)
		for (int i=0; i<m_iWidth; ++i)
		{
			int x2 = i+ cu[(m_iHeight-1-j)*m_iWidth+i]+0.5;
			int y2 = j+ cv[(m_iHeight-1-j)*m_iWidth+i]+0.5;
			if (x2<0||x2>=m_iWidth-1||y2<0||y2>m_iHeight-1)
			{
				iCount++;
				x2 = min(m_iWidth-1, max(x2, 0));
				y2 = min(m_iHeight-1, max(y2, 0));
			}
			float dx, dy;
			dx = fabs(cu[(m_iHeight-1-j)*m_iWidth+i]+nbu[(m_iHeight-1-y2)*m_iWidth+x2]);
			dy = fabs(cv[(m_iHeight-1-j)*m_iWidth+i]+nbv[(m_iHeight-1-y2)*m_iWidth+x2]);

			float dist = dx*dx + dy*dy;
			if (dist<1e-10)
				ferrImg->at(i,j) = 0;
			else
				ferrImg->at(i,j) = min(200, sqrt(dx*dx+dy*dy)/0.1);
		}
		delete[] nbv;
		delete[] nbu;
		delete[] cv;
		delete[] cu;
		pfm.Clear();
		return ferrImg;
}
void TemporalMattingManager::PrepareTemporalInfo(int _istart, int _iend, FloatImgList& srcimg, 
												 IntImgList& csMapList, FloatImgList& wMapList, ByteImgList& ferrList)
{
	float* cu = new float[m_iWidth*m_iHeight];
	float* cv = new float[m_iWidth*m_iHeight];
	float* cbu = new float[m_iWidth*m_iHeight];
	float* cbv = new float[m_iWidth*m_iHeight];
	for (int f=_istart; f<=_iend; ++f)
	{
		printf("before load flow\n");

		bool forward = false;
		bool backward = false;
		if (pfm.Forwardflow(f)->u!=0)
		{
			forward = true;
			memcpy(cu, pfm.Forwardflow(f)->u, sizeof(float)*m_iWidth*m_iHeight);
			memcpy(cv, pfm.Forwardflow(f)->v, sizeof(float)*m_iWidth*m_iHeight);
		}
		printf("before load flow1\n");

		if (pfm.Backwardflow(f)->u!=0)
		{
			backward = true;
			memcpy(cbu, pfm.Backwardflow(f)->u, sizeof(float)*m_iWidth*m_iHeight);
			memcpy(cbv, pfm.Backwardflow(f)->v, sizeof(float)*m_iWidth*m_iHeight);
		}
		printf("before load flow2\n");

		ZByteImage* pCurrError;
		if (f!=_iend)
				 pCurrError= ferrList[f-_istart];
		ZByteImage* pPrevError = NULL;
		ZByteImage* pNextError = NULL;
		ZFloatImage* pSrcImg = srcimg[f-_istart];
		ZFloatImage* pPrevImg = NULL;
		ZFloatImage* pNextImg = NULL;
		printf("before load flow3\n");
		if (f-1>=_istart)
		{
			printf("%d\n", f-1-_istart);
			pPrevError = ferrList[f-1-_istart]; 
			pPrevImg = srcimg[f-1-_istart];
		}
		if (f+1<_iend)
		{
			printf("%d\n", f+1-_istart);
			pNextError = ferrList[f+1-_istart];
			pNextImg = srcimg[f+1-_istart];
		}
	
		ZIntImage* pCsMap = new ZIntImage(m_iWidth, m_iHeight, 4);
		ZFloatImage* pWMap = new ZFloatImage(m_iWidth, m_iHeight);
		printf("before backward\n");
		//m_fTmpWeight = 10000.0;
		//Backward:
		if(backward&&pPrevError){
		for (int j=0; j<m_iHeight; ++j)
		{
			for (int i=0; i<m_iWidth; ++i)
			{
				if (i<=1||i>=m_iWidth-1||j<=1||j>=m_iHeight-1)
				{
					pCsMap->at(i,j,0) = -1;
					pCsMap->at(i,j,1) = -1;
					pWMap->at(i,j) = 0;
					continue;
				}

				float x = cbu[(m_iHeight-1-j)*m_iWidth+i] + i;
				float y = cbv[(m_iHeight-1-j)*m_iWidth+i] + j;
				int ix = x+0.5;
				int iy = y+0.5;
				if (ix>=0&&ix<m_iWidth&&iy>=0&&iy<m_iHeight&&
					x>=0&&x<=m_iWidth-1&&y>=0&&y<=m_iHeight-1)
				{
					float disterror = pPrevError->at(ix, iy)*0.1;
					float colordist2 = (pSrcImg->at(i,j,0) - pPrevImg->at(ix, iy,0))*(pSrcImg->at(i,j,0) - pPrevImg->at(ix, iy,0))
						+(pSrcImg->at(i,j,1) - pPrevImg->at(ix, iy,1))*(pSrcImg->at(i,j,1) - pPrevImg->at(ix, iy,1))+
						(pSrcImg->at(i,j,2) - pPrevImg->at(ix, iy,2))*(pSrcImg->at(i,j,2) - pPrevImg->at(ix, iy,2));
					if (disterror<1.5)
					{
						float sw = m_fTmpWeight*exp(-(disterror*disterror)/(2*1.0*1.0))/**exp(-colordist2/(2*0.03*0.03*3))*/;
						if (sw>0.0001)
						{
							pCsMap->at(i,j,0) = ix;
							pCsMap->at(i,j,1) = iy;
							pWMap->at(i,j) = sw;
						}
						else
						{
							pCsMap->at(i,j,0) = -1;
							pCsMap->at(i,j,1) = -1;
							pWMap->at(i,j) = 0;
						}
					}
					else
					{
						pCsMap->at(i,j, 0) = -1;
						pCsMap->at(i,j,1) = -1;
						pWMap->at(i,j) = 0;
					}

				}
			}
		}
	}
		else
		{
			for (int j=0; j<m_iHeight; ++j)
			{
				for (int i=0; i<m_iWidth; ++i)
				{
					pCsMap->at(i,j,0) = -1;
					pCsMap->at(i,j,1) = -1;
				}
			}
		}
		//Forward:
		printf("before forward\n");
		if (forward&&pNextError)
		{
			for (int j=0; j<m_iHeight; ++j)
				for (int i=0; i<m_iWidth; ++i)
				{
					if (i<=1||i>=m_iWidth-1||j<=1||j>=m_iHeight-1)
					{
						pCsMap->at(i,j, 2) = -1;
						pCsMap->at(i,j, 3) = -1;
						pWMap->at(i,j) = 0;
						continue;
					}
					float x = cu[(m_iHeight-1-j)*m_iWidth+i]+i;
					float y = cv[(m_iHeight-1-j)*m_iWidth+i]+j;
					int ix = x +0.5;
					int iy = y +0.5;

					if (ix>=0&&ix<m_iWidth&&iy>=0&&iy<m_iHeight&&
						x>=0&&x<=m_iWidth-1&&y>=0&&y<=m_iHeight-1)
					{
						float disterror = pCurrError->at(i,j)*0.1;
						float colordist2 = (pSrcImg->at(i,j,0)-pNextImg->at(ix, iy,0))*(pSrcImg->at(i,j,0)-pNextImg->at(ix, iy,0))+
							(pSrcImg->at(i,j,1)-pNextImg->at(ix, iy,1))*(pSrcImg->at(i,j,1)-pNextImg->at(ix, iy,1))+
							(pSrcImg->at(i,j,2)-pNextImg->at(ix, iy,2))*(pSrcImg->at(i,j,2)-pNextImg->at(ix, iy,2));
						
						if (disterror<1.5)
						{
							float sw = m_fTmpWeight*exp(-(disterror*disterror)/(2*1.0*1.0))/**exp(-colordist2/(2*0.03*0.03*3))*/;
							if (sw>0.0001)
							{
								pCsMap->at(i,j,2) = ix;
								pCsMap->at(i,j,3) = iy;
								pWMap->at(i,j) = sw;
							}
							else
							{
								pCsMap->at(i,j,2) = -1;
								pCsMap->at(i,j,3) = -1;
								pWMap->at(i,j) = 0;
							}
						}
						else
						{
							pCsMap->at(i,j,2) = -1;
							pCsMap->at(i,j,3) = -1;
							pWMap->at(i,j) = 0;
						}
					}
					else
					{
						pCsMap->at(i,j,2) = -1;
						pCsMap->at(i,j,3) = -1;
						pWMap->at(i,j) = 0;
					}

				}
			
		}
		else
		{
			for (int j=0; j<m_iHeight; ++j)
			{
				for (int i=0; i<m_iWidth; ++i)
				{
					pCsMap->at(i,j,2) = -1;
					pCsMap->at(i,j,3) = -1;
				}
			}
		}
		csMapList.push_back(pCsMap);
		wMapList.push_back(pWMap);
	}
	pfm.Clear();
	delete[] cbv;
	delete[] cbu;
	delete[] cv;
	delete[] cu;
}


TemporalMattingManager::~TemporalMattingManager(void)
{
}

void TemporalMattingManager::ExtractForground(CxImage* src, CxImage* alpha, CxImage* fg)
{
	fg->Create(m_iWidth, m_iHeight, 24);
	fg->Clear(0);
	for (int j=0; j<m_iHeight; ++j)
	{
		for (int i=0; i<m_iWidth; ++i)
		{
			RGBQUAD rgb = src->GetPixelColor(i,j);
			RGBQUAD nrgb;
			nrgb.rgbRed = alpha->GetPixelGray(i,j)/255.0*rgb.rgbRed;
			nrgb.rgbGreen = alpha->GetPixelGray(i,j)/255.0*rgb.rgbGreen;
			nrgb.rgbBlue = alpha->GetPixelGray(i,j)/255.0*rgb.rgbBlue;
			fg->SetPixelColor(i,j,nrgb);
		}
	}
}