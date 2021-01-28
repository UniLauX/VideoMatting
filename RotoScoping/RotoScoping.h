
#ifndef ROTOSCOPING_H
#define ROTOSCOPING_H


#include "../RotoScoping/PublicStructures.h"
#include "../RotoScoping/PointFrameInfo.h"
#include "../RotoScoping/LMFunc.h"
#include "simpleimage.h"

#include "../RotoScoping/Configuration.h"
//#include "../RotoScoping/Bezier.h"
#include <time.h>
#define BlobMaskSize  8			//做交互用
#define CurvePointRadius 1		//做交互用
#define ControlPointRadius 3	//做交互用
#define LineWidth 1				//做交互用



class RotoScoping{
public:


	//friend void MouseCallbackFuncFirstFrame();
	//friend void MouseCallbackFuncOtherFrame();
	//friend void MouseCallbackFuncDisplacement();

	FrameInfo * FramesAndPoints;
	int NumOfFrame;
	int NumOfControlPoint;
	int Width;
	int Height;

	int MaxPyramidLevel;
	



	//CvMat * ControlPointMask;			// Store the index of a Control Point in a given pixel. If this pixel is not a Control Point, then its values is -1
	//CvMat * BezierAdjustPointsMask;		// Store the index of a Adjust  Point in a given pixel. If this pixel is not a Adjust  Point, then its values is -1


	vector<CvPoint>	ControlPoints;		// point specified by the user (red)
	vector<CvPoint> BezierAdjustPoints; // point use to adjust the Bezier curve (yellow)  
	/******** The size of ControlPoints should be the same as that of the BezierAdjustmentPoints********/
	//vector<CvPoint> CurvePoints;		// point on the Bezier curve (blue)
	IplImage ** FilmSeries;				// 用来读入的所有的图片,析构时只需处理头
	//IplImage * CurrentProcessingImage;
	//IplImage * CurrentProcessingImageShow;
	float ** rx;
	float ** ry;






	RotoScoping(){
		rx = NULL;
		ry = NULL;
		MaxPyramidLevel = 3;

	}
	~RotoScoping();

		//NumOfFrame=0;
		//while(NumOfFrame==0){
		//cout << "NumOfFrame = "<<  endl;
		//cin >> NumOfFrame;
		//}

		//FramesAndPoints = new FrameInfo[NumOfFrame];
		//
		//FilmSeries = new IplImage*[NumOfFrame];
		//for (int i=0;i<NumOfFrame;i++)
		//{
		//	char filename[100];
		//	sprintf(filename,"E:\\personalStuff\\Desktop\\RotoScoping\\VideoSequence\\VideoSequence\\%03d.png",i);
		//	//sprintf(filename,"F:\\personalStuff\\Desktop\\RotoScoping\\VideoSequence\\VideoSequence2\\frame%03d.png",i+1);
		//	//sprintf(filename,"E:\\personalStuff\\Desktop\\RotoScoping\\VideoSequence\\%03d.jpg",i);
		//	FilmSeries[i] = cvLoadImage(filename);
		//	cout << filename<< "   Readed!" << endl;
		//}


		//Width = FilmSeries[0]->width;
		//Height = FilmSeries[0]->height;
		//for (int i=0;i<NumOfFrame;i++)
		//{
		//	FramesAndPoints[i].AssignMemory(Width,Height);
		//}


		//
		//CurrentProcessingImage = cvCreateImage(cvSize(Width,Height),8,3);
		//CurrentProcessingImageShow = cvCreateImage(cvSize(Width,Height),8,3);
		//
		//ControlPointMask = cvCreateMat(Height,Width,CV_32SC1);	// CV_32SC1 is int
		//memset(ControlPointMask->data.i,-1,sizeof(int) * Height * Width);
		//BezierAdjustPointsMask = cvCreateMat(Height,Width,CV_32SC1);	// CV_32SC1 is int
		//memset(BezierAdjustPointsMask->data.i,-1,sizeof(int) * Height * Width);


	void Begin(float ** x, float ** y, int numofFrame, int numofCurve, bool ** mask, vector<CSimpleImageb*> pImage);

	void PrepareLMData2();
	inline void setPyrLevel(int l){MaxPyramidLevel = l;}

	void getRx( float ** dst );
	void getRy( float ** dst );



};
	

	//int GetControlPointId(int x,int y){
	//	return ControlPointMask->data.i[y*Width + x];
	//}
	//int GetAdjustPointId(int x,int y){
	//	return BezierAdjustPointsMask->data.i[y*Width + x];
	//}
	//
	//void EraseAllMask(){
	//	for (int i=0;i<ControlPoints.size();i++)
	//	{
	//		cvCircle(ControlPointMask,ControlPoints[i],BlobMaskSize,cvScalar(-1),-1);
	//		cvCircle(BezierAdjustPointsMask,BezierAdjustPoints[i],BlobMaskSize,cvScalar(-1),-1);
	//	}
	//}
	//void UpdateAllMask(){
	//	for (int i=0;i<ControlPoints.size();i++)
	//	{
	//		cvCircle(ControlPointMask,ControlPoints[i],BlobMaskSize,cvScalar(i),-1);
	//		cvCircle(BezierAdjustPointsMask,BezierAdjustPoints[i],BlobMaskSize,cvScalar(i),-1);
	//	}
	//}
	//void AllPointDisplace(int dx,int dy){

	//	for (int i=0;i<ControlPoints.size();i++)
	//	{
	//		ControlPoints[i].x += dx;
	//		ControlPoints[i].y += dy;
	//		BezierAdjustPoints[i].x += dx;
	//		BezierAdjustPoints[i].y += dy;
	//	}

	//}

	//void UpdateControlPoint(int Id,int xNew,int yNew){

	//	ControlPoints[Id].x = xNew;
	//	ControlPoints[Id].y = yNew;

	//
	//}

	//void UpdateAdjustPoint(int Id,int xNew,int yNew){

	//	BezierAdjustPoints[Id].x = xNew;
	//	BezierAdjustPoints[Id].y = yNew;
	//	
	//}

	//void AdjustEndFrameKeypoint(){
	//	cvNamedWindow("EndFrame");
	//	cvSetMouseCallback("EndFrame",MouseCallbackFuncOtherFrame,this);
	//	cvCopy(FilmSeries[NumOfFrame-1],CurrentProcessingImage);
	//	cvCopy(FilmSeries[NumOfFrame-1],CurrentProcessingImageShow);

	//	DrawCurve();


	//	cout << "Press 'b' to save and finish.\n Press 'p' to change to the Point Adjust Mode. \nPress 'd' to change to Displacement Mode\n" << endl;

	//	while (1)
	//	{
	//		int keypress = cvWaitKey(27);
	//		cvShowImage("EndFrame",CurrentProcessingImageShow);
	//		if (keypress == 'b')
	//		{
	//			break;
	//		}
	//		else if (keypress == 'd')
	//		{
	//			cout << "Displacement Mode" << endl;
	//			cvSetMouseCallback("EndFrame",MouseCallbackFuncDisplacement,this);
	//		}
	//		else if (keypress == 'p')
	//		{
	//			cout << "Point Adjust Mode" << endl;
	//			cvSetMouseCallback("EndFrame",MouseCallbackFuncOtherFrame,this);
	//		}
	//	}

	//	SaveEndFrameDataToFile();
	//}

	//void InputInitialKeypoint(IplImage *img){


	//	cvNamedWindow("FirstFrame");
	//	cvSetMouseCallback("FirstFrame",MouseCallbackFuncFirstFrame,this);
	//	cvCopy(img,CurrentProcessingImage);
	//	cvCopy(img,CurrentProcessingImageShow);


	//	cout << "Press 'b' to finish drawing..." << endl;
	//	while(1){
	//		int keypress = cvWaitKey(27);
	//		cvShowImage("FirstFrame",CurrentProcessingImageShow);
	//		if (keypress == 'b')
	//		{
	//			break;
	//		}
	//	}



	//	NumOfControlPoint = BezierAdjustPoints.size();


	//	cout << "NumOfPoint = " <<NumOfControlPoint << endl;
	//	SaveFirstFrameDataToFile();
	//	



	//}

	//void DrawCurve(){

	//	if (ControlPoints.size()==1)	// it only have 1 point,i.e, at the beginning of th stage;
	//	{
	//		cvLine(CurrentProcessingImageShow,ControlPoints[0],BezierAdjustPoints[0],cvScalar(0,255,0),LineWidth);  //green
	//		cvCircle(CurrentProcessingImageShow,ControlPoints[0],ControlPointRadius,cvScalar(0,0,255),-1);				    //red
	//		cvCircle(CurrentProcessingImageShow,BezierAdjustPoints[0],ControlPointRadius,cvScalar(0,255,255),-1);				//yellow
	//	}
	//	else if(ControlPoints.size()>1){
	//		for (int i=0;i<ControlPoints.size()-1;i++)
	//		{
	//			CvPoint2D32f BezierPoint[SAMPLESPERCURVE];

	//			CvPoint OppositePoint;
	//			OppositePoint.x = ControlPoints[i+1].x - (BezierAdjustPoints[i+1].x-ControlPoints[i+1].x);
	//			OppositePoint.y = ControlPoints[i+1].y - (BezierAdjustPoints[i+1].y-ControlPoints[i+1].y);				//注意那个顺序!!!!!!!!!!!!Bezier的调整点在哪一侧对生成的插值点的顺序会有影响!!!!会有问题的!!!!

	//			
	//			Bezier::GetInstance()->GenerateCurve(
	//				cvPointTo32f(ControlPoints[i]),
	//				cvPointTo32f(BezierAdjustPoints[i]),
	//				cvPointTo32f(OppositePoint),
	//				cvPointTo32f(ControlPoints[i+1]),
	//				SAMPLESPERCURVE,
	//				BezierPoint);
	//			for (int i=0;i<SAMPLESPERCURVE;i++)
	//			{
	//				cvCircle(CurrentProcessingImageShow,cvPoint(BezierPoint[i].x,BezierPoint[i].y),CurvePointRadius,cvScalar(0,255,255),-1);			    // Bezier points are blue
	//			}
	//		
	//		}

	//		for (int i=0;i<ControlPoints.size();i++)
	//		{

	//			CvPoint OppositePoint;
	//			OppositePoint.x = ControlPoints[i].x - (BezierAdjustPoints[i].x-ControlPoints[i].x);
	//			OppositePoint.y = ControlPoints[i].y - (BezierAdjustPoints[i].y-ControlPoints[i].y);		

	//			cvLine(CurrentProcessingImageShow,ControlPoints[i],OppositePoint,cvScalar(0,255,0),LineWidth);  //green
	//			cvLine(CurrentProcessingImageShow,ControlPoints[i],BezierAdjustPoints[i],cvScalar(0,255,0),LineWidth);  //green
	//			cvCircle(CurrentProcessingImageShow,ControlPoints[i],ControlPointRadius,cvScalar(0,0,255),-1);				    //red
	//			cvCircle(CurrentProcessingImageShow,BezierAdjustPoints[i],ControlPointRadius,cvScalar(0,255,255),-1);				//yellow
	//		}


	//	}
	//}
	//
	//void SaveFirstFrameDataToFile(){



	//	if (ControlPoints.size()==1)	// it only have 1 point,i.e, at the beginning of th stage;
	//	{
	//		cout << "Only got one point, meaningless!" << endl;
	//	}
	//	else if(ControlPoints.size()>1){

	//		ofstream OutputFile;
	//		OutputFile.open("E:\\personalStuff\\Desktop\\RotoScoping\\record.txt");


	//		OutputFile << "<Point Size>" << endl;
	//		OutputFile << ControlPoints.size() << endl;
	//		OutputFile << "</Point Size>" << endl;
	//		
	//		OutputFile << "<First Frame>" << endl;

	//		OutputFile << "<Control Points>" << endl;
	//		for (int i=0;i<ControlPoints.size();i++)
	//		{
	//			OutputFile << ControlPoints[i].x << " " << ControlPoints[i].y << endl;

	//		}
	//		OutputFile << "</Control Points>" << endl;


	//		OutputFile << "<Adjust Points>" << endl;
	//		for (int i=0;i<BezierAdjustPoints.size();i++)
	//		{
	//			OutputFile << BezierAdjustPoints[i].x << " " << BezierAdjustPoints[i].y << endl;
	//		}
	//		OutputFile << "</Adjust Points>" << endl;

	//		OutputFile << "</First Frame>" << endl;
	//		
	//		


	//		
	//		OutputFile.close();
	//	}

	//}

	//void SaveEndFrameDataToFile(){
	//	ofstream OutputFile;
	//	OutputFile.open("E:\\personalStuff\\Desktop\\RotoScoping\\record.txt",ios::app);


	//	OutputFile << "<End Frame>" << endl;

	//	OutputFile << "<Control Points>" << endl;
	//	for (int i=0;i<ControlPoints.size();i++)
	//	{
	//		OutputFile << ControlPoints[i].x << " " << ControlPoints[i].y << endl;

	//	}
	//	OutputFile << "</Control Points>" << endl;


	//	OutputFile << "<Adjust Points>" << endl;
	//	for (int i=0;i<BezierAdjustPoints.size();i++)
	//	{
	//		OutputFile << BezierAdjustPoints[i].x << " " << BezierAdjustPoints[i].y << endl;
	//	}
	//	OutputFile << "</Adjust Points>" << endl;

	//	OutputFile << "</End Frame>" << endl;



	//	OutputFile.close();
	//}


	//void ReadAndDrawDataFile(){








	//	//ifstream in("E:\\personalStuff\\Desktop\\RotoScoping\\record_mP.txt"); // bug
	//	ifstream in("E:\\personalStuff\\Desktop\\RotoScoping\\record_test.txt");
	//	string line;
	//	




	//	// read in the first frame
	//	getline(in,line);
	//	in >> NumOfControlPoint;
	//	getline(in,line);
	//	getline(in,line);
	//	getline(in,line);
	//	getline(in,line);


	//	cout <<"NumOfControlPoint = " <<  NumOfControlPoint << endl;
	//	
	//	int x;
	//	int y;
	//	CvPoint Coor;
	//	for (int i=0;i<NumOfControlPoint;i++)
	//	{
	//		in >>x >>y;
	//		ControlPoints.push_back(cvPoint(x,y));
	//		cvCircle(ControlPointMask,cvPoint(x,y),BlobMaskSize,cvScalar(i),-1);
	//		
	//	}

	//	getline(in,line);
	//	getline(in,line);
	//	getline(in,line);


	//	for (int i=0;i<NumOfControlPoint;i++)
	//	{
	//		in >>Coor.x >>Coor.y;
	//		BezierAdjustPoints.push_back(Coor);
	//		cvCircle(BezierAdjustPointsMask,Coor,BlobMaskSize,cvScalar(i),-1);
	//		
	//	}


	//	SaveInfoToFrame(FramesAndPoints[0],0);

	//	cvCopy(FilmSeries[0],CurrentProcessingImage);
	//	cvCopy(FilmSeries[0],CurrentProcessingImageShow);
	//	DrawCurve();
	//	cvNamedWindow("FirstFrame");
	//	cvShowImage("FirstFrame",CurrentProcessingImageShow);
	//	cvWaitKey(27);





	//	// read in the end frame
	//	memset(BezierAdjustPointsMask->data.i,-1,sizeof(int)*Width*Height);
	//	memset(ControlPointMask->data.i,-1,sizeof(int)*Width*Height);
	//	getline(in,line);
	//	getline(in,line);
	//	getline(in,line);
	//	getline(in,line);
	//	getline(in,line);

	//	ControlPoints.clear();
	//	BezierAdjustPoints.clear();
	//	
	//	for (int i=0;i<NumOfControlPoint;i++)
	//	{
	//		in >>x >>y;
	//		ControlPoints.push_back(cvPoint(x,y));
	//		cvCircle(ControlPointMask,cvPoint(x,y),BlobMaskSize,cvScalar(i),-1);
	//		
	//	}


	//	
	//	getline(in,line);
	//	getline(in,line);
	//	getline(in,line);
	//	

	//	for (int i=0;i<NumOfControlPoint;i++)
	//	{
	//		in >>Coor.x >>Coor.y;
	//		BezierAdjustPoints.push_back(Coor);
	//		cvCircle(BezierAdjustPointsMask,Coor,BlobMaskSize,cvScalar(i),-1);
	//		
	//	}
	//	in.close();


	//	SaveInfoToFrame(FramesAndPoints[NumOfFrame-1],NumOfFrame-1);


	//	cvCopy(FilmSeries[NumOfFrame-1],CurrentProcessingImage);
	//	cvCopy(FilmSeries[NumOfFrame-1],CurrentProcessingImageShow);
	//	DrawCurve();
	//	cvNamedWindow("EndFrame");
	//	cvShowImage("EndFrame",CurrentProcessingImageShow);
	//	cout << "Press Any Key to Continue." << endl;
	//	cvWaitKey(0);
	//	
	//}

	//void SaveInfoToFrame(FrameInfo & Frame,int Id){
	//	
	//	Frame.FrameId = Id;
	//	cvCopy(BezierAdjustPointsMask,Frame.BezierAdjustPointsMask);
	//	cvCopy(ControlPointMask,Frame.ControlPointMask);
	//	Frame.ControlPoints = ControlPoints;
	//	Frame.BezierAdjustPoints = BezierAdjustPoints;

	//	Frame.NumOfControlPoint = NumOfControlPoint;


	//	for (int i=0;i<NumOfControlPoint-1;i++)
	//	{
	//		CvPoint2D32f BezierPoint[SAMPLESPERCURVE];

	//		CvPoint OppositePoint;
	//		OppositePoint.x = ControlPoints[i+1].x - (BezierAdjustPoints[i+1].x-ControlPoints[i+1].x);
	//		OppositePoint.y = ControlPoints[i+1].y - (BezierAdjustPoints[i+1].y-ControlPoints[i+1].y);				//注意那个顺序!!!!!!!!!!!!Bezier的调整点在哪一侧对生成的插值点的顺序会有影响!!!!会有问题的!!!!

	//		Frame.CurvePoints.push_back(ControlPoints[i]);									//printf("x=%d,y=%d\n",ControlPoints[i].x , ControlPoints[i].y);
	//		Bezier::GetInstance()->GenerateCurve(
	//			cvPointTo32f(ControlPoints[i]),
	//			cvPointTo32f(BezierAdjustPoints[i]),
	//			cvPointTo32f(OppositePoint),
	//			cvPointTo32f(ControlPoints[i+1]),
	//			SAMPLESPERCURVE,
	//			BezierPoint);
	//		for (int i=0;i<SAMPLESPERCURVE;i++)
	//		{
	//			Frame.CurvePoints.push_back(cvPoint(BezierPoint[i].x,BezierPoint[i].y));							//printf("x=%d,y=%d\n",BezierPoint[i].x , BezierPoint[i].y);
	//		}

	//	}
	//	
	//	Frame.CurvePoints.push_back(ControlPoints[NumOfControlPoint-1]);
	//	cout << "Frame Id=" << Id <<"   Frame.CurvePoints.size()" << Frame.CurvePoints.size() << endl;

	//	
	//}
	

	
	


//void MouseCallbackFuncFirstFrame(int mouseevent,int x,int y,int flag, void * param){
//
//	RotoScoping* RS = (RotoScoping*)param;
//
//	static bool Pressed = false;
//	static bool IsControlPoint;
//	static bool IsAdjustPoint;
//	static int ControlPointId;
//	static int AdjustPointId;
//	static int xpre,ypre;
//	static int CurPointId=0;
//	
//
//	if (mouseevent==CV_EVENT_LBUTTONDOWN)
//	{
//
//		Pressed = true;
//
//
//		// If we press on a control point that already exists, then we move it
//		ControlPointId = RS->GetControlPointId(x,y);
//		if ( ControlPointId >=0)  // This a control point
//		{
//			IsControlPoint = true;
//																		cout << "													Updating Control Point "<< ControlPointId << endl;
//
//			/******************************************************************************
//									Erase the original Mask!!!!
//									i.e,   set to -1
//			 ******************************************************************************/
//			cvCircle(RS->ControlPointMask,cvPoint(x,y),BlobMaskSize,cvScalar(-1),-1);
//			//cvCircle(RS->BezierAdjustPointsMask,RS->BezierAdjustPoints[ControlPointId],BlobMaskSize,cvScalar(-1),-1);
//
//		}
//		else{
//			IsControlPoint = false;
//		}
//
//
//
//
//		AdjustPointId = RS->GetAdjustPointId(x,y);
//		if (AdjustPointId >= 0)
//		{
//			IsAdjustPoint = true;
//																				cout << "													Updating Adjust Point " << AdjustPointId << endl;
//
//			/******************************************************************************
//									Erase the original Mask!!!!
//									i.e,   set to -1
//			 ******************************************************************************/
//			cvCircle(RS->BezierAdjustPointsMask,cvPoint(x,y),BlobMaskSize,cvScalar(-1),-1);
//			
//		}
//		else{
//			IsAdjustPoint = false;
//		}
//
//		
//
//
//		cvCircle(RS->CurrentProcessingImageShow,cvPoint(x,y),ControlPointRadius,cvScalar(0,0,255),-1);
//		
//
//		xpre = x;
//		ypre = y;
//
//
//	}
//	else if (mouseevent == CV_EVENT_LBUTTONUP)
//	{
//		Pressed = false;
//		if (IsControlPoint)
//		{
//			/*****************************************************************************************
//								Update the Control Point mask in order to search!!!!
//			 *****************************************************************************************/
//
//			cvCircle(RS->ControlPointMask,cvPoint(x,y),BlobMaskSize,cvScalar(ControlPointId),-1);
//			//cvCircle(RS->BezierAdjustPointsMask,RS->BezierAdjustPoints[ControlPointId],BlobMaskSize,cvScalar(ControlPointId),-1);
//
//		}
//
//
//		else if(IsAdjustPoint){
//			/*****************************************************************************************
//								Update the Adjust Point mask in order to search!!!!
//			 *****************************************************************************************/
//			cvCircle(RS->BezierAdjustPointsMask,cvPoint(x,y),BlobMaskSize,cvScalar(AdjustPointId),-1);
//		}
//
//
//		else
//		{
//			cvCircle(RS->CurrentProcessingImageShow,cvPoint(x,y),2,cvScalar(0,255,255),-1);   //Bezier adjutment point (yellow)
//			
//
//			/*****************************************************************************************
//							    Mark the Control Point and Adjust Point mask in order to search!!!!
//			 *****************************************************************************************/
//			RS->ControlPoints.push_back(cvPoint(xpre,ypre));
//			cvCircle(RS->ControlPointMask,cvPoint(xpre,ypre),BlobMaskSize,cvScalar(RS->ControlPoints.size()-1),-1);  //  Mark the mask in order to search!!!!
//			
//			RS->BezierAdjustPoints.push_back(cvPoint(x,y));
//			cvCircle(RS->BezierAdjustPointsMask,cvPoint(x,y),BlobMaskSize,cvScalar(RS->BezierAdjustPoints.size()-1),-1);  //  Mark the mask in order to search!!!!
//			RS->DrawCurve();
//
//
//
//		}
//		
//
//
//	}
//
//	else if (mouseevent == CV_EVENT_MOUSEMOVE)
//	{
//		if (Pressed)
//		{
//
//			if (IsControlPoint)
//			{
//				RS->UpdateControlPoint(ControlPointId,x,y);
//			
//				cvCopy(RS->CurrentProcessingImage,RS->CurrentProcessingImageShow);
//				//draw previous point
//				RS->DrawCurve();
//			}
//			else if(IsAdjustPoint){
//
//				RS->UpdateAdjustPoint(AdjustPointId,x,y);
//
//				cvCopy(RS->CurrentProcessingImage,RS->CurrentProcessingImageShow);
//				//draw previous point
//				RS->DrawCurve();
//			}
//
//
//			else{
//				cvCopy(RS->CurrentProcessingImage,RS->CurrentProcessingImageShow);
//				//draw previous point
//				RS->DrawCurve();
//
//				cvCircle(RS->CurrentProcessingImageShow,cvPoint(xpre,ypre),ControlPointRadius,cvScalar(0,0,255),-1);// control point that has not been push into the vector (red)
//				cvLine(RS->CurrentProcessingImageShow,cvPoint(xpre,ypre),cvPoint(x,y),cvScalar(0,255,0),LineWidth); // green line that denote the tangential orientation (green)
//			
//				CvPoint OppositePoint;
//				OppositePoint.x = xpre - (x - xpre);
//				OppositePoint.y = ypre - (y - ypre);
//
//				cvLine(RS->CurrentProcessingImageShow,cvPoint(xpre,ypre),OppositePoint,cvScalar(0,255,0),LineWidth); // green line that denote the tangential orientation (green)
//
//
//
//				if(RS->BezierAdjustPoints.size()>=1){
//					CvPoint2D32f BezierPoint[SAMPLESPERCURVE];
//					Bezier::GetInstance()->GenerateCurve(
//						cvPointTo32f(RS->ControlPoints[RS->ControlPoints.size()-1]),
//						cvPointTo32f(RS->BezierAdjustPoints[RS->BezierAdjustPoints.size()-1]),
//						cvPointTo32f(OppositePoint),
//						cvPoint2D32f(xpre,ypre),
//						SAMPLESPERCURVE,
//						BezierPoint);
//					for (int i=0;i<SAMPLESPERCURVE;i++)
//					{
//						cvCircle(RS->CurrentProcessingImageShow,cvPoint(BezierPoint[i].x,BezierPoint[i].y),CurvePointRadius,cvScalar(0,255,255),-1);			    // Bezier points are blue
//					}
//				}
//			}
//
//		}
//	
//	}
//}
//
//
//void MouseCallbackFuncOtherFrame(int mouseevent,int x,int y,int flag, void * param){
//		RotoScoping* RS = (RotoScoping*)param;
//
//	static bool Pressed = false;
//	static bool IsControlPoint;
//	static bool IsAdjustPoint;
//	static int ControlPointId;
//	static int AdjustPointId;
//	static int xpre,ypre;
//	static int CurPointId=0;
//	
//
//	if (mouseevent==CV_EVENT_LBUTTONDOWN)
//	{
//
//		Pressed = true;
//
//
//		// If we press on a control point that already exists, then we move it
//		ControlPointId = RS->GetControlPointId(x,y);
//		if ( ControlPointId >=0)  // This a control point
//		{
//			IsControlPoint = true;
//																		cout << "													Updating Control Point "<< ControlPointId << endl;
//
//			/******************************************************************************
//									Erase the original Mask!!!!
//									i.e,   set to -1
//			 ******************************************************************************/
//			cvCircle(RS->ControlPointMask,cvPoint(x,y),BlobMaskSize,cvScalar(-1),-1);
//			//cvCircle(RS->BezierAdjustPointsMask,RS->BezierAdjustPoints[ControlPointId],BlobMaskSize,cvScalar(-1),-1);
//
//		}
//		else{
//			IsControlPoint = false;
//		}
//
//
//
//
//		AdjustPointId = RS->GetAdjustPointId(x,y);
//		if (AdjustPointId >= 0)
//		{
//			IsAdjustPoint = true;
//																				cout << "													Updating Adjust Point "<< AdjustPointId << endl;
//
//			/******************************************************************************
//									Erase the original Mask!!!!
//									i.e,   set to -1
//			 ******************************************************************************/
//			cvCircle(RS->BezierAdjustPointsMask,cvPoint(x,y),BlobMaskSize,cvScalar(-1),-1);
//			
//		}
//		else{
//			IsAdjustPoint = false;
//		}
//
//
//
//		
//		
//
//		xpre = x;
//		ypre = y;
//
//
//	}
//	else if (mouseevent == CV_EVENT_LBUTTONUP)
//	{
//		Pressed = false;
//		if (IsControlPoint)
//		{
//			/*****************************************************************************************
//								Update the Control Point mask in order to search!!!!
//			 *****************************************************************************************/
//
//			cvCircle(RS->ControlPointMask,cvPoint(x,y),BlobMaskSize,cvScalar(ControlPointId),-1);
//			//cvCircle(RS->BezierAdjustPointsMask,RS->BezierAdjustPoints[ControlPointId],BlobMaskSize,cvScalar(ControlPointId),-1);
//
//		}
//
//
//		else if(IsAdjustPoint){
//			/*****************************************************************************************
//								Update the Adjust Point mask in order to search!!!!
//			 *****************************************************************************************/
//			cvCircle(RS->BezierAdjustPointsMask,cvPoint(x,y),BlobMaskSize,cvScalar(AdjustPointId),-1);
//		}
//
//
//	}
//
//	else if (mouseevent == CV_EVENT_MOUSEMOVE)
//	{
//		if (Pressed)
//		{
//
//			if (IsControlPoint)
//			{
//				RS->UpdateControlPoint(ControlPointId,x,y);
//			
//				cvCopy(RS->CurrentProcessingImage,RS->CurrentProcessingImageShow);
//				//draw previous point
//				RS->DrawCurve();
//			}
//			else if(IsAdjustPoint){
//
//				RS->UpdateAdjustPoint(AdjustPointId,x,y);
//
//				cvCopy(RS->CurrentProcessingImage,RS->CurrentProcessingImageShow);
//				//draw previous point
//				RS->DrawCurve();
//			}
//
//		}
//	
//	}
//
//}
//
//
//void MouseCallbackFuncDisplacement(int mouseevent,int x,int y,int flag, void * param){
//		RotoScoping* RS = (RotoScoping*)param;
//
//	static bool Pressed = false;
//	static bool IsControlPoint;
//	static bool IsAdjustPoint;
//	static int ControlPointId;
//	static int AdjustPointId;
//	static int xpre,ypre;
//	static int CurPointId=0;
//	
//
//	if (mouseevent==CV_EVENT_LBUTTONDOWN)
//	{
//
//		Pressed = true;
//		RS->EraseAllMask();
//		xpre = x;
//		ypre = y;
//
//
//	}
//	else if (mouseevent == CV_EVENT_LBUTTONUP)
//	{
//		Pressed = false;
//		RS->UpdateAllMask();
//
//		cvCopy(RS->CurrentProcessingImage,RS->CurrentProcessingImageShow);
//		//draw previous point
//		RS->DrawCurve();
//	}
//
//	else if (mouseevent == CV_EVENT_MOUSEMOVE)
//	{
//		if (Pressed)
//		{
//			int dx = x - xpre;
//			int dy = y - ypre;
//			xpre = x;
//			ypre = y;
//
//			RS->AllPointDisplace(dx,dy);
//
//			cvCopy(RS->CurrentProcessingImage,RS->CurrentProcessingImageShow);
//			//draw previous point
//			RS->DrawCurve();
//		}
//	
//	}
//
//}
//
//
//
//
//
//

#endif

