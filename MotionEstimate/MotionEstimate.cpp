#include "MotionEstimate.h"


MotionEstimate::MotionEstimate(void)
{
}

MotionEstimate::~MotionEstimate(void)
{
}

void MotionEstimate::getCentersOfNextFrame(CxImage& preImage,CxImage& curImage, const unsigned char *preLabel,unsigned char* newLabel,const VVPOINTS&localWins,VVPOINTS& newCenters,int winSize)
{
	cout<<"hello,motion estimated"<<endl;

	
} 

Wml::GMatrix<Wml::Vector2f>MotionEstimate::GenOpticalFlow(CxImage srcFrm, CxImage dstFrm,const RECT& contrRct,double alpha, double Rt, int MnWd, int ItrNm_OtFwPDE , int ItrNm_InFwPDE, int ItrNm_CG)
{

//	srcFrm.Save("D://srcImg.jpg",CXIMAGE_FORMAT_JPG);
//	dstFrm.Save("D://dstImg.jpg",CXIMAGE_FORMAT_JPG);

	cout<<"Compute Optical flow from srcFrm to dstFrm "<<endl;

	int iWidth=contrRct.right-contrRct.left+1;
	int iHeight=contrRct.top-contrRct.bottom+1;
//	int m_width=srcFrm.GetWidth();
//	int m_heigth=srcFrm.GetHeight();

/*
	DImage Im1(iWidth,iHeight,3);
	double* pImgDt=Im1.data();

	RGBQUAD rgb;
	for(int x=contrRct.left;x<=contrRct.right;x++)
	{
		for(int y=contrRct.bottom;y<=contrRct.top;y++)
		{
			
			rgb=srcFrm.GetPixelColor(x,y);
			
		   pImgDt[(y-contrRct.bottom)*iWidth*3+(x-contrRct.left)*3]=double(rgb.rgbRed/255.0);
		   pImgDt[(y-contrRct.bottom)*iWidth*3+(x-contrRct.left)*3+1]=double(rgb.rgbGreen/255.0);
		   pImgDt[(y-contrRct.bottom)*iWidth*3+(x-contrRct.left)*3+2]=double(rgb.rgbBlue/255.0);
		}
	}

	DImage Im2(iWidth,iHeight,3);
	pImgDt=Im2.data();
	for(int x=contrRct.left;x<=contrRct.right;x++)
	{
		for(int y=contrRct.bottom;y<=contrRct.top;y++)
		{
			rgb=dstFrm.GetPixelColor(x,y);
		
			pImgDt[(y-contrRct.bottom)*iWidth*3+(x-contrRct.left)*3]=double(rgb.rgbRed/255.0);
			pImgDt[(y-contrRct.bottom)*iWidth*3+(x-contrRct.left)*3+1]=double(rgb.rgbGreen/255.0);
			pImgDt[(y-contrRct.bottom)*iWidth*3+(x-contrRct.left)*3+2]=double(rgb.rgbBlue/255.0);
		}
	}

*/	
	DImage Im1(iWidth,iHeight,3);
	double* pImgDt=Im1.data();

	RGBQUAD rgb;
	for(int x=0;x<iWidth;x++)
	{
		for(int y=0;y<iHeight;y++)
		{

			rgb=srcFrm.GetPixelColor(x+contrRct.left,y+contrRct.bottom);

			pImgDt[y*iWidth*3+x*3]=double(rgb.rgbRed/255.0);
			pImgDt[y*iWidth*3+x*3+1]=double(rgb.rgbGreen/255.0);
			pImgDt[y*iWidth*3+x*3+2]=double(rgb.rgbBlue/255.0);
		}
	}

	DImage Im2(iWidth,iHeight,3);
	pImgDt=Im2.data();

	for(int x=0;x<iWidth;x++)
	{
		for(int y=0;y<iHeight;y++)
		{

			rgb=dstFrm.GetPixelColor(x+contrRct.left,y+contrRct.bottom);

			pImgDt[y*iWidth*3+x*3]=double(rgb.rgbRed/255.0);
			pImgDt[y*iWidth*3+x*3+1]=double(rgb.rgbGreen/255.0);
			pImgDt[y*iWidth*3+x*3+2]=double(rgb.rgbBlue/255.0);
		}
	}


	DImage V_x,V_y,WrpI2;
  OpticalFlow::Coarse2FineFlow(V_x,V_y,WrpI2,Im1,Im2,alpha,Rt,MnWd,ItrNm_OtFwPDE,ItrNm_InFwPDE,ItrNm_CG);

	Wml::GMatrix<Wml::Vector2f> OpFwMp(iWidth, iHeight);


	for(int x=0;x<iWidth;x++)
	{
		for(int y=0;y<iHeight;y++)
		{


			double v_x=V_x.data()[y*iWidth+x];
			double v_y=V_y.data()[y*iWidth+x];


			OpFwMp(x,y)=Wml::Vector2f(v_x,v_y);

		}
	}

	//testing
	{
		CxImage OpFwImg(iWidth, iHeight, 24);

		for(int y = 0; y <iHeight; y++)
		{
			for(int x = 0; x < iWidth; x++)
			{
				Wml::Vector3<uchar> color;
				computeColor(OpFwMp(x, y).X(), OpFwMp(x, y).Y(), &color[0]);
				//OpFwImg.SetPixelColor(x, iHeight - 1 - y, RGB(color[0], color[1], color[2]));
				OpFwImg.SetPixelColor(x, y, RGB(color[0], color[1], color[2]));
			}
		}

		const char* optRlst="D://OptFlwImg.png";
		OpFwImg.Save(optRlst,CXIMAGE_FORMAT_PNG);
		//cout<<"process OK!"<<endl;
	}

	return OpFwMp;
}



Wml::GMatrix<float>MotionEstimate::GenOpticalFlowError( const Wml::GMatrix<Wml::Vector2f>& OptFlwMatrixForeward, const Wml::GMatrix<Wml::Vector2f>& OptFlwMatrixBackward )
{
    int rowNum=OptFlwMatrixForeward.GetRows();
	int colNum=OptFlwMatrixForeward.GetColumns();
	Wml::GMatrix<float>OptFlwMatrixError(rowNum,colNum);
    for(int x=0;x<rowNum;++x)
	{
		for(int y=0;y<colNum;y++)
		{
			int fx=__max(0,__min(OptFlwMatrixForeward(x,y).X()+0.5+x,rowNum));
			int fy=__max(0,__min(OptFlwMatrixForeward(x,y).Y()+0.5+y,colNum));    //not so clear
			float bx=OptFlwMatrixBackward(fx,fy).X()+fx;
			float by=OptFlwMatrixBackward(fx,fy).Y()+fy;

			OptFlwMatrixError(x,y)=GetDistance(x,y,bx,by);
  
		}
	}

	return OptFlwMatrixError;
}

Wml::GMatrix<Wml::Vector2f>MotionEstimate::GenMeanFlowInContour( const Wml::GMatrix<Wml::Vector2f>& OptFlwMatrix,const unsigned char *preLabel,CxImage& preImage,int localWinSize,const RECT& contrRct )
{
	float meanX,meanY;
	int Sx;
	int Ex;
	int Sy;
	int Ey;
	int cnt;
	int m_width=preImage.GetWidth();
	//int m_height=preImage.GetHeight();
	int conRctWdh=contrRct.right-contrRct.left+1;
	int conRctHgt=contrRct.top-contrRct.bottom+1;

	Wml::GMatrix<Wml::Vector2f> MeanOptFlwMatrix(conRctWdh,conRctHgt);
	int flowWinRadius=localWinSize/2;   
    int index;
	 
   
int nPixels=conRctWdh*conRctHgt;
int y,x;

#pragma  omp parallel for if(THREAD_SWITCH)
	//for(int y=0;y<conRctHgt;++y)
	//{
	//	for(int x=0;x<conRctWdh;++x)
for(int pix=0;pix<nPixels;pix++)
		{
			y=pix/conRctWdh;
			x=pix%conRctWdh;
			meanX=meanY=0;
			Sx=__max(0,x-flowWinRadius);
			Ex=__min(conRctWdh-1,x+flowWinRadius);
			Sy=__max(0,y-flowWinRadius);
			Ey=__min(conRctHgt-1,y+flowWinRadius);


           
		index=(y+contrRct.bottom)*m_width+(x+contrRct.left);  
		
		
			cnt=0;
			for(int j=Sy;j<=Ey;++j)
			{
				for(int i=Sx;i<=Ex;++i)
				{
				
					if(preLabel[index]==1&&GetDistance(x,y,i,j)<=flowWinRadius)
					{
					
						meanX+=OptFlwMatrix(i,j).X();
						meanY+=OptFlwMatrix(i,j).Y();
						cnt++;

					}
				}
			}
			

			meanX/=(float)(cnt>0? cnt:1);
			meanY/=(float)(cnt>0? cnt:1);
			MeanOptFlwMatrix(x,y).X()=meanX;
			MeanOptFlwMatrix(x,y).Y()=meanY;


		//}
	}


	return OptFlwMatrix;

}

void MotionEstimate:: PropagateLyrMaskByMeanFlow(CxImage& preImage,CxImage& curImage,const unsigned char *preLabel,unsigned char* newLabel, const Wml::GMatrix<Wml::Vector2f>& MeanOptFlwMatrix,const RECT& contrRct )
{
         int m_width=preImage.GetWidth();
	//	 int m_height=preImage.GetHeight();

		 int conRctWdh=contrRct.right-contrRct.left;
		 int conRctHgt=contrRct.top-contrRct.bottom;

		 int index,newIdx;
		
		 for(int imgY=contrRct.bottom;imgY<=contrRct.top;++imgY)
		 {
			 for(int imgX=contrRct.left;imgX<=contrRct.right;++imgX)
			 {
				index=imgY*m_width+imgX;
				 if(preLabel[index]==1)
				 {
                   int tX=__max(contrRct.left,__min(contrRct.right,MeanOptFlwMatrix(imgX-contrRct.left,imgY-contrRct.bottom).X()+0.5+imgX));  //not so clear
				   int tY=__max(contrRct.bottom,__min(contrRct.top,MeanOptFlwMatrix(imgX-contrRct.left,imgY-contrRct.bottom).Y()+0.5+imgY));
				 
				    newIdx=tY*m_width+tX;
                    newLabel[newIdx]=1;
				 }
			 }
		 }

		
      /*
		 for(int imgY=0;imgY<conRctHgt;++imgY)
		 {
			 for(int imgX=0;imgX<conRctWdh;++imgX)
			 {
				 index=(imgY+contrRct.bottom)*m_width+(imgX+contrRct.left);
				 if(preLabel[index]==1)
				 {
					 int tX=__max(0,__min(conRctWdh-1,MeanOptFlwMatrix(imgX,imgY).X()+0.5+imgX));  //not so clear
					 int tY=__max(0,__min(conRctHgt-1,MeanOptFlwMatrix(imgX,imgY).Y()+0.5+imgY));

					 newIdx=(tY+contrRct.bottom)*m_width+(tX+contrRct.left);
					 newLabel[newIdx]=1;
				 }
			 }
		 }

		 */	
		 //test code print mask before smooth
		 {
			 CxImage combinedImg=GetCombinedMaskImg(curImage,newLabel,0.5);
			 combinedImg.Save("D://combineRoughImg.jpg",CXIMAGE_FORMAT_JPG);
		 }

		 SmoothTransferMask(curImage,newLabel);
		 //test code print boundry points
		 {
			 CxImage combinedImg1=GetCombinedMaskImg(curImage,newLabel,0.5);
			 combinedImg1.Save("D://combineSmoothImg.jpg",CXIMAGE_FORMAT_JPG);
		 }

}

float MotionEstimate::GetDistance(int x1,int y1,int x2,int y2)
{
	return sqrt(pow(float(x1-x2),2)+pow(float(y1-y2),2));
}

CxImage MotionEstimate:: GetCombinedMaskImg( CxImage dstImg, unsigned char* dstLabelMask, float alpha )
 {
     int m_width=dstImg.GetWidth();
	 int m_height=dstImg.GetHeight();
	// int channelNum=dstImg.GetBpp()/8;              //求Channel Num这块有点问题

	 RGBQUAD rgb;
	 int index;

	 for(int y=0;y<m_height;++y)
	 {
		 for(int x=0;x<m_width;++x)
		 {
			   index=y*m_width+x;
               rgb.rgbRed=__min(255,int(dstImg.GetPixelColor(x,y).rgbRed)*(1.0-alpha)+dstLabelMask[index]*alpha*255);
			   rgb.rgbGreen=__min(255,int(dstImg.GetPixelColor(x,y).rgbGreen)*(1.0-alpha)+dstLabelMask[index]*alpha*255);
			   rgb.rgbBlue=__min(255,int(dstImg.GetPixelColor(x,y).rgbBlue)*(1.0-alpha)+dstLabelMask[index]*alpha*255);
               dstImg.SetPixelColor(x,y,rgb);
			
		 }
	 }
     
	 //cout<<channelNum<<endl;
	 return dstImg;
 }

void MotionEstimate::SmoothTransferMask(CxImage dstImg, unsigned char* labelMask )
{
      int m_width=dstImg.GetWidth();
	  int m_height=dstImg.GetHeight();
	  int radius=1;

	  unsigned char *oldLabel;
	  int size=dstImg.GetWidth()*dstImg.GetHeight();
	  oldLabel=new unsigned char[size];
	 // memset(newLabel, 0, sizeof(unsigned char)*size);
	  memcpy(oldLabel,labelMask,sizeof(unsigned char)*size);
      int index,rIndex;
	  for(int iterIdx=0;iterIdx<1;++iterIdx)
	  {
		  for(int imgY=0;imgY<m_height;++imgY)
		  {
			  for(int imgX=0;imgX<m_width;++imgX)
			  {
                
				  index=imgY*m_width+imgX;
				  int Sx=__max(0,__min(imgX-radius,m_width-1));
				  int Ex=__max(0,__min(imgX+radius,m_width-1));
				  int Sy=__max(0,__min(imgY-radius,m_height-1));
				  int Ey=__max(0,__min(imgY+radius,m_height-1));
				  int cnt=0;
				  for(int rY=Sy;rY<=Ey;++rY)
				  {
					  for(int rX=Sx;rX<=Ex;++rX)
					  {
						  rIndex=rY*m_width+rX;
						  if(oldLabel[rIndex]==1)
							  cnt++;
					  }
				  }

				  if(cnt>=5)
				  {

					  labelMask[index]=1;
					  oldLabel[index]=1;
				  }
			  }
		  }
	  }

}

void MotionEstimate::PropagateWinsByMeanFlow( CxImage srcFrm, CxImage& dstFrm, const unsigned char *preLabel,unsigned char* newLabel,const VVPOINTS&localWins,VVPOINTS& newCenters,int winSize, const Wml::GMatrix<Wml::Vector2f>& MeanOptFlwMatrix,const RECT& contrRct )
{
  

	int m_width=srcFrm.GetWidth();
	int m_height=srcFrm.GetHeight();

	int conRctWdh=contrRct.right-contrRct.left+1;
	int conRctHgt=contrRct.top-contrRct.bottom+1;
   // cout<<" contrRct.top="<<contrRct.top<<endl;


	newCenters.clear();
	VPOINTS temVec;
	temVec.clear();
	POINT pTmp;
         pTmp.x=0;
		 pTmp.y=0;
      int m=localWins.size();
	//   cout<<"m= "<<m<<endl;
	  for(int i=0;i<m;++i)
	  {
		  int n=localWins[i].size();
	//	  cout<<"n= "<<n<<endl;
		  for(int j=0;j<n;++j)
		  {
			  int x=localWins[i][j].x;
			  int y=localWins[i][j].y;
			  int winsize=winSize;

			            //mean flow transfer result
			 pTmp.x=__max(contrRct.left,__min(contrRct.right,MeanOptFlwMatrix(x-contrRct.left,y-contrRct.bottom).X()+0.5+x));
		     pTmp.y=__max(contrRct.bottom,__min(contrRct.top,MeanOptFlwMatrix(x-contrRct.left,y-contrRct.bottom).Y()+0.5+y));
			  temVec.push_back(pTmp);

		  }
		  newCenters.push_back(temVec);
	  }


     //test code print mean flow result
    {
		RGBQUAD rgb;
	  CxImage tstNewCtrImg;
	 tstNewCtrImg.Copy(dstFrm);
       int m1=newCenters.size();
	   for(int i=0;i<m1;++i)
	   {
		   int n1=newCenters[i].size();
		   for(int j=0;j<n1;++j)
		   {
              int centerX=newCenters[i][j].x;
			  int centerY=newCenters[i][j].y;
			  rgb.rgbRed=255;
			  rgb.rgbGreen=rgb.rgbBlue=0;
			  tstNewCtrImg.SetPixelColor(centerX,centerY,rgb);
			  int x1=__min(m_width-1,(__max(0,int(centerX-winSize/2))));
			  int y1=__min(m_height-1,(__max(0,int(centerY-winSize/2))));
			  int x2=__min(m_width-1,(__max(0,int(centerX+winSize/2))));
			  int y2=__min(m_height-1,(__max(0,int(centerY+winSize/2))));
			  tstNewCtrImg.DrawLine(x1,x2,y2,y2,rgb);
			  tstNewCtrImg.DrawLine(x2,x2,y2,y1,rgb);
			  tstNewCtrImg.DrawLine(x2,x1,y1,y1,rgb);
			  tstNewCtrImg.DrawLine(x1,x1,y1,y2,rgb);
		   }
	   }
	  tstNewCtrImg.Save("D://tstNewCtrImg.jpg",CXIMAGE_FORMAT_JPG);
      }
	 
  
}