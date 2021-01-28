#include "stdafx.h"
#include "VideoEditing.h"
#include "VideoEditingDoc.h"
#include "VideoEditingView.h"
#include <WinAble.h>
#include "MainFrm.h"
#include <io.h>
#include "LayerEditDlg.h"
#include "Disparity.h"
#include "Vision/Others/opencv10/include/cv.h"
#include "Vision/Others/OpenCV10/include/highgui.h"

//begin caldepth  segmentation tools are disabled
//activated by checkbox event
void CVideoEditingView::BeginCalDepth(bool isAllLayerGenerated,bool isCalDepthStarted)
{
	for (int i=0;i<vec_layer.size();++i)
	{
		vec_layer[i].SetPos(0);
	}
	this->allLayerGenerated=isAllLayerGenerated;
	this->calDepthStarted=isCalDepthStarted;
	editTool=NOTOOL;
	CString str;
	str=filePath;
	str.AppendFormat("%s","VEDepthMap");

	cout<<vec_layer[current_layer].NumOfKeyFrame()<<endl;
	if (vec_layer[current_layer].NumOfKeyFrame()==0)
	{
		int pos=vec_layer[current_layer].GetStartIndex();
		cout<<pos<<endl;
		imageBuffer.Goto(pos);
		resImageBuffer.Goto(pos);
		alphaBuffer.Goto(pos);
		trimapBuffer.Goto(pos);
		Invalidate();
		this->setProcess();
	}

	CreateDirectory(str,0);

}

//calculate plane param
void CVideoEditingView::GetPlaneParam(Point_3D p1,Point_3D p2,Point_3D p3, double &a,double &b,double &c,double &d)//a*x+b*y+c*z+d=0;
{

	b=((p1.z-p2.z)*(p2.x-p3.x)-(p2.z-p3.z)*(p1.x-p2.x))/(double)((p1.y-p2.y)*(p2.x-p3.x)-(p1.y-p3.y)*(p1.x-p2.x));
	a=((p1.z-p2.z)-b*(p1.y-p2.y))/(double)(p1.x-p2.x);
	c=p1.z-a*p1.x-b*p1.y;

}

//record keyframe information
void CVideoEditingView::AddKeyFrame(int pos,Point_3D p1,Point_3D p2,Point_3D p3)
{

	int index=vec_layer[current_layer].IsKeyFrame(pos);
	if (index!=-1)//already exists ,update?
	{
		if(::MessageBox(NULL, "This key frame alread exists ,want to replace the points?", "Confirm", MB_OKCANCEL)!=IDOK)
			return;
		else
		{

			vec_layer[current_layer].UpdateKeyFrame(index,p1,p2,p3);

		}
	}

	else// new key frame
	{

		vec_layer[current_layer].AddKeyFrame(pos);
		vec_layer[current_layer].AddPlanePoint(p1,p2,p3);
	}
}


void CVideoEditingView::calDepth(CxImage &label,CxImage &depthImage,CxImage &alpha, CxImage &layerdepth,double &a,double &b,double &c,double &d)
{
	cout<<"in calDepth"<<endl;

	BYTE lrgb;
	RGBQUAD drgb;


	for (int j=0;j<alpha.GetHeight();++j)
	{
		for (int i=0;i<alpha.GetWidth();++i)
		{
			lrgb=alpha.GetPixelGray(i,j);

			if (lrgb!=0)
			{
				double z=a*i+b*j+c;
				if (z>255)
				{
					z=255;
				}
				else if (z<0)
				{
					z=0;
				}
				drgb.rgbBlue=drgb.rgbGreen=drgb.rgbRed=(int)z;
				layerdepth.SetPixelColor(i,j,drgb);
				RGBQUAD labelrgb = label.GetPixelColor(i,j);
				if (labelrgb.rgbBlue==255)
					depthImage.SetPixelColor(i,j,drgb);
			}
		}
	}
	std::cout<<"out of calDepth"<<std::endl;
}

void CVideoEditingView::CalDepth2(CxImage &LabelMsk, CxImage &DepthImg, ZFloatImage& DspImg, CxImage &AlphaImg, CxImage &LyrDepth, ZFloatImage& LyrDsp, double &a, double &b, double &c, double &d)
{
	cout<<"Calculating Depth."<<endl;

	int iWidth = AlphaImg.GetWidth();
	int iHeight = AlphaImg.GetHeight();

	for(int y = 0; y < iHeight; ++y)
	{
		for(int x = 0; x < iWidth; ++x)
		{
			if(AlphaImg.GetPixelGray(x, y))
			{
				double z = a * x + b * y + c;
				int DepthVal = std::min(std::max(int(z + 0.5), 0), 255);
				float DspVal = (m_fDspMax - m_fDspMin) * z / 255 + m_fDspMin;

				LyrDepth.SetPixelColor(x, y, RGB(DepthVal, DepthVal, DepthVal));
				LyrDsp.at(x, iHeight - 1 - y) = DspVal;

				if(LabelMsk.GetPixelGray(x, y) == 255)
				{
					DepthImg.SetPixelColor(x, y, RGB(DepthVal, DepthVal, DepthVal));
					DspImg.at(x, iHeight - 1 - y) = DspVal;
				}
			}
		}
	}

	std::cout<<"Depth Calculated."<<std::endl;
}

void CVideoEditingView::calDepth(CxImage &label,CxImage &depthImage,CxImage &alpha, CxImage &layerdepth,Point_3D p1,Point_3D p2,Point_3D p3)
{
	double a,b,c,d;
	GetPlaneParam(p1,p2,p3,a,b,c,d);
	calDepth(label,depthImage,alpha, layerdepth,a,b,c,d);
}

void CVideoEditingView::GetNextFramePoints(CString firstFramePath, CString secondFramePath, Point_3D &p1, Point_3D &p2, Point_3D &p3)
{
	IplImage* imgA=cvLoadImage(firstFramePath.GetBuffer(),CV_LOAD_IMAGE_GRAYSCALE);
	IplImage* imgB=cvLoadImage(secondFramePath.GetBuffer(),CV_LOAD_IMAGE_GRAYSCALE);

	CvSize img_sz=cvGetSize(imgA);
	int win_size=10;

	CvPoint2D32f* cornersA=new CvPoint2D32f[3];

	cornersA[0].x=p1.x;
	cornersA[0].y=img_sz.height-p1.y;
	cornersA[1].x=p2.x;
	cornersA[1].y=img_sz.height-p2.y;
	cornersA[2].x=p3.x;
	cornersA[2].y=img_sz.height-p3.y;

	CvSize pyr_sz = cvSize( imgA->width+8, imgB->height/3 );
	IplImage* pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
	IplImage* pyrB  = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );

	CvPoint2D32f* cornersB= new CvPoint2D32f[ 3 ];

	char features_found[ 3 ];
	float feature_errors[ 3 ];
	cvCalcOpticalFlowPyrLK(
		imgA,
		imgB,
		pyrA,
		pyrB,
		cornersA,
		cornersB,
		5,
		cvSize( win_size,win_size ),
		5,
		features_found,
		feature_errors,
		cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 ),
		0
		);

	p1.x=cornersB[0].x;
	p1.y=img_sz.height-cornersB[0].y;
	p2.x=cornersB[1].x;
	p2.y=img_sz.height-cornersB[1].y;
	p3.x=cornersB[2].x;
	p3.y=img_sz.height-cornersB[2].y;

	cvReleaseImage(&imgA);
	cvReleaseImage(&imgB);
	cvReleaseImage(&pyrA);
	cvReleaseImage(&pyrB);

	delete[] cornersA;
	delete[] cornersB;
}

void CVideoEditingView::SetZDepth(int d)
{
	this->z_depth=d;
}

//void CVideoEditingView::CalculateDepthMap()
//{
//	cout<<"in calculateDepthMap"<<endl;
//	CString tframename;
//	tframename=imageBuffer.GetFileName();
//	int index=tframename.Find(".");
//	tframename.Delete(index,4);
//
//	//cout<<"name:"<<imageBuffer.GetFileName()<<endl;
//
//	CString depthFileName;
//	depthFileName=filePath;
//	depthFileName.AppendFormat("%s","VEDepthMap\\");
//	depthFileName.AppendFormat("%s%s",tframename,trimapExt.GetBuffer());
//
//	CxImage depthMap;
//	cout<<"before load"<<endl;
//	cout<<depthFileName<<endl;
//	if(!depthMap.Load(depthFileName,CXIMAGE_FORMAT_BMP))
//	{
//		depthMap.Copy(*imageBuffer.GetImage());
//		depthMap.Clear(0);
//	}
//
//	cout<<alphaBuffer.GetFrameName()<<endl;
//
//	alphaBuffer.Goto(alphaBuffer.GetFramePos());
//
//	if (alphaBuffer.GetImage())
//	{
//		MessageBox("aaa");
//	}
//
//	calDepth(*(alphaBuffer.GetImage()),depthMap,plane_point[0],plane_point[1],plane_point[2]);
//
//	depthMap.Save(depthFileName,CXIMAGE_FORMAT_BMP);
//
//	while(alphaBuffer.Forward())
//	{
//		imageBuffer.Goto(alphaBuffer.GetFramePos());
//		resImageBuffer.Goto(alphaBuffer.GetFramePos());
//		trimapBuffer.Goto(alphaBuffer.GetFramePos());
//
//		GetNextFramePoints(imageBuffer.GetPreviousFrameName(),imageBuffer.GetFrameName(),plane_point[0],plane_point[1],plane_point[2]);
//
//		tframename=imageBuffer.GetFileName();
//		index=tframename.Find(".");
//		tframename.Delete(index,4);
//		depthFileName=filePath;
//		depthFileName.AppendFormat("%s","VEDepthMap\\");
//		depthFileName.AppendFormat("%s%s",tframename,trimapExt.GetBuffer());
//
//		if(!depthMap.Load(depthFileName,CXIMAGE_FORMAT_PNG))
//		{
//			depthMap.Copy(*imageBuffer.GetImage());
//			depthMap.Clear(0);
//		}
//		calDepth(*alphaBuffer.GetImage(),depthMap,plane_point[0],plane_point[1],plane_point[2]);
//		depthMap.Save(depthFileName,CXIMAGE_FORMAT_PNG);
//	}
//	printf("Depth cal over\n");
//}

void CVideoEditingView::CalculateDepthMap(int startFrame,int startFrameIndex,int endFrame,int endFrameIndex)
{
	//calculate planes for startFrame and endFrame
	Point_3D points[3];
	vec_layer[current_layer].GetPlanePoint(startFrameIndex,points[0],points[1],points[2]);//get startFrame points
	double a1,b1,c1,d1;
	GetPlaneParam(points[0],points[1],points[2],a1,b1,c1,d1);//first plane param
	vec_layer[current_layer].GetPlanePoint(endFrameIndex,points[0],points[1],points[2]);
	double a2,b2,c2,d2;
	GetPlaneParam(points[0],points[1],points[2],a2,b2,c2,d2);//second plane param
	CString tframename;
	CString depthFileName;
	CxImage depthMap;
	CxImage layerDepth;

	ImageBuffer softAlphaBuffer;
	CString path = filePath;
	path.AppendFormat("%s\\%s", vec_layer[current_layer].GetName(),"VEAlpha");
	//CreateDirectory(path,0);
	path.AppendFormat("\\%s", filename);
	softAlphaBuffer.SetName(path, alphaExt);
	bool alphaexist = softAlphaBuffer.OpenVideo(startFrame);
	bool maskexist = alphaBuffer.OpenVideo(startFrame);
	ImageBuffer layerDepthBuffer;
	path = filePath;
	path.AppendFormat("%s\\%s", vec_layer[current_layer].GetName(),"Depth");
	path.AppendFormat("\\%s", filename);
	layerDepthBuffer.SetName(path, alphaExt);
	ImageBuffer LyrDspBuf;
	LyrDspBuf.SetName(path, CString("raw"));


	for (int pos=startFrame;pos<=endFrame;++pos)
	{
		alphaBuffer.Goto(pos);
		imageBuffer.Goto(pos);
		resImageBuffer.Goto(pos);
		trimapBuffer.Goto(pos);
		softAlphaBuffer.Goto(pos);
		layerDepthBuffer.SetPos(pos);
		LyrDspBuf.SetPos(pos);

		tframename=imageBuffer.GetFileName();
		int index=tframename.Find(".");
		tframename.Delete(index,4);

		depthFileName=filePath;
		depthFileName.AppendFormat("%s","VEDepthMap\\");
		depthFileName.AppendFormat("%s.%s",tframename,trimapExt.GetBuffer());

		if(!depthMap.Load(depthFileName,CXIMAGE_FORMAT_PNG))
		{
			depthMap.Copy(*imageBuffer.GetImage());
			depthMap.Clear(0);
		}

		CString DspFileName = filePath;
		DspFileName.AppendFormat("VEDepthMap\\%s.%s", tframename, "raw");

		CxImage* pCxImg = imageBuffer.GetImage();
		int iWidth = pCxImg->GetWidth();
		int iHeight = pCxImg->GetHeight();
		ZFloatImage DspImg = CDisparity::LoadDspMap(iWidth, iHeight, DspFileName);
		if(!DspImg.GetMap())
			DspImg.CreateAndInit(iWidth, iHeight, 1, m_fDspMin);

		layerDepth.Copy(depthMap);
		layerDepth.Clear();
		ZFloatImage LyrDsp;
		LyrDsp.CreateAndInit(iWidth, iHeight, 1, m_fDspMin);

		double con=(pos-startFrame)/(double)(endFrame-startFrame);
		double a3,b3,c3,d3;
		a3=(1-con)*a1+con*a2;
		b3=(1-con)*b1+con*b2;
		c3=(1-con)*c1+con*c2;
		//d3=(1-con)*d1+con*d2;
		if(alphaexist&&maskexist)//不存在alpha-mat，则用mask代替
			//calDepth(*(alphaBuffer.GetImage()),depthMap, *(softAlphaBuffer.GetImage()),layerDepth,a3,b3,c3,d3);//根据alpha-mat产生深度图
			CalDepth2(*(alphaBuffer.GetImage()), depthMap, DspImg, *(softAlphaBuffer.GetImage()), layerDepth, LyrDsp, a3, b3, c3, d3);//根据alpha-mat产生深度图
		else if (!alphaexist&&maskexist)
			//calDepth(*(alphaBuffer.GetImage()),depthMap, *(alphaBuffer.GetImage()),layerDepth,a3,b3,c3,d3);
			CalDepth2(*(alphaBuffer.GetImage()), depthMap, DspImg, *(alphaBuffer.GetImage()), layerDepth, LyrDsp, a3, b3, c3, d3);
		else if(!maskexist&&alphaexist)
			//calDepth(*(softAlphaBuffer.GetImage()),depthMap,*(softAlphaBuffer.GetImage()),layerDepth,a3,b3,c3,d3);
			CalDepth2(*(softAlphaBuffer.GetImage()), depthMap, DspImg, *(softAlphaBuffer.GetImage()), layerDepth, LyrDsp, a3, b3, c3, d3);

		depthMap.Save(depthFileName,CXIMAGE_FORMAT_PNG);
		CDisparity::SaveDspMap(DspImg, DspFileName);
		layerDepth.Save(layerDepthBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);
		CDisparity::SaveDspMap(LyrDsp, LyrDspBuf.GetFrameName());
	}
}