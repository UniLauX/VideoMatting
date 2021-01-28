// VideoEditingDoc.cpp : implementation of the CVideoEditingDoc class
//

#include "stdafx.h"
#include "VideoEditing.h"
#include "VideoEditingDoc.h"
#include "../grab/BiImageProcess.h"
#include "MotionEstUsingFlow_BLACK.h"
#include "RotoScopeInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CVideoEditingDoc

IMPLEMENT_DYNCREATE(CVideoEditingDoc, CDocument)

BEGIN_MESSAGE_MAP(CVideoEditingDoc, CDocument)
	ON_COMMAND(ID_SETTING_COLOR, &CVideoEditingDoc::OnSettingColor)
END_MESSAGE_MAP()
// CVideoEditingDoc construction/destruction
CVideoEditingDoc::CVideoEditingDoc()
{
     Opened=false;
     cal=false;
     mode=GRABCUT;
     updateMode=GLOBAL_UPDATE;
     videoWinSize=80;
     updateWinSize=25;
     iterateTime=2;
	 mattingType = MAT_SEQ;
	 mattingMode=MAT_PARTIMG;
	 mattingAlgo = CLOSEDMAT;
}

CVideoEditingDoc::~CVideoEditingDoc()
{
}

BOOL CVideoEditingDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	return TRUE;
}

// CVideoEditingDoc serialization

void CVideoEditingDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}
// CVideoEditingDoc diagnostics
#ifdef _DEBUG
void CVideoEditingDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CVideoEditingDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CVideoEditingDoc commands

void CVideoEditingDoc::New()
{
	Opened=false;
	cal=false;
	image.Clear();
	scrbImage.Clear();
	alpha.Clear(128);
	scrbLocal.clear();
	trimap.Clear();
	firstFrameOK=false;
	rectangled=false;
}

void CVideoEditingDoc::UpdateImages(CxImage *image)
{
     if(image==NULL)
          return;
     this->lastImage.Copy(this->image);
     this->image.Copy(*image);
     scrbImage.Create(image->GetWidth(), image->GetHeight(), 24);
     alpha.Create(image->GetWidth(), image->GetHeight(), 8);
     alpha.Clear(128);
     alpha.SetGrayPalette();
     scrbLocal.clear();
}

void CVideoEditingDoc::AdjustPos(int &x, int &y, RECT &region)
{
     double z, zx;
	 z=((double)alpha.GetHeight())/(region.bottom-region.top+1);
	 zx = ((double)alpha.GetWidth())/(region.right - region.left +1);
	 x=(int)((x-region.left+1)*zx);
	 y=alpha.GetHeight()-1-(int)((y-region.top+1)*z);
}

void CVideoEditingDoc::SetCutRegion(int x1, int y1, int x2, int y2, RECT &region)
{
     AdjustPos(x1, y1, region);
     AdjustPos(x2, y2, region);
     //rect.left=x1;
     //rect.top=y1;
     //rect.right=x2;
     //rect.bottom=y2;
	 if (x1>x2)
	 {
		 rect.left = x2;
		 rect.right = x1;
	 }
	 else
	 {
		 rect.left = x1;
		rect.right = x2;
	 }
	 if (y1>y2)
	 {
		 rect.top = y1;
		 rect.bottom = y2;
	 }
	 else{
		 rect.top = y2;
		 rect.bottom = y1;
	 }
}

void CVideoEditingDoc::DrawRectangle(int x1, int y1, 
                                     int x2, int y2, 
                                     RECT &region, int mode)
{
     RGBQUAD rgb;
     const int BORDER=128;

     rgb.rgbRed=BORDER*mode;
     rgb.rgbGreen=rgb.rgbRed;
     rgb.rgbBlue=rgb.rgbRed;

     AdjustPos(x1, y1, region);
     AdjustPos(x2, y2, region);
}

void CVideoEditingDoc::DrawPoint(int x, int y, int radius, RECT &region, int mode)
{
     DWORD i,j;
     DWORD x1,y1,x2,y2;
     RGBQUAD rgb1,rgb2;
	 AdjustPos(x, y, region);
	 x1=(x-radius>=0)?x-radius:0, y1=(y-radius>=0)?y-radius:0;
	 x2=(x+radius<this->GetWidth())?x+radius:this->GetWidth()-1 , y2=(y+radius<this->GetHeight())?y+radius:this->GetHeight()-1;
     switch(mode) 
	 {
	 case 0://back brush
		 rgb2.rgbRed=0;
		 rgb2.rgbGreen=0;
		 rgb2.rgbBlue=0;
		 for(i=x1;i<=x2;++i)
		 {
			 for(j=y1;j<=y2;++j)
			 {
				 alpha.SetPixelIndex(i,j, 255);
				 scrbImage.SetPixelColor(i, j, rgb2);
				 scrbLocal.addBackgPt(i,j);
				 trimap.SetPixelColor(i, j, rgb2);
			 }
		 }
		 break;
	 case 1://for brush
		 rgb2.rgbRed=255;
		 rgb2.rgbGreen=255;
		 rgb2.rgbBlue=255;
		 for(i=x1;i<=x2;++i)
		 {
			 for(j=y1;j<=y2;++j)
			 {
				 alpha.SetPixelIndex(i,j, 0);
				 scrbImage.SetPixelColor(i, j, rgb2);
				 scrbLocal.addForgPt(i,j);
				 trimap.SetPixelColor(i, j, rgb2);
			 }
		 }
		 break;
	 case 2:// trimap中的unknown笔刷
		 rgb2.rgbRed=128;
		 rgb2.rgbGreen=128;
		 rgb2.rgbBlue=128;
		 for(i=x1;i<=x2;++i)
		{
				for(j=y1;j<=y2;++j)
				{
					trimap.SetPixelColor(i, j, rgb2);
				}
		 }
		 break;
	 }
}

void CVideoEditingDoc::DrawLine(int x0, int y0, int x1, int y1, 
                                int radius, RECT &region, int mode)
{
    int dy=y1-y0;
    int dx=x1-x0;
    int stepx,stepy;
    if(dy<0) 
    { 
        dy=-dy;
        stepy=-1; 
    } 
    else 
        stepy=1; 

    if(dx<0) 
    { 
        dx=-dx;  
        stepx=-1; 
    } 
    else
        stepx=1; 
    dy<<=1;                                                   // dy is now 2*dy
    dx<<=1;                                                  // dx is now 2*dx
    DrawPoint(x0, y0, radius, region, mode);
    if(dx>dy) 
    {
        int fraction=dy-(dx >> 1);                           // same as 2*dy - dx
        while(x0!=x1) 
        {
            if(fraction>=0) 
            {
                y0+=stepy;
                fraction-=dx;                                // same as fraction -= 2*dx
            }
            x0+=stepx;
            fraction+=dy;                                    // same as fraction -= 2*dy
            DrawPoint(x0, y0, radius, region, mode);
        }
    } 
    else 
    {
        int fraction=dx-(dy >> 1);
        while(y0!=y1) 
        {
            if(fraction>=0) 
            {
                x0+=stepx;
                fraction-=dy;
            }
            y0+=stepy;
            fraction+=dx;
            DrawPoint(x0, y0, radius, region, mode);
        }
    }
}

void CVideoEditingDoc::Run(CxImage &result, CxImage &alp)
{    
	double stt, ett, dtt;
	stt = clock();
     BeginWaitCursor();
     if(mode==CLOSEDFORM)
     {
          matting.SetImage(&image, &scrbImage);
          matting.GetResult(result);
     }
     else
     if(mode==GRABCUT)
     {
          if(!cal)
          {
			  RECT c_cropRect;
			  cropRect(c_cropRect,rect);
			  grab.init(image,rect.left,rect.bottom,rect.right,rect.top,c_cropRect);
			  grab.doseg();
			  updateMode=LOCAL_UPDATE;      // After grabcut, we use Local updating as the default option.
			  cal=true;
			  grab.GetResult(result);
			  grab.GetAlphaResult(alp);
			  contour.SetLocalWinSize(updateWinSize);
			  contour.SetFrameInfo(grab.GetFrame(),grab.GetLabels());
			  cutting.SetImage(grab.GetFrame());
			  cutting.SetLabel(grab.GetLabels());
			  //cutting.ImageBeta();
			  contour.GetBeta_WholeImg();
			  grab.RRelease();//release rescourse
          }
		  else
		  {
			  if(updateMode==GLOBAL_UPDATE);
			  else
			  {
				  double starttime, endtime, dtime;
				  starttime = clock();
				  CxImage tmp; 
				  VPOINTS pixels;
				  std::cout<<"Update win size: "<<updateWinSize<<std::endl;
				  contour.SetLocalWinSize(updateWinSize);
				  cutting.beta = contour.beta;
				  for(int i=0;i<3;++i)
				  {
					  cutting.UpdateLabel(scrbLocal);
					  CxImage localtrimap;
					  if (contour.GetLocalPixels(cutting.GetLabels(), scrbLocal, pixels, localtrimap)==-1)
					  {
						  cout<<"in break"<<endl;
						  break;
					  }
					  localtrimap.Save("H:/test/lotri_te.bmp", CXIMAGE_FORMAT_BMP);
					  cutting.LocalCalculate(pixels,scrbLocal,alpha, localtrimap);
				  }
				  endtime = clock();
				  dtime = (endtime - starttime)/CLK_TCK;
				  std::cout<<" Update Time: "<<dtime<<std::endl;
				  BiImageProcess::checkConnectivity(cutting.GetLabels(),image.GetWidth(), image.GetHeight());
				  cutting.GetResult(result);
				  cutting.GetAlphaResult(alp);
			  }
		  }
	  scrbLocal.clear();
     }
     EndWaitCursor();
}

/* 
Cutting video steps:
Just after the grabcut, we should use:
     1. contour.SetFrameInfo(cutting.GetFrame(), cutting.GetLabels());
     2. contour.GetLocalClassifier();
     3. contour.GetCutResult(label);
notice that these operations above can be done only once.

Video processing:
     1. Move the local windows to new positions and set the next frame image:
          contour.TransClassifier(nextFrame, newCenters, param);
     After it, we can also create new GMMs.
     2. Get the cutting result, return the result to "label":
          contour.GetCutResult(label);
     3. Get new local classifiers according to new labels(this may be changed to other forms):
          contour.GetLocalClassifier();
*/

void CVideoEditingDoc::InitVideoFirstFrame()
{
     TRANS_CLASSIFIER_PARAM param;
     // After run this, we use local updating as the default option.
     updateMode=LOCAL_UPDATE;
     param.mode=3;
     contour.SetLocalWinSize(videoWinSize);
     contour.SetFrameInfo(cutting.GetFrame(), cutting.GetLabels());
     contour.GetLocalClassifier(param);
}

bool CVideoEditingDoc::InitVideoFirstFrame(CxImage &image,unsigned char *label)
{
	if(label==NULL)
		return false;
	TRANS_CLASSIFIER_PARAM param;
	// After run this, we use local updating as the default option.
	updateMode=LOCAL_UPDATE;
	param.mode=3;
	contour.SetLocalWinSize(videoWinSize);
	contour.SetFrameInfo(&image,label);
	
	contour.GetLocalClassifier(param);
	
	contour.GetCutResultUC(label);
	return true;
}


RECT CVideoEditingDoc::FindContourRect(CxImage srcImg,int lclWinSize)
{
	RECT contrRct;

	int m_width=srcImg.GetWidth();
	int m_height=srcImg.GetHeight();
	int contrRctMinX=m_width-1;
	int contrRctMinY=m_height-1; 
	int contrRctMaxX=0;
	int contrRctMaxY=0;
	lclWinSize=lclWinSize;

	VVPOINTS allLclWinCtrs=*contour.GetLocalWinCenters();
	int m=allLclWinCtrs.size();
	for(int i=0;i<m;++i)
	{
		int n=allLclWinCtrs[i].size();
		for(int j=0;j<n;++j)
		{
			if(contrRctMinX>allLclWinCtrs[i][j].x)
				contrRctMinX=allLclWinCtrs[i][j].x;

			if(contrRctMinY>allLclWinCtrs[i][j].y)
				contrRctMinY=allLclWinCtrs[i][j].y;

			if(contrRctMaxX<allLclWinCtrs[i][j].x)
				contrRctMaxX=allLclWinCtrs[i][j].x;

			if(contrRctMaxY<allLclWinCtrs[i][j].y)
				contrRctMaxY=allLclWinCtrs[i][j].y;
		}
	}

	contrRct.left=__max(0,contrRctMinX-lclWinSize/2);
	contrRct.right=__min(contrRctMaxX+lclWinSize/2,m_width-1);
	contrRct.bottom=__max(0,contrRctMinY-lclWinSize/2);
	contrRct.top=__min(contrRctMaxY+lclWinSize/2,m_height-1);

	RGBQUAD rgb;
	rgb.rgbRed=255;
	rgb.rgbGreen=rgb.rgbBlue=0;
	CxImage contrRctImg;
	contrRctImg.Copy(srcImg);
	contrRctImg.DrawLine(contrRct.left,contrRct.right,contrRct.top,contrRct.top,rgb); 
	contrRctImg.DrawLine(contrRct.right,contrRct.right,contrRct.top,contrRct.bottom,rgb);
	contrRctImg.DrawLine(contrRct.right,contrRct.left,contrRct.bottom,contrRct.bottom,rgb);
	contrRctImg.DrawLine(contrRct.left,contrRct.left,contrRct.bottom,contrRct.top,rgb);

	contrRctImg.Save("D://contrRctImg.jpg",CXIMAGE_FORMAT_JPG);


	return contrRct;
}


//未调用

void CVideoEditingDoc::RunVideo(CxImage &result, CxImage &alpha)
{
	double startT,endT,duration;
	startT=clock();


	 const int localWinSize=80;

	 VVPOINTS newCenters;

     TRANS_CLASSIFIER_PARAM param;
     param.mode=3;

	 int size;
	 size=image.GetWidth()*image.GetHeight();
     unsigned char *newLabel;
     newLabel=new unsigned char[size];
     memset(newLabel, 0, sizeof(unsigned char)*size);
     

    RECT contrMinRect;
	contrMinRect=FindContourRect(lastImage,localWinSize);
	int ctRctWdh=contrMinRect.right-contrMinRect.left+1;
	int ctRctHgt=contrMinRect.top-contrMinRect.bottom+1;


	float t1,t2,t3;
	float totalT=clock();

	if(logText)
		logText<<"************************************"<<endl;

	if(cutting.GetLabels()==NULL)
		cout<<"point is null"<<endl;
  


	t1=clock();
	Wml::GMatrix<Wml::Vector2f>OptFlwMatrixForeward(ctRctWdh,ctRctHgt);
	//Wml::GMatrix<Wml::Vector2f>OptFlwMatrixBackward(ctRctWdh,ctRctHgt);

	OptFlwMatrixForeward=motion.GenOpticalFlow(lastImage,image,contrMinRect);
	// OptFlwMatrixBackward=motion.GenOpticalFlow(dstImg,srcImg,contrMinRect);

	t1=clock()-t1;
	t1/=CLOCKS_PER_SEC;
	cout<<"Generate Optical Flow time is: "<<t1<<endl;

	//	Wml::GMatrix<float> OptFlwMatrixError=motion.GenOpticalFlowError(OptFlwMatrixForeward,OptFlwMatrixBackward);

	t2=clock();
	Wml::GMatrix<Wml::Vector2f> MeanOptFlwMatrix=motion.GenMeanFlowInContour(OptFlwMatrixForeward,cutting.GetLabels(),lastImage,videoWinSize,contrMinRect);
	t2=clock()-t2;
	t2/=CLK_TCK;
	cout<<"Calculate mean flow in Contour cost time is: "<<t2<<endl;


	t3=clock();
	MotionEstimate::PropagateLyrMaskByMeanFlow(lastImage,image,cutting.GetLabels(),newLabel,MeanOptFlwMatrix,contrMinRect);
	MotionEstimate::PropagateWinsByMeanFlow(lastImage,image,cutting.GetLabels(),newLabel,*(contour.GetLocalWinCenters()),newCenters,localWinSize,MeanOptFlwMatrix,contrMinRect);
	t3=clock()-t3;
	t3/=CLK_TCK;
	cout<<"Calculate mean flow in Contour and propagate mask& localWinCenters cost time is: "<<t3<<endl;

	totalT=clock()-totalT;
	totalT/=CLK_TCK;
	cout<<" Total time of calculate the Optical Flow is: "<<totalT<<endl;


	//motion.getCentersOfNextFrame(lastImage,image,cutting.GetLabels(),newLabel,*(contour.GetLocalWinCenters()),newCenters,localWinSize/2);
	


	endT=clock();
	duration=(endT-startT)/CLK_TCK;
	if(logText)
		logText<<"getCentersofNextFrame time: "<<duration<<endl;

	cout<<"getCentersofNextFrame time: "<<duration<<endl;

	//contour.TestLocalWinPosition(&image,newCenters);



	contour.SetNewLabel(newLabel, size);
	contour.TransClassifier(&image, newCenters, param);     
	contour.GetBeta();

	endT=clock();
	duration=(endT-startT)/CLK_TCK;
	cout<<"prepare time: "<<duration<<endl;

    for(int i=0;i<3;++i)
    {
		double st , et, dt;
		st = clock();
        contour.GetCutResultUC(cutting.GetLabels());
		et = clock();
		dt = (et - st)/CLK_TCK;
		cout<<" get cut result time"<<dt<<endl;
        contour.GetLocalClassifier(param, i);
		et = clock();
		dt = (et - st)/CLK_TCK;
		cout<<"Get local classifier time"<<dt<<endl;
    }

	endT=clock();
	duration=(endT-startT)/CLK_TCK;
	cout<<"iteration time: "<<duration<<endl;
    cout << "*************************************" << endl;
    contour.GetCutResult(result);
	
    contour.GetAlphaResult(alpha);
	delete[] newLabel;
	endT = clock();
	duration = (endT - startT)/CLK_TCK;
	cout<<" total propagation time: "<<duration<<endl;
   
}

//  editingView->GetDocument()->RunVideo(result, alpha, editingView->filePath, editingView->imageBuffer.GetFramePos()-1);
void CVideoEditingDoc::RunVideo(CxImage &result, CxImage &alpha, CString path, int frame)
{
	CheckMemory();
	std::cout<<"in runvideo"<<std::endl;
	double startT,endT,sAllT,duration;
    sAllT=clock();
	
	CString uflowpath = path;     //path="F:\VideoSequence\circle2"
	uflowpath.AppendFormat("opt\\u_%d_%d.raw", frame, frame+1);  //uflowpath="F:\VideoSequence\circle2\opt\u_0_1.raw"

	CString vflowpath = path;
	vflowpath.AppendFormat("opt\\v_%d_%d.raw", frame, frame+1);
    
	int size;

	TRANS_CLASSIFIER_PARAM param;
	unsigned char *newLabel;



	startT=clock();
	param.mode=3; 
	size=image.GetWidth()*image.GetHeight();

	newLabel=new unsigned char[size];
	memset(newLabel, 0, sizeof(unsigned char)*size);

	{
		VVPOINTS newCenters;
		printf("before motion\n");
		MotionEstUsingFlow_BLACK motionEst;

		motionEst.PredictMotion(uflowpath.GetBuffer(), vflowpath.GetBuffer(), cutting.GetLabels(), newLabel,
			image.GetWidth(), 	image.GetHeight(),	newCenters,
			*(contour.GetLocalWinCenters()),contour.GetLocalWinSize()/2);            //预测newCenters 和 newLabel,newCenters未改变，newLabel有错
		
	//	motion.getCentersOfNextFrame(lastImage,image,cutting.GetLabels(),
	//		newCenters,*(contour.GetLocalWinCenters()),
	//		contour.GetLocalWinSize()/2,newLabel);
		printf("after motion\n");

		endT=clock();
		duration=(endT-startT)/CLK_TCK;
	    cout<<"Propagate and getCentersofNextFrame time: "<<duration<<endl;
	//	contour.TestLocalWinPosition(&image,newCenters);






		contour.SetNewLabel(newLabel, size);           //将得到的错误Label设置为contour的label
		delete[] newLabel;
     /*                                                //在此处newCenters完全和winCenters相同
		VVPOINTS oldCenters=contour.winCenters;
		for(int m=0;m<oldCenters.size();++m)
		{
			for(int n=0;n<oldCenters[m].size();++n)
			{
				if(oldCenters[m][n].x!=newCenters[m][n].x)
					cout<<"have changed"<<endl;
			}
		}
      */

		contour.TransClassifier(&image, newCenters, param);                //主要作用是移动窗口中心，以及利用错误的Label对color,shape模型设置参数
	}
	std::cout<<"local win num: "<<contour.winCenters[0].size()<<std::endl;
	contour.GetBeta();                                                     //不知道有何用处


	endT=clock();
	duration=(endT-startT)/CLK_TCK;
	cout<<"prepare time,that is sum of propagate and transClassifier: "<<duration<<endl;


	startT=clock();
	printf("before interation\n");
	CheckMemory();
	for(int i=0;i<iterateTime;++i)// 大图像需增加迭代的次数2
	{
		double st , et, dt;
		st = clock();   
		contour.GetCutResultUC(cutting.GetLabels());
		et = clock();
		dt = (et - st)/CLK_TCK;
		CheckMemory();
		cout<<" get cut result time"<<dt<<endl;



		st = clock();
		contour.GetLocalClassifier(param, i);
		et = clock();
		dt = (et - st)/CLK_TCK;
    	CheckMemory();
		cout<<"Get local classifier time"<<dt<<endl;
	}
	endT=clock();
	duration=(endT-startT)/CLK_TCK;
	cout<<"iteration time: "<<duration<<endl;



//	boost::thread thrd1(boost::BOOST_BIND(&Contour::GetCutResult,&contour,result));
//	boost::thread thrd1(boost::BOOST_BIND(&Contour::GetCutResult,&contour, result));
//	thrd1.join();
	
    contour.GetCutResult(result);
	contour.GetAlphaResult(alpha);



	endT = clock();
	duration = (endT - sAllT)/CLK_TCK;
	cout<<" total propagation time: "<<duration<<endl;
	CheckMemory();

	cout << "*************************************" << endl;
}

void CVideoEditingDoc::RunMatting(CxImage &result)
{
	contour.SetLocalWinSize(updateWinSize);
	contour.SetFrameInfo(cutting.GetFrame(), cutting.GetLabels());
	contour.RunMatting(matting, result);
}

// For matting...
void GetBound(CxImage &label, vector<POINT> &boundPts)
{
    POINT pt;
    int i,j,k;
    int x,y,w,h;
    RGBQUAD rgb1,rgb2;
    int direct[4][2]={{1,0},{0,1},{-1,0},{0,-1}};

    boundPts.clear();
    w=label.GetWidth();
    h=label.GetHeight();
    for(i=0;i<=h;++i)
    {
        for(j=0;j<=w;++j)
        {
            rgb1=label.GetPixelColor(j, i);
            if(rgb1.rgbRed==255)
            {			
                for(k=0;k<4;++k)
                {	
                    y=i+direct[k][0];
                    x=j+direct[k][1];
                    rgb2=label.GetPixelColor(x, y);
                    if(x>=0 && x<w && y>=0 && y<h)
                    {
                        if(rgb1.rgbRed!=rgb2.rgbRed)
                        {
                            pt.x=j;
                            pt.y=i;
                            boundPts.push_back(pt);
                            break;
                        }
                    }
                }
            }
        }
    }
}

int GetMinDistance(int x, int y, vector<POINT> &boundPts)
{
    int i,size;
    int x1,y1;
    POINT pt;
    int tmp;
    int Min(1000000);

    size=(int)boundPts.size();
    for(i=0;i<size;++i)
    {
        pt=boundPts[i];
        x1=pt.x-x;
        y1=pt.y-y;
        tmp=x1*x1+y1*y1;
        if(tmp<Min)
            Min=tmp;
    }
    return Min;
}

void CVideoEditingDoc::RunMatting(CxImage &result, CxImage &pic, CxImage &label, CxImage &trimap, CxImage &fgimg)
{
    int i,j;
    int w,h;
    int left,right;
    int top,bottom;
    RGBQUAD rgb;
    CxImage image,scrib;
    vector<POINT> boundPts;
    const int Z(100);

    w=pic.GetWidth();
    h=pic.GetHeight();
    left=w; right=0;
    top=0; bottom=h;
    for(i=0;i<h;++i)
    {
        for(j=0;j<w;++j)
        {
            rgb=trimap.GetPixelColor(j, i);
            if(rgb.rgbRed!=0)
            {
                if(j<left)
                    left=j;
                if(j>right)
                    right=j;
                if(i<bottom)
                    bottom=i;
                if(i>top)
                    top=i;
            }
        }
    }

    left=left-Z<0 ? 0:left-Z;
    right=right+Z>=w ? w-1:right+Z;
    bottom=bottom-Z<0 ? 0:bottom-Z;
    top=top+Z>=h ? h-1:top+Z;
    GetBound(label, boundPts);
    image.Create(right-left+1, top-bottom+1, 24);
	scrib.Create(right-left+1, top-bottom+1, 24);
    for(i=bottom;i<=top;++i)
    {
        for(j=left;j<=right;++j)
        {
            rgb=pic.GetPixelColor(j, i);
            image.SetPixelColor(j-left, i-bottom, rgb);
			scrib.SetPixelColor(j-left, i-bottom, trimap.GetPixelColor(j,i));
        }
    }
    int index;
    double *localAlpha;
    result.Copy(pic);
    result.Clear(0);
	fgimg.Copy(pic);
	fgimg.Clear(230);
	image.Save("H:/test/image.jpg", CXIMAGE_FORMAT_JPG);
	scrib.Save("H:/test/scrib.bmp", CXIMAGE_FORMAT_BMP);
    matting.SetImage(&image, &scrib);
	//matting.SetLevelNum(3);
	//matting.SetActiveLevelNum(3);

    localAlpha=matting.DoMatting();
    for(i=bottom;i<=top;++i)
    {
        for(j=left;j<=right;++j)
        {
            index=(i-bottom)*(right-left+1)+j-left;
            rgb.rgbRed=(BYTE)(localAlpha[index]*255);
            rgb.rgbGreen=rgb.rgbRed;
            rgb.rgbBlue=rgb.rgbRed;
            result.SetPixelColor(j, i, rgb);
			 RGBQUAD colrgb;
			 colrgb = pic.GetPixelColor(j,i);
			 if (localAlpha[index]>0)
			 {
				 colrgb.rgbRed = localAlpha[index]*colrgb.rgbRed+(1- localAlpha[index])*230;
				 colrgb.rgbBlue = localAlpha[index]*colrgb.rgbBlue+(1-localAlpha[index])*230;
				 colrgb.rgbGreen = localAlpha[index]*colrgb.rgbGreen+(1- localAlpha[index])*230;
				 fgimg.SetPixelColor(j,i,colrgb);
			 }
			 
        }
    }
    delete [] localAlpha;
}
void CVideoEditingDoc::SetParameter(int level, int activelevel, int winsize, 
                                    double alphath, double epsilon)
{
	matting.SetLevelNum(level);
	matting.SetActiveLevelNum(activelevel);
	matting.SetErodeWinSize(winsize);
	matting.SetThrAlpha(alphath);
	matting.SetEpsilon(epsilon);
}

void CVideoEditingDoc::RestoreFrame(CxImage *frame, CxImage *alpha)
{
     if(frame==NULL || alpha==NULL)
          return;
     //grab.SetImage(frame);
     cutting.SetImage(frame);
     //grab.UpdateLabel(*alpha,scrbLocal);
     /*CxImage temp;
     grab.GetAlphaResult(temp);*/
     cutting.UpdateLabel(*alpha, scrbLocal);
     contour.SetFrameInfo(frame, cutting.GetLabels());
     UpdateImages(frame);
}

void CVideoEditingDoc::GetResult(CxImage &result, CxImage &alpha)
{

	//grab.UpdateLabel(scrbLocal);
	//grab.GetResult(result);
	//grab.GetAlphaResult(alpha);
     cutting.UpdateLabel(scrbLocal);
     cutting.GetResult(result);
     cutting.GetAlphaResult(alpha);
}

void CVideoEditingDoc::GetCFParameter(int &level,int &activeLevel,int &winsize,
									  double &thralpha,double & epsilon)
{
	level=matting.GetLevelNum();
	activeLevel=matting.GetActiveLevelNum();
	winsize=matting.GetWinSize();
	thralpha=matting.GetThrAlpha();
	epsilon=matting.GetEpsilon();
}

void CVideoEditingDoc::SetVideoWinSize(int vWinSize)
{
	this->videoWinSize=vWinSize;

}

void CVideoEditingDoc::SetUpdateWinSize(int uWinSize)
{
	this->updateWinSize=uWinSize;

}

void CVideoEditingDoc::SetIterateTime(int time)
{
	this->iterateTime=time;

}

void CVideoEditingDoc::SetLocalClassifierParam(CLASSIFIERPARAM &param)
{
    LocalClassifier::param=param;
}

void CVideoEditingDoc::GetLocalClassifierParam(CLASSIFIERPARAM &param)
{
    param=LocalClassifier::param;
}

void CVideoEditingDoc::OnSettingColor()
{
	// TODO: Add your command handler code here
	CColorDialog colorDlg;
	INT_PTR nres=colorDlg.DoModal();
	if (nres==IDOK)
	{
		COLORREF col=colorDlg.GetColor();
		BYTE r=GetRValue(col);
		BYTE g=GetGValue(col);
		BYTE b=GetBValue(col);
		contour.SetColor(r,g,b);
		cutting.SetColor(r,g,b);
		//grab.SetColor(r,g,b);
	}
}

int CVideoEditingDoc::GetWidth()
{
	return alpha.GetWidth();
}

int CVideoEditingDoc::GetHeight()
{
	return alpha.GetHeight();
}

void CVideoEditingDoc::cropRect(RECT &rect,CRect &rectangleRegion)
{
	int xstart,ystart,xend,yend;

	xstart=rectangleRegion.left;
	yend=rectangleRegion.top;
	xend=rectangleRegion.right;
	ystart=rectangleRegion.bottom;

	rect.left=(xstart-100>0)?(xstart-100):0;
	rect.top=(yend+100<image.GetHeight())?(yend+100):(image.GetHeight());
	rect.right=(xend+100<image.GetWidth())?(xend+100):(image.GetWidth());
	rect.bottom=(ystart-100>0)?(ystart-100):0;

	rectangleRegion.left=xstart-rect.left;
	rectangleRegion.bottom=ystart-rect.bottom;
	rectangleRegion.right=xend-rect.left;
	rectangleRegion.top=yend-rect.bottom;

}

void CVideoEditingDoc::GetTrimap(CxImage &trimap, int dis)
{
	//grab.GetTrimap(trimap, dis);
	cutting.GetTrimap(trimap, dis);
	this->trimap.Copy(trimap);
}

void CVideoEditingDoc::SetTrimap(CxImage &trimap)
{
    this->trimap.Copy(trimap);
}

void CVideoEditingDoc::GetTrimap(CxImage &trimap)
{
    trimap.Copy(this->trimap);
}

void CVideoEditingDoc::ResetScribble(void)
{
	//this->scrbLocal.Clear(128);
	this->scrbLocal.clear();
}

RGBQUAD CVideoEditingDoc::GetContourBG()
{
	return contour.GetRGB();
}

#include <Windows.h>

void CVideoEditingDoc::CheckMemory()
{
	MEMORYSTATUS myMemoryStatus;
	memset(&myMemoryStatus, 0, sizeof(MEMORYSTATUS));
	myMemoryStatus.dwLength = sizeof(MEMORYSTATUS);

	GlobalMemoryStatus(&myMemoryStatus);

	const int nOneM = 1024 * 1024;
	printf("memory of P %fM, available memory of P %fM\n",
		(double)myMemoryStatus.dwTotalVirtual / nOneM, (double)myMemoryStatus.dwAvailVirtual / nOneM
		);
}

void CVideoEditingDoc::setRotoScopeParameter( int kup, int klow, int startFrameIndex, int endFrameIndex, int pyr_level )
{
	roto->setRotoScopeParameter(kup, klow, startFrameIndex, endFrameIndex, pyr_level);
}


