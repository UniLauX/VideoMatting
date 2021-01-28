#include "Beizer.h"
#include "cximage.h"
#include "BeizerCurveDataManager.h"
#include "cv.h"
#include "highgui.h"
#include "ImageSequenceManager.h"
#include "RotoScopeInterface.h"
#include "../RotoScoping/bdrf.h"
#include "simpleimage4cximage.h"
#include <algorithm>


//快速开方函数
float CarmSqrt1(float number){ // 不要更改数据类型
	long i;
	float x, y;
	const float f = 1.5F;   
	x = number * 0.5F;   
	y = number;    
	i = * ( long * ) &y;  
	i = 0x5f3759df - ( i >> 1 ); //注意这一行   
	y = * ( float * ) &i;    
	y = y * ( f - ( x * y * y ) );  
	y = y * ( f - ( x * y * y ) );
	return number * y;
}

//bool cmp(struct extremes e1, struct extremes e2)
//{
//	if (e1.row == e2.row)
//	{
//		return e1.col < e2.col;
//	}
//	return e1.row < e2.row;
//}

void FloodSeedFill(CSimpleImageb * img, int x, int y, unsigned char old_color, unsigned char new_color)
{
	if ( x >= 0 && x < img->_width && y >= 0 && y < img->_height)
	{
		 if(*(img->at(x, y)) == old_color)
		 { 
			 *(img->at(x, y)) = new_color;
	        
			 FloodSeedFill(img, x - 1, y, old_color, new_color);
			 FloodSeedFill(img, x + 1, y, old_color, new_color);
			 FloodSeedFill(img, x, y - 1, old_color, new_color);
			 FloodSeedFill(img, x, y + 1, old_color, new_color);
		}
	}
}
//设了一些初始值
int RotoScopeInterface::kup = 5;
int RotoScopeInterface::klow = -5;
int RotoScopeInterface::startFrameIndex = 0;
int RotoScopeInterface::endFrameIndex = 9;
int RotoScopeInterface::pyramidLevel = 1;

int RotoScopeInterface::Ls = 7;
int RotoScopeInterface::Lb = 3;
float RotoScopeInterface::bMin = 0.3;
float RotoScopeInterface::bMax = 0.7;

RotoScopeInterface::RotoScopeInterface()
{
	pImage.clear();
	numofFrame = 0;
	numofCurve = 0;
	x = NULL;
	y = NULL;
	mask = NULL;
	imageSequence = NULL;
	keyFrameFlag = NULL;
	keyFrameUpdateFlag = NULL;
	mask = NULL;
	MaskMat = NULL;
	maskImage = NULL;
	maskImageAfterSmooth = NULL;
	rx = NULL;
	ry = NULL;
	oldnc = -1;
	oldnf = -1;
}

RotoScopeInterface::~RotoScopeInterface()
{
	for (size_t i = 0; i < pImage.size(); i++)
	{
		delete pImage[i];
	}
	
	if (x != NULL && y!= NULL)
	{
		for (int i = 0; i < numofFrame; i++)
		{
			delete[] x[i];
			delete[] y[i];
		}
		delete[] x;
		delete[] y;
	}

	if (rx != NULL)
	{
		for (int i = 0; i < numofFrame - 2; i++)
		{
			delete[] rx[i];
			delete[] ry[i];
		}
		delete[] rx;
		delete[] ry;
	}

	if (mask != NULL)
	{
		for (int i = 0; i < numofFrame; i++)
		{
			delete[] mask[i];
 		}
		delete[] mask;
	}

	if (!imageSequence)
	{
		delete imageSequence;
	}

	if (maskImage != NULL)
	{
		for (int i = 0; i < totalNumofImage; i++)
		{
			if (maskImage[i]!= NULL)
			{
				delete maskImage[i];
			}
		}
		delete[] maskImage;
	}

	if (maskImageAfterSmooth != NULL)
	{
		for (int i = 0; i < totalNumofImage; i++)
		{
			if (maskImageAfterSmooth[i]!= NULL)
			{
				delete maskImageAfterSmooth[i];
			}
		}
		delete[] maskImageAfterSmooth;
	}
	


	if (MaskMat != NULL)
	{
		cvReleaseMat(&MaskMat);
	}
	
	if (keyFrameFlag != NULL)
	{
		delete[] keyFrameFlag;
	}

	
	
	cout << "~RotoScopeInterface" << endl;
}

void RotoScopeInterface::loadImage( CxImage * image )
{
	
	CSimpleImageb * temp = CxImage2SimpleImage(image);
	pImage.push_back(temp);
	/*IplImage * test = cvCreateImage(cvSize(image->GetWidth(), image->GetHeight()), IPL_DEPTH_8U, 3);
	for (int i = 0; i < test->height; i++)
	{
		for (int j = 0; j < test->width; j++)
		{
			CV_IMAGE_ELEM(test, uchar, i, j * test->nChannels) = image->GetPixelColor(j, i).rgbBlue;
			CV_IMAGE_ELEM(test, uchar, i, j * test->nChannels+ 1) = image->GetPixelColor(j, i).rgbGreen;
			CV_IMAGE_ELEM(test, uchar, i, j * test->nChannels+2) = image->GetPixelColor(j, i).rgbRed;

		}
	}
	cvNamedWindow("a");
	cvShowImage("a", test);
	cvWaitKey(0);*/

}

void RotoScopeInterface::loadControlPointAndMask( BeizerCurveDataManager & curveData, int ta, int tb )
{

	
	//cout << "loadControlPointAndMask" << endl;
	oldnc = numofCurve;
	oldnf = numofFrame;
	numofCurve = curveData.getCurveData(ta).getNumofCurve();
	numofFrame = tb - ta + 1;

	
	//cout << "numofFrame" << numofFrame << endl;
	if (oldnc != -1)
	{
		if (x != NULL && y!= NULL)
		{
			for (int i = 0; i < numofFrame; i++)
			{
				delete[] x[i];
				delete[] y[i];
			}
			delete[] x;
			delete[] y;
			x = NULL;
			y = NULL;
		}
		
		if (mask != NULL)
		{
			for(int i = 0; i < numofFrame; i++)
			{
				delete[] mask[i];
			}
			delete[] mask;
			mask = NULL;
		}
	}
	x = new float*[numofFrame];
	y = new float*[numofFrame];
	
	mask = new bool *[numofFrame];

	int numControl = 2 * (numofCurve + 1);
	
	for (int i = 0; i < numofFrame; i++)
	{//如果曲线闭合，控制点坐标数组较实际大两个，最开始的两个控制点存储两次
		x[i] = new float[numControl];
		y[i] = new float[numControl];
		mask[i] = new bool[numControl];
	}
	
	if (curveData.isClosed())
	{
		numControl -= 2;
		//cout << "curve closed!" << endl;
	}
	//cout << "numControl" << numControl << endl;
	//cout << "initialize over!" << endl;
	for (int i = ta; i <= tb; i++)
	{
		int j = 0;
		for (j = 0; j < numControl; j+=2)
		{
			int index = j / 2 * 3; //切线的index,一个切线三个点，这里只选取两个处理，中间点和前进方向的点
			ControlPoint p = curveData.getCurveData(i).getPoint(index + 1);
			x[i - ta][j] = p.x();
			y[i - ta][j] = p.y();
			mask[i - ta][j] = p.bIsInteractive;

			//cout << j << endl;

			p = curveData.getCurveData(i).getPoint(index + 2);
			x[i - ta][j + 1] = p.x();
			y[i - ta][j + 1] = p.y();
			mask[i - ta][j + 1] = p.bIsInteractive;
			//cout << j << endl;
		}

		if(curveData.isClosed())
		{

			ControlPoint p = curveData.getCurveData(i).getPoint(1);
			x[i - ta][j] = p.x();
			y[i - ta][j] = p.y();
			mask[i - ta][j] = p.bIsInteractive;

			p = curveData.getCurveData(i).getPoint(2);
			x[i - ta][j + 1] = p.x();
			y[i - ta][j + 1] = p.y();
			mask[i - ta][j + 1] = p.bIsInteractive;
		}

		//cout << i << endl;

		memset(mask[0], true, sizeof(bool) * numControl);
		memset(mask[tb-ta], true, sizeof(bool) * numControl);
	}

	//cout << "loadControlPointAndMask " << endl;

}


void RotoScopeInterface::startRotoscope()
{
	RotoScoping roto;
	roto.setPyrLevel(pyramidLevel);
	roto.Begin(x, y, numofFrame, numofCurve, mask, pImage);
	if (oldnf != -1)
	{
		if (rx != NULL && ry != NULL)
		{
			for (int i = 0; i < oldnf - 2; i++)
			{
				delete[] rx[i];
				delete[] ry[i];
 			}
			delete[] rx;
			delete[] ry;
			rx = NULL;
			ry = NULL;
		}
	}
	rx = new float*[numofFrame - 2];
	ry = new float*[numofFrame - 2];
	for (int i = 0; i < numofFrame - 2; i++)
	{
		rx[i] = new float[2 * numofCurve + 1];
		ry[i] = new float[2 * numofCurve + 1];
	}
	roto.getRx(rx);
	roto.getRy(ry);
}

CSimpleImageb * RotoScopeInterface::CxImage2SimpleImage( CxImage * cximage )
{
	if (cximage == NULL)
	{
		cout << "CxImage is NULL! CxImage2SimpleImage failed!" << endl;
		cout << "Please Check!" << endl;
	}
	int imagechannel = 0;
	if (cximage->GetBpp() == 24)
	{
		imagechannel = 3;
	}
	else if (cximage->GetBpp() == 8)
	{
		imagechannel = 1;
	}
	else
	{
		cout << "the channel is not 1 or 3, please check!" << endl;
	}
	CSimpleImageb * simage = new CSimpleImageb;
	simage->set_data(cximage->GetBits(), cximage->GetWidth(), cximage->GetHeight(), imagechannel);

	/*IplImage * test = cvCreateImage(cvSize(simage->_width, simage->_height), IPL_DEPTH_8U, 3);
	for (int i = 0; i < test->height; i++)
	{
		for (int j = 0; j < test->width; j++)
		{
			uchar * color = simage->at(j, test->height - 1 - i);
			CV_IMAGE_ELEM(test, uchar, i, j * test->nChannels) = color[0];
			CV_IMAGE_ELEM(test, uchar, i, j * test->nChannels+ 1) = color[1];
			CV_IMAGE_ELEM(test, uchar, i, j * test->nChannels+2) = color[2];

		}
	}
	cvNamedWindow("a");
	cvShowImage("a", test);
	cvWaitKey(0);*/

	return simage;
}

void RotoScopeInterface::initializeRotoscopeData( int fileRange, int imageHeight, int imageWidth, CString& pathname, CString & extname, int startVedioIndex )
{
	cout << "fileRange: " << fileRange << endl;
	cout << "imageHeight: " << imageHeight << endl;
	cout << "imageWidth: " << imageWidth << endl;
	cout << "pathname: " << pathname << endl;
	cout << "extname: " << extname << endl;
	cout << "startVedioIndex: " << startVedioIndex << endl;
	
	CurveManager.initilaizeCurveData(fileRange);

	totalNumofImage = fileRange;
	if (imageSequence != NULL)
	{
		delete imageSequence;
	}
	imageSequence = new ImageSequenceManager;
	imageSequence->pathname = pathname;
	imageSequence->extName  = extname;
	imageSequence->startVedioIndex = startVedioIndex;

	if (MaskMat != NULL)
	{
		cvReleaseMat(&MaskMat);
	}
	MaskMat = cvCreateMat(imageHeight, imageWidth, CV_32FC1);
	cvZero(MaskMat);

	if (maskImage != NULL)
	{
		delete[] maskImage;
	}
	maskImage = new CSimpleImageb *[fileRange];
	for (int i = 0; i < totalNumofImage; i++)
	{
		maskImage[i] = NULL;
	}

	if (maskImageAfterSmooth != NULL)
	{
		delete[] maskImageAfterSmooth;
	}
	maskImageAfterSmooth = new CSimpleImageb *[fileRange];
	for (int i = 0; i < totalNumofImage; i++)
	{
		maskImageAfterSmooth[i] = NULL;
	}

	if (keyFrameFlag != NULL)
	{
		delete[] keyFrameFlag;
	}
	keyFrameFlag = new int[fileRange];
	memset(keyFrameFlag, 0, fileRange * sizeof(int));

	if (keyFrameUpdateFlag != NULL)
	{
		keyFrameUpdateFlag->clear();
		delete keyFrameUpdateFlag;
	}
	keyFrameUpdateFlag = new vector<bool>(fileRange, false);



}

void RotoScopeInterface::setRotoScopeParameter( int ku, int kl, int startIndex, int endIndex, int pyr_level)
{
	kup  = ku;
	klow = kl;
	startFrameIndex = startIndex;
	endFrameIndex = endIndex;
	pyramidLevel = pyr_level;
	//cout << kup << " " << klow << " " << startFrameIndex << " " << endFrameIndex << " " << pyramidLevel << endl;
}

void RotoScopeInterface::loadRotoScopeData()
{
	imageSequence->loadImage(startFrameIndex, endFrameIndex - startFrameIndex + 1);

	imageHeight = imageSequence->getImageBuffer(startFrameIndex)->GetImage()->GetHeight();
	imageWidth  = imageSequence->getImageBuffer(startFrameIndex)->GetImage()->GetWidth();

	if (pImage.size() != 0)
	{
		for (vector<CSimpleImageb*>::iterator iter = pImage.begin(); iter != pImage.end(); iter++)
		{
			delete (*iter);
		}
		pImage.clear();
	}

	for (int i = startFrameIndex; i <= endFrameIndex; i++)
	{
		loadImage(imageSequence->getImageBuffer(i)->GetImage());
	}

	loadControlPointAndMask(CurveManager, startFrameIndex, endFrameIndex);
}

void RotoScopeInterface::getResult()
{
	cout << "getResult" << endl;
	int numControl = 2 * (numofCurve + 1);//选取两个控制点
	
	if (CurveManager.isClosed())
	{
		numControl -= 2;
		
	}
	
	for (int i = startFrameIndex+1; i < endFrameIndex; i++)
	{
		int index = i - startFrameIndex - 1;
		for (int j = 0; j < numControl / 2; j++)
		{
			float tempx = 2 * rx[index][2 * j] - rx[index][2 * j + 1];
			float tempy = 2 * ry[index][2 * j] - ry[index][2 * j + 1];
			CurveManager.getCurveData(i).updatePoint(CPoint(tempx, tempy), 3 * j);
			CurveManager.getCurveData(i).updatePoint(CPoint(rx[index][2 * j], ry[index][2 * j]), 3 * j + 1);
			CurveManager.getCurveData(i).updatePoint(CPoint(rx[index][2 * j + 1], ry[index][2 * j + 1]), 3 * j + 2);
			//cout << rx[index][2 * j] << " " << ry[index][2 * j] << endl;
			//cout << rx[index][2 * j + 1] << " " << ry[index][2 * j + 1] << endl;
		}
		//cout << i << endl;
	}
	/*for (int i = startFrameIndex; i <= endFrameIndex; i++)
	{
		generateMaskBoundary(i);
		showMaskImage(i);
		generateMaskRegion(i);
		showMaskImage(i);
		alphaSmooth(i);
		showMaskImage(i);
	}*/
	cout << "getResult over!!!" << endl;
}


void RotoScopeInterface::generateMaskBoundary( int index)
{
	cout << "generateMaskImage" << endl;
	BeizerCurveData curvedata = CurveManager.getCurveData(index);
	vector<struct extremes> extremesValue;
	extremesValue.clear();

	if (maskImage[index] != NULL)
	{
		delete maskImage[index];
	}
	maskImage[index] = new CSimpleImageb;
	maskImage[index]->set_size(imageWidth, imageHeight, 1);
	maskImage[index]->zero();


	cout << "here 1 ?" << endl;
	int numofControl = 3 * (numofCurve + 1);//此处三个控制点都计数
	//cout << numofControl << endl;
	if (CurveManager.isClosed())
	{
		numofControl -= 3;
	}

	Beizer * beizer = new Beizer;

	enum line_status{init, rise, fall};

	line_status ls = init;

	line_status fc = init;

	CPoint fisrtP;
	CPoint ppre;

	vector<CPoint> vc;
	vc.clear();

	CPoint pre;
	for ( int i = 0; i < numofCurve; i++ )
	{
		//cout << numofControl << endl;
		int left2index = i * 3 + 1;
		int right2index = ( 3 * (i + 1) ) % numofControl;
		//cout << "left2index: " << left2index << endl;
		//cout << "right2index: " << right2index << endl;
		cout << "getPoint" << endl;
		cout << curvedata.getNumofCurve() << endl;
		CPoint p1 = curvedata.getPoint( left2index );
		CPoint p2 = curvedata.getPoint( left2index + 1 );
		CPoint p3 = curvedata.getPoint( right2index );
		CPoint p4 = curvedata.getPoint( right2index + 1 );

		cout << "addPoint" << endl;

		beizer->addControlPoint(p1.x, p1.y);
		beizer->addControlPoint(p2.x, p2.y);
		beizer->addControlPoint(p3.x, p3.y);
		beizer->addControlPoint(p4.x, p4.y);

		//cout << p1.x << " " << p1.y << endl;
		//cout << p2.x << " " << p2.y << endl;
		//cout << p3.x << " " << p3.y << endl;
		//cout << p4.x << " " << p4.y << endl;

		CPoint tempMid1, tempMid2, tempMid3;

		tempMid1.x = (p1.x + p2.x) / 2;
		tempMid1.y = (p1.y + p2.y) / 2;
		tempMid2.x = (p2.x + p3.x) / 2;
		tempMid2.y = (p2.y + p3.y) / 2;
		tempMid3.x = (p3.x + p4.x) / 2;
		tempMid3.y = (p3.y + p4.y) / 2;

		cout << "sqrt" << endl;
		float dis = 0;
		dis += CarmSqrt1((float)((tempMid1.x - p1.x) * (tempMid1.x - p1.x) 
			                   + (tempMid1.y - p1.y) * (tempMid1.y - p1.y)));
		dis += CarmSqrt1((float)((tempMid2.x - tempMid1.x) * (tempMid2.x - tempMid1.x)
			                   + (tempMid2.y - tempMid1.y) * (tempMid2.y - tempMid1.y)));
		dis += CarmSqrt1((float)((tempMid3.x - tempMid2.x) * (tempMid3.x - tempMid2.x)
			                   + (tempMid3.y - tempMid2.y) * (tempMid3.y - tempMid2.y)));
		dis += CarmSqrt1((float)((p4.x - tempMid3.x) * (p4.x - tempMid3.x)
			                   + (p4.y - tempMid3.y) * (p4.y - tempMid3.y)));

		cout << "down here?" << endl;

		int sampleNum = dis + 2;//折线长度和近似为 曲线上点的数量。

		double delta = static_cast<double>(1.0) / sampleNum;

		//cout << i << endl;
		//cout << "sample Num" << sampleNum << endl;
		//cout << "delta" << delta << endl;

		pre = beizer->getPoint(0);
		if ( i == 0 )
		{
			fisrtP = pre;
			ppre = pre;
		}
		else
		{
			if (pre.y == ppre.y)
			{
				vc.push_back(pre);
				//ppre = pre;
			}
			else if ((pre.y > ppre.y && ls == fall) || (pre.y < ppre.y && ls == rise))//判断是否为极值点
			{
				
				int num = 0;
				int prex = -1;
				int orient;
				int minx = 10000000;
				for ( vector<CPoint>::iterator iter = vc.begin(); iter != vc.end(); iter++ )
				{
					if (iter->x != prex)
					{
						*(maskImage[index]->at(iter->x, iter->y)) = 128;
						prex = iter->x;
						//orient = iter->x - prex;
						minx = __min(iter->x, minx);
						num++;
					}
					
				}

				struct extremes exV;
				if (ls == fall)
				{
					exV.IsMinimal = true;
				}
				else
				{
					exV.IsMinimal = false;
				}
				
				//exV.orient = orient;
				exV.pointNum = num;
				exV.row = vc[0].y;
				exV.col = minx;
				extremesValue.push_back(exV);

				vc.clear();
				vc.push_back(pre);
				if (pre.y > ppre.y)
				{
					ls = rise;
				}
				else
				{
					ls = fall;
				}
			}
		}
		cout << "test table 1" << endl;
		*(maskImage[index]->at(pre.x, pre.y)) = 255;
		for (int j = 1; j <= sampleNum + 1; j++)
		{
			double pointIndex = __min(j * delta, 1);
			CPoint p = beizer->getPoint(pointIndex);
	
			if (p.y == pre.y)
			{
				vc.push_back(p);
			}
			else if ((p.y > pre.y && ls == fall) || (p.y < pre.y && ls == rise))
			{
				int num = 0;
				int prex = -1;
				int orient;
				int minx = 10000000;
				for ( vector<CPoint>::iterator iter = vc.begin(); iter != vc.end(); iter++ )
				{
					if (iter->x != prex)
					{
						*(maskImage[index]->at(iter->x, iter->y)) = 128;
						prex = iter->x;
						//orient = iter->x - prex;
						minx = __min(iter->x, minx);
						num++;
					}

				}

				struct extremes exV;
				if (ls == fall)
				{
					exV.IsMinimal = true;
				}
				else
				{
					exV.IsMinimal = false;
				}
				
				//exV.orient = orient;
				exV.pointNum = num;
				//cout << num << endl;
				exV.row = vc[0].y;
				exV.col = minx;
				//cout << "row:" << vc[0].y << " col" << vc[0].x << endl;
				//cout << "row:" << exV.row << " col" << exV.col << endl;
				extremesValue.push_back(exV);
				
				//*(maskImage[index]->at(vc[0].x, vc[0].y)) = 255;

				vc.clear();
				vc.push_back(p);
				if (p.y > pre.y)
				{
					ls = rise;
				}
				else
				{
					ls = fall;
				}
			}
			//cout << "pre:" << pre.x << " " << pre.y << " p:" << p.x << " " << p.y << endl;
			int insertx;
			int inserty;
			if (abs(pre.y - p.y) > 1 || abs(pre.x - p.x) > 1)
			{
				//cout << "begin...";
				int maxd = __max(abs(pre.x - p.x), abs(pre.y - p.y));
				double ex = static_cast<double>(p.x - pre.x) / maxd;
				double ey = static_cast<double>(p.y - pre.y) / maxd;
				int prex = pre.x;
				int prey = pre.y;
				for (int i = 1; i < maxd; i++)
				{
					insertx = static_cast<int>(pre.x + i * ex + 0.5);
					inserty = static_cast<int>(pre.y + i * ey + 0.5);
					//cout << "x:" << x << " y:" << y << endl;
					*(maskImage[index]->at(insertx, inserty)) = 255;
					if (prey == inserty)
					{
						vc.push_back(CPoint(insertx, inserty));
					}
					else if (prey > inserty)
					{
						vc.clear();
						ls = fall;
						vc.push_back(CPoint(insertx, inserty));
					}
					else if (prey < inserty)
					{
						vc.clear();
						ls = rise;
						vc.push_back(CPoint(insertx, inserty));
					}
				}
				pre.x = insertx;
				pre.y = inserty;
				//cout << "end" << endl;
			}

			*(maskImage[index]->at(p.x, p.y)) = 255;
			if (p.y > pre.y)
			{
				vc.clear();
				ls = rise;
				vc.push_back(p);
			}
			else if (p.y < pre.y)
			{
				vc.clear();
				ls = fall;
				vc.push_back(p);
			}
			else
			{
				vc.push_back(p);
			}

			
			if (i == numofCurve - 1 && j == sampleNum + 1)
			{
				if (fc != ls)
				{
					int ty = vc[0].y;
					int tx = vc[0].x;

					//cout << "status: " << fc << " " << ls << endl;
					int scol;
					int tempnum = 0;
					for ( int j = tx; j >= 0 && *(maskImage[index]->at(j, ty)) >100 ; j--)
					{
						*(maskImage[index]->at(j, ty)) = 128;
						tempnum++;
						scol = j;
					}

					for ( int j = tx; j < imageWidth && *(maskImage[index]->at(j, ty)) >100 ; j++)
					{
			
						*(maskImage[index]->at(j, ty)) = 128;
						tempnum++;
					}
					tempnum--;
					struct extremes exV;
					exV.row = ty;
					exV.col = scol;
					//exV.orient = 1;
					exV.pointNum = tempnum;
					if (ls == fall)
					{
						exV.IsMinimal = true;
					}
					else
					{
						exV.IsMinimal = false;
					}
				}

			}

			if (fc == init)
			{
				fc = ls;
			}
			
			//cout << p.x << " " << p.y << endl;
			pre = p;
			ppre = pre;
		}

		beizer->clearControlPoint();
		//cout << "data over!" << endl;

	}
	
	for (vector<struct extremes>::iterator iter = extremesValue.begin(); iter != extremesValue.end(); iter++)
	{
		int tw = -1;
		int delta = -1;

		int pointNum = iter->pointNum;
		//cout << pointNum << endl;

		//cout << "row:" << iter->row << " col" << iter->col << endl;
		
		if (iter->IsMinimal)
		{
			tw = 1;
			delta = 1;
		}

		bool outloopflag = true;

		CPoint temp;

		//for (int i = 0; i < pointNum; i++)
		//{
		//	*(maskImage[index]->at(iter->col + i/* * iter->orient*/, iter->row)) = 255;
		//}

		for (int i = 1; outloopflag ; i++)
		{
			int row = iter->row + tw;
			int col = iter->col - i;

			//cout << "row" << row << " " << "srow" << iter->row << endl;
			//cout << "col" << col << endl;
			int flag = 0;

			for ( int j = 0; j < iter->pointNum + 2 * i; j++ )
			{
				int tcol = __max(0, __min(j  + col, maskImage[index]->_width -1));
			
				/*if (*(maskImage[index]->at(tcol, row)) == 128)
				{
					outloopflag = false;
					break;
				}*/


				if (*(maskImage[index]->at(tcol, row)) != 0 && flag == 0)
				{
					if (j < pointNum + 2 * i - 1)
					{
						flag = 1;
						temp.x = tcol;
						temp.y = row;
					}
					
				}
				else if (*(maskImage[index]->at(tcol, row)) == 0 && flag == 1)
				{
					flag = 2;
				}
				else if (*(maskImage[index]->at(tcol, row)) != 0 && flag == 2)
				{
					flag = 3;
				}
			}
			if (flag == 3)
			{
				break;
			}

			else if ( flag == 2 || flag == 1 )
			{

				for (int k = temp.x - 1; k >= 0; k--)
				{
					if (*(maskImage[index]->at(k, temp.y)) == 255)
					{
						*(maskImage[index]->at(k, temp.y)) = 128;
					}
					else
					{
						break;
					}
				}
				
				for (int k = temp.x; k < maskImage[index]->_width; k++)
				{
					if (*(maskImage[index]->at(k, temp.y)) == 255)
					{
						*(maskImage[index]->at(k, temp.y)) = 128;
					}
					else
					{
						break;
					}
				}
				tw += delta;
			}
		}
	}

	vc.clear();
	extremesValue.clear();

	cout << "generateMaskBoundary over!" << endl;

}

void RotoScopeInterface::showMaskImage( int index )
{
	if (maskImage[index] != NULL)
	{
		IplImage * imageshow = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 1);
		//imageshow->imageData = (char *)maskImage[index]->_data;
		cout << imageWidth << " " << imageHeight << endl;
		for (int i = 0; i < imageHeight; i++)
		{
			for (int j = 0; j < imageWidth; j++)
			{
				CV_IMAGE_ELEM(imageshow, uchar, imageHeight - 1 -  i, j) = *(maskImage[index]->at(j, i));
			}
		}
		cvNamedWindow("mask");
		cvShowImage("mask", imageshow);
		char filename[100];
		sprintf(filename, "E:\\mask%03d.jpg", index);
		cvSaveImage(filename, imageshow);
		cvWaitKey(0);
		cvReleaseImage(&imageshow);
	}
}

void RotoScopeInterface::generateMaskRegion( int index )
{
	cout << "generateMaskRegion" << endl;
	
	for (int i = 0; i < imageHeight; i++)
	{
		bool need_fill = false;
		bool isInBound = false;
		bool isExtrem = false;
		for (int j = 0; j < imageWidth; j++)
		{
			if (*(maskImage[index]->at(j, i)) == 255)
			{
				while (j < (imageWidth - 1) && *(maskImage[index]->at(j+1, i)) == 255)
				{
					j++;
				}
				need_fill = !need_fill;
			}

			if (isExtrem && *(maskImage[index]->at(j, i)) != 0)
			{
				*(maskImage[index]->at(j, i)) = 255;
				continue;
			}
			else if (isExtrem && *(maskImage[index]->at(j, i)) == 0)
			{
				isExtrem = false;
				continue;
			}

			if (need_fill)
			{
				*(maskImage[index]->at(j, i)) = 255;
				
			}

			else if (*(maskImage[index]->at(j, i)) != 255 && *(maskImage[index]->at(j, i)) != 0)
			{
				*(maskImage[index]->at(j, i)) = 255;
				isExtrem = true;
			}
		}
	}


	

}

void RotoScopeInterface::generateMaskRegion( int index, vector<CPoint>& seedPoint )
{

	cout << "generateMaskRegion" << endl;

	for (vector<CPoint>::iterator iter = seedPoint.begin(); iter != seedPoint.end(); iter++)
	{
		FloodSeedFill(maskImage[index], iter->x, iter->y, 0, 255);
	}

}

void RotoScopeInterface::alphaSmooth( int index )
{
	CxImage * srcimg = imageSequence->getImageBuffer(index)->GetImage();
	uchar * img = srcimg->GetBits();
	int width = srcimg->GetWidth();
	int height = srcimg->GetHeight();
	int effwidth = srcimg->GetEffWidth();

	if (maskImageAfterSmooth[index]!= NULL)
	{
		delete maskImageAfterSmooth[index];
	}
	maskImageAfterSmooth[index] = new CSimpleImageb;
	maskImageAfterSmooth[index]->set_size(width, height, 1);
	maskImageAfterSmooth[index]->zero();


	vsf_bdrf_param bdryParam;
	bdryParam.m_bhwsz = Lb;
	bdryParam.m_shwsz = Ls;
	bdryParam.m_bmin = bMin;
	bdryParam.m_bmax = bMax;

	cout << "bdryParam.m_bhwsz : " << bdryParam.m_bhwsz << endl;
	cout << "bdryParam.m_shwsz : " << bdryParam.m_shwsz << endl;
	cout << "bdryParam.m_rhwsz : " << bdryParam.m_rhwsz << endl;
	cout << "bdryParam.m_bmin : " << bdryParam.m_bmin << endl;
	cout << "bdryParam.m_bmax : " << bdryParam.m_bmax << endl;
	cout << "bdryParam.m_ssig : " << bdryParam.m_ssig << endl;
	cout << "bdryParam.m_mu0 : " << bdryParam.m_mu0 << endl;
	cout << "bdryParam.m_mus : " << bdryParam.m_mus << endl;
	cout << "bdryParam.m_msw : " << bdryParam.m_msw << endl;
	cout << "bdryParam.m_pe : " << bdryParam.m_pe << endl;
	vsf_refine_boundary(img, width, height, effwidth, maskImage[index]->_data, maskImage[index]->_effwidth, 255, maskImageAfterSmooth[index]->_data, maskImageAfterSmooth[index]->_effwidth, &bdryParam);

	memcpy(maskImage[index]->_data, maskImageAfterSmooth[index]->_data, maskImage[index]->_size);

	delete maskImageAfterSmooth[index];
	maskImageAfterSmooth[index] = NULL;
}


CxImage * RotoScopeInterface::getMaskImage( int index )
{
	generateMaskBoundary(index);
	//showMaskImage(index);
	generateMaskRegion(index);
	//generateMaskRegion(index, seedPoint);
	showMaskImage(index);
	alphaSmooth(index);
	//showMaskImage(index);
	CxImage * img = new CxImage;
	bool flag = SimpleImage::convert(*(maskImage[index]), *img);
	if (flag)
	{
		return img;
	}
	else
	{
		cout << "simpleImage convert to CxImage failed!" << endl;
		return NULL;

	}
	
}

void RotoScopeInterface::setAlphaSmoothParameter( int ls, int lb, float bmin, float bmax )
{
	RotoScopeInterface::Ls = ls;
	RotoScopeInterface::Lb = lb;
	RotoScopeInterface::bMin = bmin;
	RotoScopeInterface::bMax = bmax;
	cout << Ls << Lb << bMin << bMax << endl;
}



