#include "../RotoScoping/RotoScoping.h"


const bool m_bShowImage = false;

RotoScoping::~RotoScoping(){

	cout << "~RotoScoping" << endl;
	delete [] FramesAndPoints;

	for (int i=0;i<NumOfFrame;i++)
	{
		cvReleaseImageHeader(&FilmSeries[i]);//只释放头，因为图像数据体部分不是在本类中申请的。
	}
	delete [] FilmSeries;

	if (rx != NULL)
	{
		for (int i = 0; i < NumOfFrame-2; i++)
		{
			delete[] rx[i];
		}
		//delete[] rx;
	}

	if (ry != NULL)
	{
		for (int i = 0; i < NumOfFrame-2; i++)
		{
			delete[] ry[i];
		}
		//delete[] ry;
	}

	cout << "~RotoScoping over!" << endl;

	//cvReleaseMat(&ControlPointMask);	
	//cvReleaseMat(&BezierAdjustPointsMask);	
	//cvReleaseImage(&CurrentProcessingImageShow);
	//cvReleaseImage(&CurrentProcessingImage);



}


void RotoScoping::Begin(float ** x, float ** y, int numofFrame, int numofCurve, bool ** mask, vector<CSimpleImageb*> pImage){

	//cout << numofFrame << " " << numofCurve << endl;
		 NumOfControlPoint = numofCurve + 1;
		 NumOfFrame = numofFrame;

		 if (!pImage.empty())
		 {
			 Height = pImage[0]->_height;
			 Width  = pImage[0]->_width;
		 }

		 if (pImage.size() != numofFrame)
		 {
			 cout << "Error! Image vector size is not equal to parameter-numofFrame!" << endl;
		 }

		 FilmSeries = new IplImage *[NumOfFrame];
		 FramesAndPoints = new FrameInfo[NumOfFrame];

		 /*if (m_bShowImage)
		 {
			 ImageShow = new IplImage *[NumOfFrame];
		 }*/

		 for (int i = 0; i < numofFrame; i++)
		 {
			FilmSeries[i] = cvCreateImageHeader(cvSize(Width, Height), IPL_DEPTH_8U, 3);
			FilmSeries[i]->imageData = (char*)pImage[i]->_data; //数据又转换了！！～～  有问题 坐标系不同
			/*if (m_bShowImage)
			{
				ImageShow[i] = cvCreateImage(cvSize(Width, Height), IPL_DEPTH_8U, 3);
				for (int h = 0; h < Height; h++)
				 {
					for (int w = 0; w < Width; w++)
					 {
					 uchar * e = pImage[i]->at(w, Height - 1 - h);
					 CV_IMAGE_ELEM(FilmSeries[i], uchar, h, w * 3) = e[0];
					 CV_IMAGE_ELEM(FilmSeries[i], uchar, h, w * 3 + 1) = e[1];
					 CV_IMAGE_ELEM(FilmSeries[i], uchar, h, w * 3 + 2) = e[2];
					}
				}
				cvNamedWindow("s");
				cvShowImage("s", ImageShow[i]);
				cvWaitKey(0);

 			}*/
			/* FilmSeries[i] = cvCreateImage(cvSize(Width, Height), IPL_DEPTH_8U, 3);
			 for (int h = 0; h < Height; h++)
			 {
				 for (int w = 0; w < Width; w++)
				 {
					 uchar * e = pImage[i]->at(w, Height - 1 - h);
					 CV_IMAGE_ELEM(FilmSeries[i], uchar, h, w * 3) = e[0];
					 CV_IMAGE_ELEM(FilmSeries[i], uchar, h, w * 3 + 1) = e[1];
					 CV_IMAGE_ELEM(FilmSeries[i], uchar, h, w * 3 + 2) = e[2];
				 }
			 }*/
		 }

		 for (int i = 0; i < numofFrame; i++)
		 {
			 FramesAndPoints[i].AssignMemory(Width,Height);
			 FramesAndPoints[i].FrameId = i;
			 FramesAndPoints[i].NumOfControlPoint = SAMPLESPERCURVE;
			 for (int j = 0; j < NumOfControlPoint; j++)
			 {
				 FramesAndPoints[i].ControlPoints.push_back(cvPoint(x[i][2*j], y[i][2*j]));
				 FramesAndPoints[i].BezierAdjustPoints.push_back(cvPoint(x[i][2*j+1], y[i][2*j+1]));
			 }
		 }

		 for (int f = 0; f < numofFrame; f++)
		 {
			 for (int i=0;i<NumOfControlPoint-1;i++)
			 {
				 CvPoint2D32f BezierPoint[SAMPLESPERCURVE];

				 CvPoint OppositePoint;
				 OppositePoint.x = FramesAndPoints[f].ControlPoints[i+1].x - (FramesAndPoints[f].BezierAdjustPoints[i+1].x-FramesAndPoints[f].ControlPoints[i+1].x);
				 OppositePoint.y = FramesAndPoints[f].ControlPoints[i+1].y - (FramesAndPoints[f].BezierAdjustPoints[i+1].y-FramesAndPoints[f].ControlPoints[i+1].y);				//注意那个顺序!!!!!!!!!!!!Bezier的调整点在哪一侧对生成的插值点的顺序会有影响!!!!会有问题的!!!!

				 FramesAndPoints[f].CurvePoints.push_back(FramesAndPoints[f].ControlPoints[i]);									//printf("x=%d,y=%d\n",ControlPoints[i].x , ControlPoints[i].y);
				 Bezier::GetInstance()->GenerateCurve(
					 cvPointTo32f(FramesAndPoints[f].ControlPoints[i]),
					 cvPointTo32f(FramesAndPoints[f].BezierAdjustPoints[i]),
					 cvPointTo32f(OppositePoint),
					 cvPointTo32f(FramesAndPoints[f].ControlPoints[i+1]),
					 SAMPLESPERCURVE,
					 BezierPoint);
				 for (int j=0;j<SAMPLESPERCURVE;j++)
				 {
					 FramesAndPoints[f].CurvePoints.push_back(cvPoint(BezierPoint[j].x,BezierPoint[j].y));							//printf("x=%d,y=%d\n",BezierPoint[i].x , BezierPoint[i].y);
				 }

			 }

			 FramesAndPoints[f].CurvePoints.push_back(FramesAndPoints[f].ControlPoints[NumOfControlPoint-1]);
		 }
		 
		 PrepareLMData2();

		/*cout << "Read(1) HandInput(0):";
		int what;
		cin >> what;

		if (what > 0)
		{
	
			ReadAndDrawDataFile();	
			
		}
		else{
			InputInitialKeypoint(FilmSeries[0]);		
			SaveInfoToFrame(FramesAndPoints[0],0);
			AdjustEndFrameKeypoint();
			SaveInfoToFrame(FramesAndPoints[NumOfFrame-1],NumOfFrame-1);

		}	


		PrepareLMData2();*/

	}



void RotoScoping::PrepareLMData2(){

	
		
		cout << "Preparing LMData..." << endl;


		int NumOfPointPerFrame = (SAMPLESPERCURVE + 1) * (NumOfControlPoint - 1) + 1;	

		cout << "NumOfPointPerFrame = " << NumOfPointPerFrame << endl;

		int xLength = 2 * NumOfPointPerFrame * (NumOfFrame -1) +        // EV
					  NumOfFrame * NumOfPointPerFrame +                 // EG
					  (NumOfPointPerFrame-1) * (NumOfFrame-1) +		    // EL
					   2 * (NumOfPointPerFrame-2) * (NumOfFrame-1) +    // EC
			          NumOfPointPerFrame * (NumOfFrame-1) * (2*K+1) *3; // EI
		

		
		int NumOfParam = 4 * NumOfControlPoint * (NumOfFrame-2);	//  一定要乘以4， 因为是一个点要提供x和y两个参数,ControlPoint和AdjustPoint总共两个点，要提供4个参数！！！！
		LMData adata(NumOfControlPoint,NumOfFrame,NumOfPointPerFrame,Width,Height,SAMPLESPERCURVE,xLength,NumOfParam);







		//Calculate  the length of each Energy Term                                   //修改
		adata.nEC = 2 * (NumOfPointPerFrame-2) * (NumOfFrame-1);  // EC
		adata.nEL = (NumOfPointPerFrame-1) * (NumOfFrame-1);   // EL
		adata.nEV = 2 * NumOfPointPerFrame * (NumOfFrame -1);  // EV
		adata.nEG = NumOfFrame * NumOfPointPerFrame;   // EG
		adata.nEI = NumOfPointPerFrame * (NumOfFrame-1) * (2*K+1) *3;  // EI 


		cout << "EC: " << 2 * (NumOfPointPerFrame-2) * (NumOfFrame-1) << endl;
		cout << "EL: " << (NumOfPointPerFrame-1) * (NumOfFrame-1) << endl;
		cout << "EV: " << 2 * NumOfPointPerFrame * (NumOfFrame -1) << endl;
		cout << "EG: " << NumOfFrame * NumOfPointPerFrame << endl;
		cout << "EI: " << NumOfPointPerFrame * (NumOfFrame-1) * (2*K+1) *3 << endl; 
		cout << "xLength: " << xLength << endl;
		cout << "NumOfParam: " << NumOfParam << endl;
		cout << "NumOfCurvePointPerFrame:" << NumOfPointPerFrame << endl;



		
		//金字塔的最高层数
		int LowestLevelRatio = pow(2.0,MaxPyramidLevel-1);
		//这里的p是要传入去levmar里面那个，设置最底层的参数为线性插值
		double *p = new double[NumOfParam];
		int pcounter=0;
		for (int f=0;f<NumOfFrame-2;f++)	// 只有夹在第一帧和最后一帧的帧我们才需要考虑，所以要减去2
		{
			for (int pid = 0;pid<NumOfControlPoint;pid++)
			{
				//不能都设置为第一帧的位置，要线性插值比较好。优化问题的初值很重要。我们认为只要两帧间的时间差足够小，那么都是可以认为是是线性运动的。
				// Use linear interpolation				
				p[pcounter++] = (f+1)*(FramesAndPoints[NumOfFrame - 1].ControlPoints[pid].x/LowestLevelRatio - FramesAndPoints[0].ControlPoints[pid].x/LowestLevelRatio) / double(NumOfFrame-1) + FramesAndPoints[0].ControlPoints[pid].x/LowestLevelRatio;
				p[pcounter++] = (f+1)*(FramesAndPoints[NumOfFrame - 1].ControlPoints[pid].y/LowestLevelRatio - FramesAndPoints[0].ControlPoints[pid].y/LowestLevelRatio) / double(NumOfFrame-1) + FramesAndPoints[0].ControlPoints[pid].y/LowestLevelRatio;

				p[pcounter++] = (f+1)*(FramesAndPoints[NumOfFrame - 1].BezierAdjustPoints[pid].x/LowestLevelRatio - FramesAndPoints[0].BezierAdjustPoints[pid].x/LowestLevelRatio) / double(NumOfFrame-1) + FramesAndPoints[0].BezierAdjustPoints[pid].x/LowestLevelRatio;
				p[pcounter++] = (f+1)*(FramesAndPoints[NumOfFrame - 1].BezierAdjustPoints[pid].y/LowestLevelRatio - FramesAndPoints[0].BezierAdjustPoints[pid].y/LowestLevelRatio) / double(NumOfFrame-1) + FramesAndPoints[0].BezierAdjustPoints[pid].y/LowestLevelRatio;
			}
		}
		//这里的x是要传入去levmar里面那个
		double *x = new double[xLength]; 
		memset(x,0,sizeof(double)*xLength);
		
		//set fix mark
		for (int pid=0;pid<NumOfControlPoint;pid++)
		{
			adata.FixMask[pid][0] = true;
			adata.FixMask[pid][NumOfFrame-1] = true;
		}

		//生成用来暂存的金字塔的空间 
		IplImage** BufferImage = new IplImage*[MaxPyramidLevel];
		for (int i=1,pow2 = 2;i<MaxPyramidLevel;i++,pow2*=2)
		{
			BufferImage[i] = cvCreateImage(	//这个图像是用来将原始大小的图像缩小到当前尺度时所用的缓存区
				cvSize(Width/pow2,Height/pow2),
				FilmSeries[0]->depth,
				FilmSeries[0]->nChannels);
		}

		//金字塔从这里开始
		for (int PyrLevel = MaxPyramidLevel - 1; PyrLevel>=0; PyrLevel--)
		{
			int Ratio = int(pow(2.0,PyrLevel));
			int CurrentWidth = Width / Ratio;
			int CurrentHeight = Height / Ratio;
			adata.Width = CurrentWidth;
			adata.Height = CurrentHeight;
			
			// 由于图像的大小变了，所以要重新分配RBG的空间
			for (int f=0;f<NumOfFrame;f++)
			{
				delete[]adata.R[f];
				delete[]adata.G[f];
				delete[]adata.B[f];
				delete[]adata.Gradient[f];
				adata.R[f] = new unsigned char[CurrentWidth*CurrentHeight];
				adata.G[f] = new unsigned char[CurrentWidth*CurrentHeight];
				adata.B[f] = new unsigned char[CurrentWidth*CurrentHeight];
				adata.Gradient[f] = new double[CurrentWidth*CurrentHeight];
			}



			// set xCp , yCp and xAp , yAp
			for (int pid=0;pid<NumOfControlPoint;pid++)
			{
				adata.xCp[pid][0] = FramesAndPoints[0].ControlPoints[pid].x / Ratio;
				adata.yCp[pid][0] = FramesAndPoints[0].ControlPoints[pid].y / Ratio;
				adata.xAp[pid][0] = FramesAndPoints[0].BezierAdjustPoints[pid].x / Ratio;
				adata.yAp[pid][0] = FramesAndPoints[0].BezierAdjustPoints[pid].y / Ratio;

				adata.xCp[pid][NumOfFrame-1] = FramesAndPoints[NumOfFrame-1].ControlPoints[pid].x / Ratio;
				adata.yCp[pid][NumOfFrame-1] = FramesAndPoints[NumOfFrame-1].ControlPoints[pid].y / Ratio;
				adata.xAp[pid][NumOfFrame-1] = FramesAndPoints[NumOfFrame-1].BezierAdjustPoints[pid].x / Ratio;
				adata.yAp[pid][NumOfFrame-1] = FramesAndPoints[NumOfFrame-1].BezierAdjustPoints[pid].y / Ratio;
			}



			//set x and y
			for (int pid=0;pid<NumOfPointPerFrame;pid++)
			{
				adata.x[pid][0] = FramesAndPoints[0].CurvePoints[pid].x / Ratio;
				adata.y[pid][0] = FramesAndPoints[0].CurvePoints[pid].y / Ratio;

				adata.x[pid][NumOfFrame-1] = FramesAndPoints[NumOfFrame-1].CurvePoints[pid].x / Ratio;
				adata.y[pid][NumOfFrame-1] = FramesAndPoints[NumOfFrame-1].CurvePoints[pid].y / Ratio;
			}
		
			//看看采样方向对不对
			{
				int Opx,Opy;
				CvPoint2D32f *CurvePoint = new CvPoint2D32f[adata.IntermediateSamplePerCurve+2];	//这里是加2，因为包括了第一个和最后一个点
				Normal *normals= new Normal[adata.IntermediateSamplePerCurve+2];	//这里是加2，因为包括了第一个和最后一个点
				IplImage * NormalImage = cvCreateImage(cvSize(CurrentWidth,CurrentHeight),8,3);
				
			
				//查看第一帧的采样方向
					BufferImage[0] = FilmSeries[0];			//Pyrlevel=0的时候就是原图
					for (int i=1;i<=PyrLevel;i++)
					{
						cvPyrDown(BufferImage[i-1],BufferImage[i]);
					}
					cvCopy(BufferImage[PyrLevel],NormalImage);
					for (int cpid = 0;cpid<adata.NumOfControlPoint - 1;cpid++)
					{	
						

						Opx = adata.xCp[cpid+1][0] - (adata.xAp[cpid+1][0] - adata.xCp[cpid+1][0]);
						Opy = adata.yCp[cpid+1][0] - (adata.yAp[cpid+1][0] - adata.yCp[cpid+1][0]);

						Bezier::GetInstance()->GenerateCurveAndNormal(cvPoint2D32f(adata.xCp[cpid][0],adata.yCp[cpid][0]),
							cvPoint2D32f(adata.xAp[cpid][0],adata.yAp[cpid][0]),
							cvPoint2D32f(Opx,Opy),
							cvPoint2D32f(adata.xCp[cpid+1][0],adata.yCp[cpid+1][0]),
							adata.IntermediateSamplePerCurve,
							CurvePoint,
							normals);

						//在图像中画出这个法向量
						for (int i=0;i<adata.IntermediateSamplePerCurve + 2;i++)
						{
							cvLine(NormalImage,cvPoint(CurvePoint[i].x,CurvePoint[i].y),cvPoint(CurvePoint[i].x + 10*normals[i].NormalX,CurvePoint[i].y + 10*normals[i].NormalY),cvScalar(255,0,255),1);
							cvCircle(NormalImage,cvPoint(CurvePoint[i].x,CurvePoint[i].y),2,cvScalar(0,255,0),-1);			
						}
					}
					if (m_bShowImage)
					{
						cvNamedWindow("NormalImage");
						cvShowImage("NormalImage",NormalImage);
						cvWaitKey(0);
					}
				

				//查看最后一帧的采样方向
					BufferImage[0] = FilmSeries[NumOfFrame-1];			//Pyrlevel=0的时候就是原图
					for (int i=1;i<=PyrLevel;i++)
					{
						cvPyrDown(BufferImage[i-1],BufferImage[i]);
					}
					cvCopy(BufferImage[PyrLevel],NormalImage);
					for (int cpid = 0;cpid<adata.NumOfControlPoint - 1;cpid++)
					{	
						Opx = adata.xCp[cpid+1][adata.NumOfFrame-1] - (adata.xAp[cpid+1][adata.NumOfFrame-1] - adata.xCp[cpid+1][adata.NumOfFrame-1]);
						Opy = adata.yCp[cpid+1][adata.NumOfFrame-1] - (adata.yAp[cpid+1][adata.NumOfFrame-1] - adata.yCp[cpid+1][adata.NumOfFrame-1]);

						Bezier::GetInstance()->GenerateCurveAndNormal(cvPoint2D32f(adata.xCp[cpid][adata.NumOfFrame-1],adata.yCp[cpid][adata.NumOfFrame-1]),
							cvPoint2D32f(adata.xAp[cpid][adata.NumOfFrame-1],adata.yAp[cpid][adata.NumOfFrame-1]),
							cvPoint2D32f(Opx,Opy),
							cvPoint2D32f(adata.xCp[cpid+1][adata.NumOfFrame-1],adata.yCp[cpid+1][adata.NumOfFrame-1]),
							adata.IntermediateSamplePerCurve,
							CurvePoint,
							normals);

						//在图像中画出这个法向量
						for (int i=0;i<adata.IntermediateSamplePerCurve + 2;i++)
						{
							cvLine(NormalImage,cvPoint(CurvePoint[i].x,CurvePoint[i].y),cvPoint(CurvePoint[i].x + 10*normals[i].NormalX,CurvePoint[i].y + 10*normals[i].NormalY),cvScalar(255,0,255),1);
							cvCircle(NormalImage,cvPoint(CurvePoint[i].x,CurvePoint[i].y),2,cvScalar(0,255,0),-1);			
						}
					}
					if (m_bShowImage)
					{
						cvNamedWindow("NormalImage1");
						cvShowImage("NormalImage1",NormalImage);
						cvWaitKey(0);
					}
					
					//cvReleaseImage(&NormalImage);
				delete [] CurvePoint;
				delete [] normals;




			}

			//set R G B
			{										
				IplImage *B = cvCreateImage(cvSize(CurrentWidth,CurrentHeight),8,1);
				IplImage *G = cvCreateImage(cvSize(CurrentWidth,CurrentHeight),8,1);
				IplImage *R = cvCreateImage(cvSize(CurrentWidth,CurrentHeight),8,1);
				unsigned char * Bptr = (unsigned char *)B->imageData;
				unsigned char * Gptr = (unsigned char *)G->imageData;
				unsigned char * Rptr = (unsigned char *)R->imageData;
			

				int WidthStep = R->widthStep;
				for (int f=0;f<NumOfFrame;f++)
				{
					BufferImage[0] = FilmSeries[f];			//Pyrlevel=0的时候就是原图
					for (int i=1;i<=PyrLevel;i++)
					{
						cvPyrDown(BufferImage[i-1],BufferImage[i]);
					}
					
					

					//for (int pid = 0; pid < NumOfPointPerFrame; pid++)
					//{
					//	cvCircle(BufferImage[PyrLevel],cvPoint(adata.x[pid][0],adata.y[pid][0]),2,cvScalar(0,0,255),1);
					//	cvCircle(BufferImage[PyrLevel],cvPoint(adata.x[pid][NumOfFrame-1],adata.y[pid][NumOfFrame-1]),2,cvScalar(255,0,0),1);
					//}
					//for (int pid = 0; pid < NumOfControlPoint; pid++)
					//{
					//	cvCircle(BufferImage[PyrLevel],cvPoint(adata.xCp[pid][0],adata.yCp[pid][0] ),2,cvScalar(0,255,0),1);
					//	cvCircle(BufferImage[PyrLevel],cvPoint(adata.xCp[pid][NumOfFrame-1],adata.yCp[pid][NumOfFrame-1] ),2,cvScalar(255,255,0),1);
					//	cvCircle(BufferImage[PyrLevel],cvPoint(adata.xAp[pid][0],adata.yAp[pid][0] ),2,cvScalar(0,255,255),1);
					//	cvCircle(BufferImage[PyrLevel],cvPoint(adata.xAp[pid][NumOfFrame-1],adata.yAp[pid][NumOfFrame-1] ),2,cvScalar(255,0,255),1);
					//}
					//cvShowImage("CurrentLevel",BufferImage[PyrLevel]);
					//cvWaitKey(27);

					cvSplit(BufferImage[PyrLevel],B,G,R,NULL);		//FilmSeries[f]改成BufferImage[PyrLevel]
					Bptr = (unsigned char *)B->imageData;
					Gptr = (unsigned char *)G->imageData;
					Rptr = (unsigned char *)R->imageData;
					for (int h = 0;h<CurrentHeight;h++)	//这里将Height改成了CurrentHeight
					{
						memcpy(adata.B[f] + CurrentWidth * h, Bptr + WidthStep*h,sizeof(unsigned char)*CurrentWidth);	//Width改成CurrentWidth
						memcpy(adata.G[f] + CurrentWidth * h, Gptr + WidthStep*h,sizeof(unsigned char)*CurrentWidth);
						memcpy(adata.R[f] + CurrentWidth * h, Rptr + WidthStep*h,sizeof(unsigned char)*CurrentWidth);
					}
				}
				cvReleaseImage(&B);
				cvReleaseImage(&G);
				cvReleaseImage(&R);
			}

			//set Gradient
			{
				int ThisIndex;
				double dx,dy,g;
				for (int f=0;f<NumOfFrame;f++)
				{
					for (int h=0;h<CurrentHeight;h++)			//Height改成CurrentHeight
					{
					
						for (int w=0;w<CurrentWidth;w++)
						{
							g=0.0;		//   Important!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
							ThisIndex = CurrentWidth*h +  w;

							dx = ConvolveX(adata.B[f],w,h,CurrentWidth,CurrentHeight);
							dy = ConvolveY(adata.B[f],w,h,CurrentWidth,CurrentHeight);
							g += sqrt(dx*dx + dy*dy);
 
							dx = ConvolveX(adata.G[f],w,h,CurrentWidth,CurrentHeight);
							dy = ConvolveY(adata.G[f],w,h,CurrentWidth,CurrentHeight);
							g += sqrt(dx*dx + dy*dy);

							dx = ConvolveX(adata.R[f],w,h,CurrentWidth,CurrentHeight);
							dy = ConvolveY(adata.R[f],w,h,CurrentWidth,CurrentHeight);
							g += sqrt(dx*dx + dy*dy);										
							adata.Gradient[f][ThisIndex] = g;					
						}
					}
					//cvNamedWindow("Gradient");
					//IplImage *Gradient = cvCreateImage(cvSize(CurrentWidth,CurrentHeight),8,1);
					//int WidthStep = Gradient->widthStep;
					//unsigned char * Bptr = (unsigned char *)Gradient->imageData;

					//for (int h=0;h<CurrentHeight-1;h++)
					//{
					//	for (int w=0;w<CurrentWidth-1;w++)
					//	{
					//		Bptr[WidthStep*h + w] = adata.Gradient[f][CurrentWidth*h + w];
					//	}
					//}
					//cvShowImage("Gradient",Gradient);
					//cvWaitKey(0);
				}
			}

			// set M
			{
				double KMax = 1200;   //KMAX = sqrt(255.0*255.0*2.0)*3 = 1081
				int indexta;
				int indextb;
				double Gpta,Gptb;
				for (int pid =0;pid < NumOfPointPerFrame;pid++)
				{

					indexta = int(adata.y[pid][0]) * CurrentWidth + int(adata.x[pid][0]) ;		//将width改成CurrentWidth
					indextb = int(adata.y[pid][NumOfFrame-1]) * CurrentWidth + int(adata.x[pid][NumOfFrame-1]) ;
				

					Gpta = KMax - adata.Gradient[0][indexta];
					Gptb = KMax - adata.Gradient[NumOfFrame-1][indextb];

					if (Gpta < Gptb )
					{
						adata.M[pid] = Gpta;
					}
					else{
						adata.M[pid] = Gptb;
					}
				}
			}

		



			/************************************************************************************************************************************************
																		Begin Levmar
																
																	The arrangement of p:
					------------------------------------------------------------------------------------------------------------
							(xit,yit)......................................(xit_1,yit_1)......................
					------------------------------------------------------------------------------------------------------------
			************************************************************************************************************************************************/


		cout << "prepare over!!" << endl;
			clock_t start, stop; /* clock_t is a built-in type for processor time (ticks) */
			double duration; /* records the run time (seconds) of a function */

			double opts[LM_OPTS_SZ], info[LM_INFO_SZ];
			opts[0]=LM_INIT_MU; opts[1]=1E-15; opts[2]=1E-15; opts[3]=1E-20;opts[4]=LM_DIFF_DELTA;
			cout << "Begin Levmar!" << endl;
		
			start = clock(); /* records the ticks at the beginning of the function call */
			dlevmar_der(lmfunc, lmjacfunc, p, x, NumOfParam, xLength, 1000, opts, info, NULL, NULL, &adata); // with analytic Jacobian
			stop = clock(); /* records the ticks at the end of the function call */
			duration = ((double)(stop - start))/CLK_TCK;		

			printf("Levenberg-Marquardt returned in %g iter, reason %g, sumsq %g [%g]\n", info[5], info[6], info[1], info[0]);
			printf("Time Duration =%fs\n",duration);


			//最后要将参数传递到下一层
			for (int i=0;i<NumOfParam;i++)
			{
				p[i] = p[i] * 2;
			}

		}

		for (int i=0;i<NumOfParam;i++)
		{
			p[i] = p[i] / 2;
		}
		rx = new float *[NumOfFrame-2];
		ry = new float *[NumOfFrame-2];

		for (int i = 0; i < NumOfFrame-1; i++)
		{
			rx[i] = new float[2*NumOfControlPoint];
			ry[i] = new float[2*NumOfControlPoint];
		}

		for (int f = 1; f < NumOfFrame - 1; f++)
		{
			for (int pid = 0; pid < NumOfControlPoint; pid++)
			{
				rx[f-1][2*pid] = adata.xCp[pid][f];//rx， ry为result，只保存非关键帧的数据。
				ry[f-1][2*pid] = adata.yCp[pid][f];
				rx[f-1][2*pid+1] = adata.xAp[pid][f];
				ry[f-1][2*pid+1] = adata.yAp[pid][f];
			}
		}
		//显示结果
		int n = 10;
		while(0){
			IplImage *Gradient = cvCreateImage(cvSize(adata.Width,adata.Height),8,3);
			
			
			

			pcounter=0;
			for (int f=1;f<NumOfFrame-1;f++)
			{
				int WidthStep = Gradient->widthStep;
				unsigned char * Bptr = (unsigned char *)Gradient->imageData;

				for (int h=0;h<adata.Height-1;h++)
				{
					for (int w=0;w<adata.Width-1;w++)
					{
						Bptr[WidthStep*h + 3*w] =Bptr[WidthStep*h + 3*w + 1] = Bptr[WidthStep*h + 3*w + 2]= adata.Gradient[f][adata.Width*h + w];
					}
				}
				for (int pid = 0; pid < NumOfPointPerFrame; pid++)
				{
					cvCircle(Gradient,cvPoint(adata.x[pid][f],adata.y[pid][f]),2,cvScalar(0,0,255),1);
				}
				cout << "lmResult" << endl;
				for (int pid = 0; pid < NumOfControlPoint; pid++)
				{
					cvCircle(Gradient,cvPoint(adata.xCp[pid][f],adata.yCp[pid][f] ),2,cvScalar(0,255,0),1);
					cvCircle(Gradient,cvPoint(adata.xAp[pid][f],adata.yAp[pid][f] ),2,cvScalar(0,255,255),1);
					cout << adata.xCp[pid][f] << " " << adata.yCp[pid][f] << endl;
					cout << adata.xAp[pid][f] << " " << adata.yAp[pid][f] << endl;

				}
				if (m_bShowImage)
				{
					cvNamedWindow("Gradient");
					cvShowImage("Gradient",Gradient);
					cvWaitKey(0);
				}
				
				/*cvReleaseImage(&Gradient);
				cvCopy(FilmSeries[f],CurrentProcessingImageShow);
				ControlPoints.clear();
				BezierAdjustPoints.clear();
				for (int j=0;j<NumOfControlPoint;j++)
				{
					ControlPoints.push_back(cvPoint(p[pcounter],p[pcounter+1]));
					pcounter+=2;
					BezierAdjustPoints.push_back(cvPoint(p[pcounter],p[pcounter+1]));
					pcounter+=2;
				}
				
				cvNamedWindow("IntermediateFrame");
				cvShowImage("IntermediateFrame",CurrentProcessingImageShow);
				cvWaitKey(0);*/
			}
		}
		

		for (int i=1,pow2 = 2;i<MaxPyramidLevel;i++,pow2*=2)
		{
			cvReleaseImage(&BufferImage[i]);
		}

		delete[]p;
		delete[]x;
		//system("pause");
	}

	void RotoScoping::getRx(float ** dst)
	{
		if (rx == NULL)
		{
			cout << "result is NULL!" << endl;
			exit(0);
		}
		for (int i = 0; i < NumOfFrame - 2; i++)
		{
			for (int j = 0; j < 2 * NumOfControlPoint; j++)
			{
				dst[i][j] = rx[i][j];
			}
		}
	}
	void RotoScoping::getRy(float ** dst)
	{
		if (ry == NULL)
		{
			cout << "result is NULL!" << endl;
			exit(0);
		}
		for (int i = 0; i < NumOfFrame - 2; i++)
		{
			for (int j = 0; j < 2 * NumOfControlPoint; j++)
			{
				dst[i][j] = ry[i][j];
			}
		}
	}
