#include "stdafx.h"
#include "VideoEditing.h"
#include "VideoEditingDoc.h"
#include "VideoEditingView.h"
#include <WinAble.h>
#include "MainFrm.h"
#include <io.h>
#include "LayerEditDlg.h"
#include "Vision/Others/opencv10/include/cv.h"
#include "Vision/Others/OpenCV10/include/highgui.h"
#include "FlowCalculator_Black.h"
#include "MattingParamDlg.h"
#include "MattingWorkUnit.h"
#include "../CutRef/CutRef/Refiner.h"
#include "Inpainting.h"
#include "../CutRef/CutRef/Global.h"
#include "color_flow.h"
#include "FlowCalculator_LiuCe.h"
#include "..\mexOpticalFlow\mex\OpticalFlow.h"

void CVideoEditingView::OnRunRun()
{
	CxImage result;
	CxImage alpha;
	CxImage trimap;
	CxImage layer;
	processing=false;
	firstFrameOK=true;
	imageEditView->Clear();

	double startt, endt, dt;
	startt = clock();
	GetDocument()->Run(result, alpha);
	endt = clock();
	dt = (endt-startt)/CLK_TCK;
	std::cout<<" run time: "<<dt<<std::endl;

	startt = clock();
	//ReUpdateResult(result,alpha);
//	GetDocument()->GetTrimap(trimap,trimapWidth);

//	this->GetEditLayer(layerBuffer.GetImage(),&alpha,layer);
	//计算trimap并保存
	if(mode==PICVIEW)
	{
		resImageBuffer.OpenImage(result);
		Invalidate();
	}
	else
		if(mode==EDITVIEW)
		{
			CString title; 
			resImageBuffer.SetPos(imageBuffer.GetFramePos());
			alphaBuffer.SetPos(imageBuffer.GetFramePos());
			trimapBuffer.SetPos(imageBuffer.GetFramePos());

			title=imageBuffer.GetFrameName();
			AfxGetMainWnd()->SetWindowText(title);
			resImageBuffer.OpenImage(result);
			result.Save((LPCTSTR)resImageBuffer.GetFrameName(), CXIMAGE_FORMAT_JPG);
			alpha.Save((LPCTSTR)alphaBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);
			std::cout<<alphaBuffer.GetFrameName()<<std::endl;
	//		trimap.Save((LPCTSTR)trimapBuffer.GetFrameName(),CXIMAGE_FORMAT_BMP);
//			CString temp;
//			temp=editLayerPath;
//			temp.AppendFormat("%s",imageBuffer.GetFileName());
//			layer.Save((LPCTSTR)temp,CXIMAGE_FORMAT_JPG);
			layerGenerated=true;
	//		result.Copy(*imageBuffer.GetImage());
	//		Composite(result, result, trimap);
	//		trimapBuffer.OpenImage(result);
			Invalidate();
		}
		endt = clock(); 
		dt = (endt-startt)/CLK_TCK;
		std::cout<<" other time: "<<dt<<std::endl;
}

void CVideoEditingView::OnUpdateRunRun(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(imageOpened && (!processing) && rectangled&&(!allLayerGenerated)&&(!calDepthStarted) && !IsToolManager);
}
 

void CVideoEditingView::OnRunRunvideo()
{
	CWinThread *thread;
	clickToStop=false;
	editTool=NOTOOL;
	processing=true;
	// Should disable some buttons...
	GetDocument()->InitVideoFirstFrame();

   // RunVideo(LPVOID(this));
	thread=AfxBeginThread(RunVideo, LPVOID(this));
	
}


void CVideoEditingView::OnUpdateRunRunvideo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(imageOpened && firstFrameOK&&!processing&&!allLayerGenerated&&!calDepthStarted&& !IsToolManager);
}

void CVideoEditingView::OnRunStop()
{
	if(::MessageBox(NULL, "Stop the processing?", "Thread", MB_OKCANCEL)!=IDOK)
		return;
	clickToStop=true;
	if(processing)
	{
		//imageBuffer.SetName(backGroundLayerPath);
		imageBuffer.SetName(backGroundLayerPath, imageExt);
		imageBuffer.Goto(imageBuffer.GetFramePos());
		exitThread.SetEvent();
		mutex.SetEvent();
		::WaitForSingleObject(finishExiting.m_hObject, INFINITE);

		::MessageBox(NULL, "Thread exited", "Thread", MB_OK);
		processing=false;

		// Also for moving the slide...
		GetDocument()->RestoreFrame(imageBuffer.GetImage(), alphaBuffer.GetImage());
		this->setProcess();
		//restore trimap
		Invalidate();
	}
}

void CVideoEditingView::OnUpdateRunStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(imageOpened && processing&&(!allLayerGenerated)&&(!calDepthStarted)&& !IsToolManager);
}

void CVideoEditingView::OnUpdateGlobalupdate()
{
	GetDocument()->updateMode=LOCAL_UPDATE;
}

void CVideoEditingView::OnUpdateLocalupdate()
{
	GetDocument()->updateMode=LOCAL_UPDATE;
}

void CVideoEditingView::OnUpdateUpdateGlobalupdate(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetDocument()->updateMode==GLOBAL_UPDATE&& !IsToolManager);
}

void CVideoEditingView::OnUpdateUpdateLocalupdate(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetDocument()->updateMode==LOCAL_UPDATE&& !IsToolManager);
}

/*void CVideoEditingView::OnRunMatting()
{
	cout<<"on run matting"<<endl;
	editTool=NOTOOL;

	if(mode==PICVIEW)
	{
		GetDocument()->RunMatting(*resImageBuffer.GetImage());
		Invalidate();
	}
	else if(mode==EDITVIEW&&mattingMode==CLOSEDMAT)
	{
		CxImage result;
		static int index(0);
		CString dir;
		alphaBuffer.SetName(alphaPath, alphaExt);
		alphaBuffer.Goto(0);
		imageBuffer.Goto(0);
		do
		{
			GetDocument()->RestoreFrame(imageBuffer.GetImage(), alphaBuffer.GetImage());
			GetDocument()->RunMatting(result);
			// Test
			dir.Format("%s%d%s", "D:\\Result\\alpha\\", index++, ".bmp");
			result.Save(dir, CXIMAGE_FORMAT_BMP);
			Invalidate();
		}
		while(imageBuffer.Forward() && alphaBuffer.Forward());
		MessageBox("Matting over!");
	}

	else if(mode==EDITVIEW&&mattingMode==BAYESIAN)
	{
		CxImage alpha_channel;
		alphaBuffer.Goto(0);
		trimapBuffer.Goto(0);
		imageBuffer.Goto(0);
		do 
		{
			cout<<trimapBuffer.GetFrameName()<<endl;
			pi.init(*(imageBuffer.GetImage()),*(trimapBuffer.GetImage()));
			alpha_channel.Copy(*(imageBuffer.GetImage()));
			alpha_channel.Clear(0);
			if (pi.isValidName())
			{
				cout<<"begin matting"<<endl;
				pi.mattingImage();
				cout<<"after matting"<<endl;

				for (int i=0;i<alpha_channel.GetWidth();++i)
				{
					for (int j=0;j<alpha_channel.GetHeight();++j)
					{
						RGBQUAD cc={pi.alpha[i][j]*255,pi.alpha[i][j]*255,pi.alpha[i][j]*255,0};
						alpha_channel.SetPixelColor(i,j,cc,false);
					}
				}
			}
			alpha_channel.Save(alphaBuffer.GetFrameName(),CXIMAGE_FORMAT_BMP);
			Invalidate();

		} while (imageBuffer.Forward() && trimapBuffer.Forward()&&alphaBuffer.Forward());

		MessageBox("Matting over!");

	}
}*/

void CVideoEditingView::OnRunMatting()
{
	CString layername = vec_layer[current_layer].GetName();
	int startframe = vec_layer[current_layer].GetStartIndex();
	int endframe = CheckTrimapExist();
	if (endframe<startframe)
	{
		AfxMessageBox("Generate trimap first");
		return;
	}
	if (GetDocument()->mattingType==MAT_SEQ)
	{
		CMattingParamDlg matdlg(layername, startframe, endframe);
		if (matdlg.DoModal()==IDOK)
		{
			int _startframe = matdlg.m_startFrame;
			int _endframe = matdlg.m_endFrame;
			startframe = (startframe<_startframe)?_startframe:startframe;
			endframe = (endframe>_endframe)?_endframe:endframe;
			CString dir;
			editTool=NOTOOL;
			GetDocument()->mattingMode = matdlg.m_radio?MAT_WHOLEIMG:MAT_PARTIMG;
			GetDocument()->mattingAlgo = matdlg.m_algorithm;
			imageBuffer.Goto(startframe);
			alphaBuffer.Goto(startframe);
			trimapBuffer.Goto(startframe);
			resImageBuffer.Goto(startframe);
			ImageBuffer softAlphaBuffer;
			CString path = filePath;
			path.AppendFormat("%s/%s", vec_layer[current_layer].GetName(),"VEAlpha");
			CreateDirectory(path,0);
			path.AppendFormat("/%s", filename);
			softAlphaBuffer.SetName(path, alphaExt);
			if (mode==TRIMAPVIEW&&GetDocument()->mattingAlgo==CLOSEDMAT&&GetDocument()->mattingMode==MAT_PARTIMG)
			{
				std::cout<<"in matting"<<std::endl;
				MattingWorkUnit mtu(&imageBuffer, &trimapBuffer, &softAlphaBuffer,&resImageBuffer);
				mtu.MattingSequence();

				imageBuffer.Goto(startframe);
				trimapBuffer.Goto(startframe);
				resImageBuffer.Goto(startframe);
				Invalidate();
			}
			else if(mode==TRIMAPVIEW&&GetDocument()->mattingAlgo==CLOSEDMAT&&GetDocument()->mattingMode==MAT_WHOLEIMG)
			{
				CxImage result;
				CxImage fgimg;
				int pos = imageBuffer.GetFramePos();
				do
				{
					std::cout<<trimapBuffer.GetFrameName()<<std::endl;
					GetDocument()->RunMatting(result, *imageBuffer.GetImage(), *alphaBuffer.GetImage(),*trimapBuffer.GetImage(), fgimg);
					softAlphaBuffer.SetPos(imageBuffer.GetFramePos());
					resImageBuffer.SetPos(imageBuffer.GetFramePos());
					result.Save(softAlphaBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);
					fgimg.Save(resImageBuffer.GetFrameName(), CXIMAGE_FORMAT_JPG);
					Invalidate();
				}while(imageBuffer.Forward() && alphaBuffer.Forward()&&trimapBuffer.Forward());
				imageBuffer.Goto(pos);
				alphaBuffer.Goto(pos);
				trimapBuffer.Goto(pos);
				resImageBuffer.Goto(pos);
				MessageBox("Matting over!");
				Invalidate();
			}
		}
	}
	else if (GetDocument()->mattingType==MAT_FRAME)
	{
		ImageBuffer softAlphaBuffer;
		CString path = filePath;
		path.AppendFormat("%s/%s", vec_layer[current_layer].GetName(),"VEAlpha");
		CreateDirectory(path,0);
		path.AppendFormat("/%s", filename);
		softAlphaBuffer.SetName(path, alphaExt);
		trimapBuffer.Goto(imageBuffer.GetFramePos());		
		if (mode==TRIMAPVIEW&&GetDocument()->mattingAlgo==CLOSEDMAT&&GetDocument()->mattingMode==MAT_PARTIMG)
		{
			CxImage result;
			result.Copy(*(imageBuffer.GetImage()));
			CxImage fgimg;
			fgimg.Copy(result);
			fgimg.Clear();
			MattingWorkUnit mtu;
			mtu.MattingFrame(imageBuffer.GetImage(), trimapBuffer.GetImage(),&result, &fgimg);
			softAlphaBuffer.SetPos(imageBuffer.GetFramePos());
			result.Save(softAlphaBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);
			fgimg.Save(resImageBuffer.GetFrameName(), CXIMAGE_FORMAT_JPG);
		}
		else if(mode==TRIMAPVIEW&&GetDocument()->mattingAlgo==CLOSEDMAT&&GetDocument()->mattingMode==MAT_WHOLEIMG)
		{
			CxImage result;
			CxImage fgimg;
			std::cout<<"In mat_wholeImage: "<< trimapBuffer.GetFrameName()<<std::endl;
			GetDocument()->RunMatting(result, *imageBuffer.GetImage(), *alphaBuffer.GetImage(),*trimapBuffer.GetImage(), fgimg);
			softAlphaBuffer.SetPos(imageBuffer.GetFramePos());
			resImageBuffer.SetPos(imageBuffer.GetFramePos());
			result.Save(softAlphaBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);
			fgimg.Save(resImageBuffer.GetFrameName(), CXIMAGE_FORMAT_JPG);
			Invalidate();
		}
	}
}
void CVideoEditingView::OnUpdateRunMatting(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(imageOpened&&(!processing)&&(firstFrameOK)&& !IsToolManager&&(mode==TRIMAPVIEW));
}

void CVideoEditingView::OnRunGetfinallayer()
{
	// TODO: Add your command handler code here
	if(::MessageBox(NULL, "Make sure all other layers are ok! ", "Thread", MB_OKCANCEL)!=IDOK)
		return;

	for (int i=0; i<vec_layer.size(); ++i)
	{
		this->SetRLTPath(filePath, vec_layer[i].GetName());
		ImageBuffer softAlphaBuffer;
		CString path = filePath;
		path.AppendFormat("%s\\%s", vec_layer[i].GetName(),"VEAlpha");
		//CreateDirectory(path,0);
		path.AppendFormat("\\%s", filename);
		softAlphaBuffer.SetName(path, alphaExt);
		bool alphaexist = softAlphaBuffer.OpenVideo(vec_layer[i].GetStartIndex());
		if (alphaBuffer.OpenVideo(vec_layer[i].GetStartIndex())==false)//不存在mask
		{
			if(alphaexist)
			{
				do 
				{
					CxImage mask;
					mask.Copy(*(softAlphaBuffer.GetImage()));
					for (int jtemp=0;jtemp<mask.GetHeight(); ++jtemp)
					{
						for (int itemp=0; itemp<mask.GetWidth(); ++itemp)
						{
							BYTE gray = mask.GetPixelGray(itemp,jtemp);
							if (gray!=255)
								mask.SetPixelColor(itemp, jtemp, RGB(0,0,0));
						}
					}
					alphaBuffer.SetPos(softAlphaBuffer.GetFramePos());
					mask.Save(alphaBuffer.GetFrameName().GetBuffer(),CXIMAGE_FORMAT_PNG);
				} while (softAlphaBuffer.Forward());
			}
		}
	}

	bool finalLayerExists=false;
	for (int i=0;i<vec_layer.size();++i)
	{
		if (vec_layer[i].GetName()=="finalLayer")
		{
			finalLayerExists=true;
			break;
		}
	}

	if (!finalLayerExists)
	{
		this->AddLayer("finalLayer",0);
	}

	CString labelstr;
	CString restr;

	CxImage l;
	CxImage labelFinalLayer;
	labelFinalLayer.Copy(*(imageBuffer.GetImage()));
	labelFinalLayer.Clear(255);

	CxImage re;

	RGBQUAD rgb2;

	CString finalLayerLabelLocation;
	CString finalLayerReLocation;

	finalLayerReLocation=filePath;
	finalLayerLabelLocation=filePath;

	rgb2.rgbBlue=rgb2.rgbGreen=rgb2.rgbRed=0;

	for (int i=0;i<fileRange;++i)
	{

		labelFinalLayer.Clear(255);

		imageBuffer.Goto(startIndex+i);
		resImageBuffer.Goto(startIndex+i);
		alphaBuffer.Goto(startIndex+i);
		trimapBuffer.Goto(startIndex+i);
		cout<<startIndex+i<<endl;

		re.Copy(*(imageBuffer.GetImage()));

		CString tframename;
		tframename=imageBuffer.GetFileName();
		int index=tframename.Find(".");
		tframename.Delete(index,4);


		finalLayerReLocation=filePath;
		finalLayerLabelLocation=filePath;
		finalLayerLabelLocation.AppendFormat("%s%s%s%s.%s","finalLayer","\\VELabel","\\",tframename,alphaExt.GetBuffer());
		finalLayerReLocation.AppendFormat("%s%s%s%s.%s","finalLayer","\\VEResult","\\",tframename,imageExt.GetBuffer());
		for (int j=0;j<vec_layer.size();++j)
		{
			if (vec_layer[j].GetName()!="finalLayer"&&vec_layer[j].GetName()!="editLayer"&&vec_layer[j].GetName()!="background")
			{
				labelstr=filePath;
				labelstr.AppendFormat("%s%s%s%s.%s",vec_layer[j].GetName(),"\\VELabel","\\",tframename,alphaExt.GetBuffer());
				
				cout<<labelstr<<endl;
				l.Load(labelstr);

				for (int p=0;p<l.GetHeight();++p)
				{
					for (int q=0;q<l.GetWidth();++q)
					{
						RGBQUAD r=l.GetPixelColor(q,p);
						if (r.rgbBlue!=0)
						{
							labelFinalLayer.SetPixelColor(q,p,rgb2);
							re.SetPixelColor(q,p,rgb2);
						}
					}
				}

			}
		}
		labelFinalLayer.Save(finalLayerLabelLocation,CXIMAGE_FORMAT_PNG);
		re.Save(finalLayerReLocation,CXIMAGE_FORMAT_JPG);

	}

}


void CVideoEditingView::OnModePicture()
{
	mode=PICVIEW;
	hasMoved=false;
	GetDocument()->Opened=false;
	imageBuffer.ClearBuffer();
	Invalidate();
}

void CVideoEditingView::OnModeVideo()
{
	mode=EDITVIEW;
	hasMoved=false;
	GetDocument()->Opened=false;
	imageBuffer.ClearBuffer();
	Invalidate();
}

void CVideoEditingView::OnUpdateModePicture(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(mode==PICVIEW&& !IsToolManager);
}

void CVideoEditingView::OnUpdateModeVideo(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(mode==EDITVIEW&& !IsToolManager);
}

#include "TemporalRefineDlg.h"
void CVideoEditingView::OnRunRefine()
{

	int startindex = vec_layer[current_layer].GetStartIndex();
	int endindex = CheckLabelExist();
	if (startindex>endindex)
	{
		MessageBox("Please segment the sequence first");
		return;
	}

	CTemporalRefineDlg ctrd(vec_layer[current_layer].GetName(),startindex, endindex);

	int _startIndex;
	int _endIndex;
	if (ctrd.DoModal()==IDOK)
	{
		_startIndex = ctrd.m_startFrame;
		_endIndex = ctrd.m_endFrame;
		_startIndex = (startindex>_startIndex)?startindex:_startIndex;
		_endIndex = (endindex>_endIndex)?_endIndex:endindex;


		CString optPath = filePath;
		optPath.AppendFormat("opt\\");

		CString labelPath = filePath;
		labelPath.AppendFormat("%s\\%s\\", vec_layer[current_layer].GetName(),"VELabel");

		CString trimapPath = filePath;
		trimapPath.AppendFormat("%s\\%s\\", vec_layer[current_layer].GetName(), "VETrimap");

		int startFrame = vec_layer[current_layer].GetStartIndex();

		int pos = alphaBuffer.GetFramePos();
		alphaBuffer.Goto(startFrame);
		int frameCount = 1;
		while (alphaBuffer.Forward())
			frameCount ++;

		alphaBuffer.Goto(pos);
		std::cout<<"start Frame:"<< startFrame<<" frame count: "<<frameCount<<std::endl;
		std::cout<<labelPath.GetBuffer()<<" "<<optPath.GetBuffer()<<std::endl;
		Refiner temporalRefine("", labelPath.GetBuffer(), optPath.GetBuffer(), trimapPath.GetBuffer());	
		if(ctrd.m_use_global_flow)
			temporalRefine.SetVideo(imageBuffer.GetFrameName().GetBuffer(), _startIndex, _endIndex+1, startindex, endindex);//last frame excluded
		else
			temporalRefine.SetVideo(imageBuffer.GetFrameName().GetBuffer(), _startIndex, _endIndex+1, _startIndex, _endIndex);//last frame excluded


		//temporalRefine.SetVideo(imageBuffer.GetFrameName().GetBuffer(), startFrame, startFrame+frameCount);
		//temporalRefine.SetVideo(imageBuffer.GetFrameName().GetBuffer(), 50, 60);

		double start = clock();
		temporalRefine.GMMRefineCutting();
		double end = clock();
		double du = (end-start)/CLK_TCK;
		std::cout<<"Refine Time: "<<du<<std::endl;
		int pos2;
		for (int i=0; i<frameCount; ++i)
		{
			pos2 = startFrame+i;
			alphaBuffer.Goto(pos2);
			imageBuffer.Goto(pos2);
			resImageBuffer.Goto(pos2);
			CxImage* img = imageBuffer.GetImage();
			CxImage temp;
			temp.Create(img->GetWidth(), img->GetHeight(),24);
			for (int itemp = 0; itemp<img->GetWidth(); ++itemp)
			{
				for (int jtemp = 0; jtemp<img->GetHeight(); ++jtemp)
				{
					if (alphaBuffer.GetImage()->GetPixelGray(itemp, jtemp))
					{
						temp.SetPixelColor(itemp, jtemp, img->GetPixelColor(itemp, jtemp));
					}
					else
						temp.SetPixelColor(itemp, jtemp, RGB(0,0,0));
				}
			}
			temp.Save(resImageBuffer.GetFrameName().GetBuffer(), CXIMAGE_FORMAT_JPG);
		}
		MessageBox("Label Refine Over");
	}
	//else
	//	return;

	//CString optPath = filePath;
	//optPath.AppendFormat("opt\\");

	//CString labelPath = filePath;
	//labelPath.AppendFormat("%s\\%s\\", vec_layer[current_layer].GetName(),"VELabel");

	//CString trimapPath = filePath;
	//trimapPath.AppendFormat("%s\\%s\\", vec_layer[current_layer].GetName(), "VETrimap");

	//int startFrame = vec_layer[current_layer].GetStartIndex();

	//int pos = alphaBuffer.GetFramePos();
	//alphaBuffer.Goto(startFrame);
	//int frameCount = 1;
	//while (alphaBuffer.Forward())
	//	frameCount ++;
	//
	////startFrame = 50;
	////frameCount = 11;

	//alphaBuffer.Goto(pos);

	//std::cout<<"start Frame:"<< startFrame<<" frame count: "<<frameCount<<std::endl;
	//std::cout<<labelPath.GetBuffer()<<" "<<optPath.GetBuffer()<<std::endl;
	//Refiner temporalRefine("", labelPath.GetBuffer(), optPath.GetBuffer(), trimapPath.GetBuffer());
	////temporalRefine.SetVideo(imageBuffer.GetFrameName().GetBuffer(), startFrame, startFrame+frameCount);
	//temporalRefine.SetVideo(imageBuffer.GetFrameName().GetBuffer(), startindex, endindex+1);//last frame excluded
	////temporalRefine.SetVideo(imageBuffer.GetFrameName().GetBuffer(), 50, 60);

	//double start = clock();
	//temporalRefine.GMMRefineCutting();
	//double end = clock();
	//double du = (end-start)/CLK_TCK;
	//std::cout<<"Refine Time: "<<du<<std::endl;
	//int pos2;
	//for (int i=0; i<frameCount; ++i)
	//{
	//	pos2 = startFrame+i;
	//	alphaBuffer.Goto(pos2);
	//	imageBuffer.Goto(pos2);
	//	resImageBuffer.Goto(pos2);
	//	CxImage* img = imageBuffer.GetImage();
	//	CxImage temp;
	//	temp.Create(img->GetWidth(), img->GetHeight(),24);
	//	for (int itemp = 0; itemp<img->GetWidth(); ++itemp)
	//	{
	//		for (int jtemp = 0; jtemp<img->GetHeight(); ++jtemp)
	//		{
	//			if (alphaBuffer.GetImage()->GetPixelGray(itemp, jtemp))
	//			{
	//				temp.SetPixelColor(itemp, jtemp, img->GetPixelColor(itemp, jtemp));
	//			}
	//			else
	//				temp.SetPixelColor(itemp, jtemp, RGB(0,0,0));
	//		}
	//	}
	//	temp.Save(resImageBuffer.GetFrameName().GetBuffer(), CXIMAGE_FORMAT_JPG);
	//}
	//MessageBox("Label Refine Over");
}
/*
void CVideoEditingView::OnPreprocessOpticalflow()    
{
	// TODO: Add your command handler code here
	CString OptFlowPath = filePath;
    
	OptFlowPath.AppendFormat("%s","opt");
	if(!CreateDirectory(OptFlowPath, 0))
	{
		if(MessageBox((LPCTSTR)"Optical flow already exists, do you want to over write it?","Optical flow", MB_YESNO) == IDNO)
			return;
	}
	imageBuffer.Goto(0);
	CxImage SrcImg;
	SrcImg.Copy(*imageBuffer.GetImage());
	CxImage RfSrcImg;
	RfSrcImg.Copy(SrcImg);

	FlowCalculator_Black FlowCal; 
	//FlowCalculator_LiuCe FlowCal; ////
	int iWidth = SrcImg.GetWidth();
	int iHeight = SrcImg.GetHeight();
	float radio = iWidth / FlowCal.minwidth;
	std::cout<<radio<<std::endl;
	int level = (int)(log(radio) / log(2.0f)+1);
	if(level <= 3)
		level = 4;
	FlowCal.SetLevel(level);
	printf("level: %d\n", level);
    
	int n=1;
	while(imageBuffer.Forward()&&n!=0)
	{
		printf("Frame %d\n", imageBuffer.GetFramePos() - 1);
		CString OptFlowUName = OptFlowPath;
		CString OptFlowVName = OptFlowPath;
		SrcImg = RfSrcImg;                     //SrcImg前一帧图像
		RfSrcImg.Copy(*imageBuffer.GetImage());    //RfSrcImg当前帧图像

		float* U = new float[iWidth * iHeight];
		memset(U, 0, sizeof(float) * iWidth * iHeight);
		float* V = new float[iWidth * iHeight];
		memset(V, 0, sizeof(float) * iWidth * iHeight);
		FlowCal.CalOpticalFlow(&SrcImg, &RfSrcImg, U, V);                    //光流计算的关键……理解不清楚
		int index = imageBuffer.GetFramePos() - 1;

		//CString flowimpath = flowfilepathu;
		//flowimpath.AppendFormat("\\u_%d_%d.png", index, index+1);

		//SaveOpticalIm(u, width, height, flowimpath.GetBuffer());
		//flowimpath = flowfilepathv;
		//flowimpath.AppendFormat("\\v_%d_%d.png", index, index+1);

		//SaveOpticalIm(v, width, height, flowimpath.GetBuffer());

		OptFlowUName.AppendFormat("\\u_%d_%d.raw", index, index + 1);
		OptFlowVName.AppendFormat("\\v_%d_%d.raw", index, index + 1);



		                                                  //将光流计算的结果v和u存入到opt目录下的v和u文件中

		FILE* pFile;
		pFile = fopen(OptFlowUName.GetBuffer(), "wb");                   
		if(pFile)
		{
			fwrite(U, iWidth*iHeight, sizeof(float), pFile);
			fclose(pFile);
		}
		pFile = fopen(OptFlowVName.GetBuffer(), "wb");
		if(pFile)
		{
			fwrite(V, iWidth*iHeight, sizeof(float), pFile);
			fclose(pFile);
		}
                                                              
		                                                      
		{
			                                                             //将U和V的值赋给FlowMap
			CFloatImage FlowMap;
			FlowMap.ReAllocate(CShape(iWidth, iHeight, 2));
			for(int y = 0, p = 0; y < iHeight; y++)
			{
				for(int x = 0; x < iWidth; x++, p++)
				{
					FlowMap.Pixel(x, iHeight - 1 - y, 0) = U[p];
					FlowMap.Pixel(x, iHeight - 1 - y, 1) = V[p];
				}
			}



			CByteImage FlowImg;
			MotionToColor(FlowMap, FlowImg, -1);
			CxImage OptFlowCxImg(iWidth, iHeight, 24);
			for(int y = 0; y < iHeight; y++)
			{
				for(int x = 0; x < iWidth; x++)
				{
					RGBQUAD c;
					c.rgbRed = FlowImg.Pixel(x, y, 0);
					c.rgbGreen = FlowImg.Pixel(x, y, 1);
					c.rgbBlue = FlowImg.Pixel(x, y, 2);
					OptFlowCxImg.SetPixelColor(x, iHeight - 1 - y, c);
				}
			}
			CString OptFlowName = OptFlowPath;
			OptFlowName.AppendFormat("\\OptFlowMap_%d_%d.png", index, index + 1);
			OptFlowCxImg.Save((LPCTSTR)OptFlowName, CXIMAGE_FORMAT_PNG);               //存入光流计算的结果png文件到opt目录下
		}

		//back-wards optical flow
		memset(U, 0, sizeof(float) * iWidth * iHeight);
		memset(V, 0, sizeof(float) * iWidth * iHeight);

		FlowCal.CalOpticalFlow(&RfSrcImg, &SrcImg, U, V);
		OptFlowUName = OptFlowPath;
		OptFlowVName = OptFlowPath;

		//flowimpath = flowfilepathu;
		//flowimpath.AppendFormat("\\u_%d_%d.png", index+1, index);

		//SaveOpticalIm(u, width, height, flowimpath.GetBuffer());
		//flowimpath = flowfilepathv;
		//flowimpath.AppendFormat("\\v_%d_%d.png", index+1, index);

		//SaveOpticalIm(v, width, height, flowimpath.GetBuffer());

		OptFlowUName.AppendFormat("\\u_%d_%d.raw", index + 1, index);
		OptFlowVName.AppendFormat("\\v_%d_%d.raw", index + 1, index);

		pFile = fopen(OptFlowUName.GetBuffer(), "wb");
		if(pFile)
		{
			fwrite(U, iWidth*iHeight, sizeof(float), pFile);
			fclose(pFile);
		}
		pFile = fopen(OptFlowVName.GetBuffer(), "wb");
		if(pFile)
		{
			fwrite(V, iWidth*iHeight, sizeof(float), pFile);
			fclose(pFile);
		}

		{
			CFloatImage FlowMap;
			FlowMap.ReAllocate(CShape(iWidth, iHeight, 2));
			for(int y = 0, p = 0; y < iHeight; y++)
			{
				for(int x = 0; x < iWidth; x++, p++)
				{
					FlowMap.Pixel(x, iHeight - 1 - y, 0) = U[p];
					FlowMap.Pixel(x, iHeight - 1 - y, 1) = V[p];
				}
			}
			CByteImage FlowImg;
			MotionToColor(FlowMap, FlowImg, -1);
			CxImage OptFlowCxImg(iWidth, iHeight, 24);
			for(int y = 0; y < iHeight; y++)
			{
				for(int x = 0; x < iWidth; x++)
				{
					RGBQUAD c;
					c.rgbRed = FlowImg.Pixel(x, y, 0);
					c.rgbGreen = FlowImg.Pixel(x, y, 1);
					c.rgbBlue = FlowImg.Pixel(x, y, 2);
					OptFlowCxImg.SetPixelColor(x, iHeight - 1 - y, c);
				}
			}
			CString OptFlowName = OptFlowPath;
			OptFlowName.AppendFormat("\\OptFlowMap_%d_%d.png", index + 1, index);
			OptFlowCxImg.Save((LPCTSTR)OptFlowName, CXIMAGE_FORMAT_PNG);
		}

		delete [] U;
		delete [] V;
		
		n--;
	}
}
*/ 

void CVideoEditingView::OnPreprocessOpticalflow()    
{
	// TODO: Add your command handler code here
	CString OptFlowPath = filePath;

	OptFlowPath.AppendFormat("%s","opt");
	if(!CreateDirectory(OptFlowPath, 0))
	{
		if(MessageBox((LPCTSTR)"Optical flow already exists, do you want to over write it?","Optical flow", MB_YESNO) == IDNO)
			return;
	}
	imageBuffer.Goto(0);
	CxImage SrcImg;
	SrcImg.Copy(*imageBuffer.GetImage());
	CxImage RfSrcImg;
	RfSrcImg.Copy(SrcImg);



//	FlowCalculator_Black FlowCal; 
	//FlowCalculator_LiuCe FlowCal; ////
	int iWidth = SrcImg.GetWidth();
	int iHeight = SrcImg.GetHeight();
//	float radio = iWidth / FlowCal.minwidth;
//	std::cout<<radio<<std::endl;
//	int level = (int)(log(radio) / log(2.0f)+1);
//	if(level <= 3)
//		level = 4;
//	FlowCal.SetLevel(level);
//	printf("level: %d\n", level);

	//int n=15;
	while(imageBuffer.Forward()/*&&n!=0*/)
	{
		printf("Frame %d\n", imageBuffer.GetFramePos() - 1);
		CString OptFlowUName = OptFlowPath;
		CString OptFlowVName = OptFlowPath;
		SrcImg = RfSrcImg;                     //SrcImg前一帧图像
		RfSrcImg.Copy(*imageBuffer.GetImage());    //RfSrcImg当前帧图像

          
		DImage Im1(iWidth,iHeight,3);
		double* pImgDt=Im1.data();

		RGBQUAD rgb;
		for(int x=0;x<iWidth;x++)
		{
			for(int y=0;y<iHeight;y++)
			{

				rgb=SrcImg.GetPixelColor(x,y);

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

				rgb=RfSrcImg.GetPixelColor(x,y);

				pImgDt[y*iWidth*3+x*3]=double(rgb.rgbRed/255.0);
				pImgDt[y*iWidth*3+x*3+1]=double(rgb.rgbGreen/255.0);
				pImgDt[y*iWidth*3+x*3+2]=double(rgb.rgbBlue/255.0);
			}
		}


		DImage V_x,V_y,WrpI2;
	//	OpticalFlow::Coarse2FineFlow(V_x,V_y,WrpI2,Im1,Im2,alpha,Rt,MnWd,ItrNm_OtFwPDE,ItrNm_InFwPDE,ItrNm_CG);
	    OpticalFlow::Coarse2FineFlow(V_x,V_y,WrpI2,Im1,Im2,0.01,0.75,30,15,1,40);
	//	static Wml::GMatrix<Wml::Vector2f> GenOpticalFlow(CxImage srcImg, CxImage dstImg,const RECT& contrRct,double alpha = 0.01, double Rt = 0.75, int MnWd = 30, int ItrNm_OtFwPDE = 15, int ItrNm_InFwPDE = 1, int ItrNm_CG = 40);
        
        

		float* U = new float[iWidth * iHeight];
		memset(U, 0, sizeof(float) * iWidth * iHeight);
		float* V = new float[iWidth * iHeight];
		memset(V, 0, sizeof(float) * iWidth * iHeight);

//		FlowCal.CalOpticalFlow(&SrcImg, &RfSrcImg, U, V);                    //光流计算的关键……理解不清楚


	//代码实现
         //U=V_x;
		 //V=V_y;

	
		for(int x=0;x<iWidth;x++)
		{
			for(int y=0;y<iHeight;y++)
			{

				U[y*iWidth+x]=V_x.data()[y*iWidth+x];
				V[y*iWidth+x]=V_y.data()[y*iWidth+x];
			
			}
		}

		int index = imageBuffer.GetFramePos() - 1;

		//CString flowimpath = flowfilepathu;
		//flowimpath.AppendFormat("\\u_%d_%d.png", index, index+1);

		//SaveOpticalIm(u, width, height, flowimpath.GetBuffer());
		//flowimpath = flowfilepathv;
		//flowimpath.AppendFormat("\\v_%d_%d.png", index, index+1);

		//SaveOpticalIm(v, width, height, flowimpath.GetBuffer());

		OptFlowUName.AppendFormat("\\u_%d_%d.raw", index, index + 1);
		OptFlowVName.AppendFormat("\\v_%d_%d.raw", index, index + 1);


		//将光流计算的结果v和u存入到opt目录下的v和u文件中

		FILE* pFile;
		pFile = fopen(OptFlowUName.GetBuffer(), "wb");                   
		if(pFile)
		{
			fwrite(U, iWidth*iHeight, sizeof(float), pFile);
			fclose(pFile);
		}
		pFile = fopen(OptFlowVName.GetBuffer(), "wb");
		if(pFile)
		{
			fwrite(V, iWidth*iHeight, sizeof(float), pFile);
			fclose(pFile);
		}

	
		{

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

		 //  	const char* optRlst="D://OptFlwImg.png";
		//	OpFwImg.Save(optRlst,CXIMAGE_FORMAT_PNG);
			
				/*
			//将U和V的值赋给FlowMap
			CFloatImage FlowMap;
			FlowMap.ReAllocate(CShape(iWidth, iHeight, 2));
			for(int y = 0, p = 0; y < iHeight; y++)
			{
				for(int x = 0; x < iWidth; x++, p++)
				{
					FlowMap.Pixel(x, iHeight - 1 - y, 0) = U[p];
					FlowMap.Pixel(x, iHeight - 1 - y, 1) = V[p];
				}
			}



			CByteImage FlowImg;
			MotionToColor(FlowMap, FlowImg, -1);
			CxImage OptFlowCxImg(iWidth, iHeight, 24);
			for(int y = 0; y < iHeight; y++)
			{
				for(int x = 0; x < iWidth; x++)
				{
					RGBQUAD c;
					c.rgbRed = FlowImg.Pixel(x, y, 0);
					c.rgbGreen = FlowImg.Pixel(x, y, 1);
					c.rgbBlue = FlowImg.Pixel(x, y, 2);
					OptFlowCxImg.SetPixelColor(x, iHeight - 1 - y, c);
				}
			}
			  */
			CString OptFlowName = OptFlowPath;
			OptFlowName.AppendFormat("\\OptFlowMap_%d_%d.png", index, index + 1);
			OpFwImg.Save((LPCTSTR)OptFlowName, CXIMAGE_FORMAT_PNG);               //存入光流计算的结果png文件到opt目录下
          
		}

		//back-wards optical flow

		delete [] U;
		delete [] V;

		//n--;
	}
	
}

void CVideoEditingView::SaveOpticalIm(float* flow, int width, int height, char* name)
{
	CxImage temp;
	temp.Create(width, height, 8);
	temp.SetGrayPalette();
	float max = 0;
	for (int j=0; j<height; ++j)
	{
		for (int i=0; i<width; ++i)
		{
			int index = j*width+i;
			if (fabs(flow[index])>max)
			{
				max = fabs(flow[index]);
			}
		}
	}

	for (int j=0; j<height; ++j)
	{
		for (int i=0; i<width; ++i)
		{
			int index = j*width+i;
			int col =fabs(flow[index])/max*255;
			temp.SetPixelIndex(i,j, col);
		}
	}
	temp.Save(name, CXIMAGE_FORMAT_PNG);
}

int CVideoEditingView::CheckTrimapExist()
{
	int startindex = vec_layer[current_layer].GetStartIndex();
	for (int i=startIndex; ;++i) 
	{
		CString trimapname = trimapBuffer.GetFrameName(i);
				if (_access(trimapname.GetBuffer(),0)==-1)//文件不存在
					return i-1;
	}
}
int CVideoEditingView::CheckLabelExist()
{
	int startindex = vec_layer[current_layer].GetStartIndex();
	for (int i=startIndex;;++i)
	{
		CString labelname = alphaBuffer.GetFrameName(i);
		if(_access(labelname.GetBuffer(),0)==-1)
			return i-1;
	}
}

#include "TemporalMattingManager.h"
#include "TemporalMattingDlg.h"
void CVideoEditingView::OnRunTemporalmatting()
{
	// TODO: Add your command handler code here
	CString layername = vec_layer[current_layer].GetName();
	int startframe = vec_layer[current_layer].GetStartIndex();
	int endframe = CheckTrimapExist();
	if (endframe<startframe)
	{
		AfxMessageBox("Generate trimap first");
		return;
	}
	CTemporalMattingDlg temporalmd(layername, startframe, endframe, 4, 1.0,10000.0);
	if (temporalmd.DoModal()==IDOK)
	{
		int _startframe = temporalmd.m_startFrame;
		int _endframe = temporalmd.m_endFrame;
		startframe = (startframe<_startframe)?_startframe:startframe;
		endframe = (endframe>_endframe)?_endframe:endframe;
		int step = temporalmd.m_step;
		float fTmpWeight = temporalmd.m_temporalWeight;

		CString OptFlowPath = filePath;
		OptFlowPath.AppendFormat("%s\\opt\\", vec_layer[current_layer].GetName());
		CString OptFlowUName = OptFlowPath;
		OptFlowUName.AppendFormat("u_%d_%d.raw", startframe, startframe + 1);
		CString OptFlowVName = OptFlowPath;
		OptFlowVName.AppendFormat("v_%d_%d.raw", startframe, startframe + 1);
		if(_access(OptFlowUName.GetBuffer(), 0) == -1 && _access(OptFlowVName.GetBuffer(), 0) == -1)
		{
			OptFlowPath = filePath;
			OptFlowPath.Append("opt\\");
		}
		std::cout<<"Optical Flow Path: "<<OptFlowPath<<std::endl;

		//TemporalMattingManager tmm(&(this->imageBuffer), &(this->trimapBuffer), std::string("H:/2D to 3D/data/hair2/opt"));
		//tmm.PathMatting(0, 19, 4);
		ImageBuffer softAlphaBuffer;
		CString path = filePath;
		path.AppendFormat("%s/%s", vec_layer[current_layer].GetName(),"VEAlpha");
		CreateDirectory(path,0);
		path.AppendFormat("/%s", filename);
		softAlphaBuffer.SetName(path, alphaExt);

		ImageBuffer FGBuffer,BGBuffer;//foreground and background
		CString fbpath = filePath;
		fbpath.AppendFormat("%s/%s", vec_layer[current_layer].GetName(),"FG");
		fbpath.AppendFormat("/%s", filename);
		FGBuffer.SetName(fbpath, alphaExt);
		fbpath = filePath;
		fbpath.AppendFormat("%s/%s", vec_layer[current_layer].GetName(),"BG");
		fbpath.AppendFormat("/%s", filename);
		BGBuffer.SetName(fbpath, alphaExt);

		double start = clock();
		TemporalMattingManager tmm(&(this->imageBuffer), &(this->trimapBuffer), 
			&softAlphaBuffer, &(this->resImageBuffer),&FGBuffer, &BGBuffer,std::string(OptFlowPath.GetBuffer()), fTmpWeight);
		std::cout<<"AnchorFirst:" << temporalmd.m_bAnchorFirst << "AnchorEnd:" << temporalmd.m_bAnchorLast <<std::endl;
		tmm.PathMatting(startframe, endframe,step, temporalmd.m_bAnchorFirst, temporalmd.m_bAnchorLast);
		double end = clock();
		double duration=(end-start)/CLK_TCK;
		std::cout<<"Temporal Matting Time:"<<duration<<std::endl;
	}
}

#include "FlowCalculator_LiuCe.h"
#include "colorcode.h"
//void CVideoEditingView::OnPostprocessLayeropticalflow()
//{
//	// TODO: Add your command handler code here
//	for (int i=0; i<vec_layer.size(); ++i)
//	{
//		int start = vec_layer[i].GetStartIndex();
//		ImageBuffer fgbuffer;
//		CString fbpath = filePath;
//		fbpath.AppendFormat("%s/%s", vec_layer[i].GetName(),"FG");
//		fbpath.AppendFormat("/%s", filename);
//		fgbuffer.SetName(fbpath, alphaExt);
//		fgbuffer.OpenVideo(start);
//		int width = fgbuffer.GetImage()->GetWidth();
//		int height = fgbuffer.GetImage()->GetHeight();
//		CxImage fim, sim;//first second image
//		fim.Copy(*(fgbuffer.GetImage()));
//		float* u = new float[width*height];
//		float* v = new float[width*height];
//		CString path = filePath;
//		path.AppendFormat("%s/%s/", vec_layer[i].GetName(),"opt");
//		 while (fgbuffer.Forward())
//		 {
//			 memset(u, 0, sizeof(float)*width*height);
//			 memset(v, 0, sizeof(float)*width*height);
//			 sim.Copy(*(fgbuffer.GetImage()));
//			 FlowCalculator_LiuCe fllc;
//			// FlowCalculator_Black flb;
//			 fllc.OpticalFlow(fim, sim, u, v);
//			// flb.CalOpticalFlow(&fim, &sim,u,v);
//			 printf("flow:::%f %f",u[0], v[0]);
//			//save file
//			 CString pathsave=path;
//			 int pos = fgbuffer.GetFramePos()-1;
//			 pathsave.AppendFormat("u_%d_%d.raw",pos, pos+1 );
//			 FILE* pfile;
//			 pfile = fopen(pathsave.GetBuffer(), "wb");
//			 if (pfile)
//			 {
//				 fwrite(u, sizeof(float), width*height, pfile);
//				 fclose(pfile);
//			 }
//			 std::cout<<pathsave<<std::endl;
//			 pathsave = path;
//			 pathsave.AppendFormat("v_%d_%d.raw", pos, pos+1);
//			 pfile = fopen(pathsave.GetBuffer(), "wb");
//			 if (pfile)
//			 {
//				 fwrite(v, sizeof(float), width*height, pfile);
//				 fclose(pfile);
//			 }	
//			 memset(u, 0, sizeof(float)*width*height);
//			 memset(v, 0, sizeof(float)*width*height);
//			 fllc.OpticalFlow(sim, fim, u, v);
//			 //save file
//			 std::cout<<pathsave<<std::endl;
//			 pathsave = path;
//			 pathsave.AppendFormat("u_%d_%d.raw", pos+1, pos);
//			 pfile = fopen(pathsave.GetBuffer(), "wb");
//			 if (pfile)
//			 {
//				 fwrite(u, sizeof(float), width*height, pfile);
//				 fclose(pfile);
//			 }
//			 std::cout<<pathsave<<std::endl;
//			 pathsave = path;
//			 pathsave.AppendFormat("v_%d_%d.raw",pos+1, pos);
//			 pfile = fopen(pathsave.GetBuffer(),"wb");
//			 if (pfile)
//			 {
//				 fwrite(v, sizeof(float), width*height, pfile);
//				 fclose(pfile);
//			 }
//			 std::cout<<pathsave<<std::endl;
//			 fim.Copy(sim);
//		 }
//		 delete[] u;
//		 delete[] v;
//	}
//}

void CVideoEditingView::OnPostprocessLayeropticalflow()
{
	// TODO: Add your command handler code here

	for(int k = 0; k < vec_layer.size(); ++k)
	{
		int iStartIdx = vec_layer[k].GetStartIndex();
		imageBuffer.Goto(iStartIdx);
		CxImage* pCxImg = imageBuffer.GetImage();
		int iWidth = pCxImg->GetWidth();
		int iHeight = pCxImg->GetHeight();

		std::vector<float> U(iWidth * iHeight);
		std::vector<float> V(iWidth * iHeight);

		ImageBuffer LyrMskBuffer;
		CString LyrMskPath = filePath;
		LyrMskPath.AppendFormat("%s/VELabel/", vec_layer[k].GetName());
		LyrMskPath.AppendFormat("%s", filename);
		LyrMskBuffer.SetName(LyrMskPath, alphaExt);

		ImageBuffer TriMapBuffer;
		CString TriMapPath = filePath;
		TriMapPath.AppendFormat("%s/VETrimap/", vec_layer[k].GetName());
		TriMapPath.AppendFormat("%s", filename);
		TriMapBuffer.SetName(TriMapPath, alphaExt);
		//LabelBuffer.OpenVideo(start);

		CxImage LyrImg, RfLyrImg;//First/Second Image
		CString LyrMskName = LyrMskBuffer.GetFrameName(iStartIdx);
		if(_access(LyrMskName.GetBuffer(), 0) != -1)
		{
			CxImage LyrMskCxImg;
			LyrMskCxImg.Load(LyrMskName.GetBuffer());
			LyrMskCxImg.GrayScale();
			LyrImg.Copy(*imageBuffer.GetImage());
			for(int y = 0; y < iHeight; ++y)
			{
				for(int x = 0; x < iWidth; ++x)
				{
					if(!LyrMskCxImg.GetPixelIndex(x, y))
					{
						RGBQUAD c;
						c.rgbRed = c.rgbGreen = c.rgbBlue = c.rgbReserved = 0;
						LyrImg.SetPixelColor(x, y, c);
					}
				}
			}
		}

		CString OptFlowPath = filePath;
		OptFlowPath.AppendFormat("%s/opt/", vec_layer[k].GetName());

		while(imageBuffer.Forward())
		{
			int iFrmIdx = imageBuffer.GetFramePos();

			LyrMskName = LyrMskBuffer.GetFrameName(iFrmIdx);
			if(_access(LyrMskName.GetBuffer(), 0) != -1)
			{
				CxImage LyrMskCxImg;
				LyrMskCxImg.Load(LyrMskName.GetBuffer());
				LyrMskCxImg.GrayScale();
				RfLyrImg.Copy(*imageBuffer.GetImage());
				for(int y = 0; y < iHeight; ++y)
				{
					for(int x = 0; x < iWidth; ++x)
					{
						if(!LyrMskCxImg.GetPixelIndex(x, y))
						{
							RGBQUAD c;
							c.rgbRed = c.rgbGreen = c.rgbBlue = c.rgbReserved = 0;
							RfLyrImg.SetPixelColor(x, y, c);
						}
					}
				}
			}
			else
				RfLyrImg.Clear();

			if(LyrImg.IsValid() && RfLyrImg.IsValid())
			{
				memset(&U[0], 0, sizeof(float) * iWidth * iHeight);
				memset(&V[0], 0, sizeof(float) * iWidth * iHeight);
				FlowCalculator_LiuCe fllc;
				//FlowCalculator_Black flb;
				fllc.OpticalFlow(LyrImg, RfLyrImg, &U[0], &V[0]);
				//flb.CalOpticalFlow(&fim, &sim, &u[0], &v[0]);
				CString LyrTriMapName = TriMapBuffer.GetFrameName(iFrmIdx - 1);
				if(_access(LyrTriMapName.GetBuffer(), 0) != -1)
				{
					CxImage LyrTriMap;
					LyrTriMap.Load(LyrTriMapName.GetBuffer());
					LyrTriMap.GrayScale();

					ZFloatImage OptFlowMap(iWidth, iHeight, 2);
					for(int y = 0, p = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++, p++)
						{
							OptFlowMap.at(x, iHeight - 1 - y, 0) = U[p];
							OptFlowMap.at(x, iHeight - 1 - y, 1) = V[p];
						}
					}

					ZIntImage IptMsk(iWidth, iHeight);
					for(int y = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++)
						{
							BYTE TriVal = LyrTriMap.GetPixelIndex(x, iHeight - 1 - y);
							if(TriVal == 255)
								IptMsk.at(x, y) = 0;
							else if(TriVal == 0)
								IptMsk.at(x, y) = -1;
							else
								IptMsk.at(x, y) = 1;
						}
					}
					OptFlowMap = CInpainting::OptFlowInpainting(OptFlowMap, IptMsk);

					for(int y = 0, p = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++, p++)
						{
							U[p] = OptFlowMap.at(x, iHeight - 1 - y, 0);
							V[p] = OptFlowMap.at(x, iHeight - 1 - y, 1);
						}
					}
				}
				//Save File
				CString OptFlowName = OptFlowPath;
				OptFlowName.AppendFormat("u_%d_%d.raw", iFrmIdx - 1, iFrmIdx);
				FILE* pFile = fopen(OptFlowName.GetBuffer(), "wb+");
				if(pFile)
				{
					fwrite(&U[0], sizeof(float), iWidth * iHeight, pFile);
					fclose(pFile);
				}
				std::cout<<OptFlowName<<std::endl;
				OptFlowName = OptFlowPath;
				OptFlowName.AppendFormat("v_%d_%d.raw", iFrmIdx - 1, iFrmIdx);
				pFile = fopen(OptFlowName.GetBuffer(), "wb+");
				if(pFile)
				{
					fwrite(&V[0], sizeof(float), iWidth * iHeight, pFile);
					fclose(pFile);
				}	
				std::cout<<OptFlowName<<std::endl;
				{
					CFloatImage FlowMap;
					FlowMap.ReAllocate(CShape(iWidth, iHeight, 2));
					for(int y = 0, p = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++, p++)
						{
							FlowMap.Pixel(x, iHeight - 1 - y, 0) = U[p];
							FlowMap.Pixel(x, iHeight - 1 - y, 1) = V[p];
						}
					}
					CByteImage FlowImg;
					MotionToColor(FlowMap, FlowImg, -1);
					CxImage OptFlowCxImg(iWidth, iHeight, 24);
					for(int y = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++)
						{
							RGBQUAD c;
							c.rgbRed = FlowImg.Pixel(x, y, 0);
							c.rgbGreen = FlowImg.Pixel(x, y, 1);
							c.rgbBlue = FlowImg.Pixel(x, y, 2);
							OptFlowCxImg.SetPixelColor(x, iHeight - 1 - y, c);
						}
					}
					OptFlowName = OptFlowPath;
					OptFlowName.AppendFormat("OptFlowMap_%d_%d.png", iFrmIdx - 1, iFrmIdx);
					OptFlowCxImg.Save((LPCTSTR)OptFlowName, CXIMAGE_FORMAT_PNG);
				}

				memset(&U[0], 0, sizeof(float) * iWidth * iHeight);
				memset(&V[0], 0, sizeof(float) * iWidth * iHeight);
				fllc.OpticalFlow(RfLyrImg, LyrImg, &U[0], &V[0]);
				LyrTriMapName = TriMapBuffer.GetFrameName(iFrmIdx);
				if(_access(LyrTriMapName.GetBuffer(), 0) != -1)
				{
					CxImage LyrTriMap;
					LyrTriMap.Load(LyrTriMapName.GetBuffer());
					LyrTriMap.GrayScale();

					ZFloatImage OptFlowMap(iWidth, iHeight, 2);
					for(int y = 0, p = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++, p++)
						{
							OptFlowMap.at(x, iHeight - 1 - y, 0) = U[p];
							OptFlowMap.at(x, iHeight - 1 - y, 1) = V[p];
						}
					}

					ZIntImage IptMsk(iWidth, iHeight);
					for(int y = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++)
						{
							BYTE TriVal = LyrTriMap.GetPixelIndex(x, iHeight - 1 - y);
							if(TriVal == 255)
								IptMsk.at(x, y) = 0;
							else if(TriVal == 0)
								IptMsk.at(x, y) = -1;
							else
								IptMsk.at(x, y) = 1;
						}
					}
					OptFlowMap = CInpainting::OptFlowInpainting(OptFlowMap, IptMsk);

					for(int y = 0, p = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++, p++)
						{
							U[p] = OptFlowMap.at(x, iHeight - 1 - y, 0);
							V[p] = OptFlowMap.at(x, iHeight - 1 - y, 1);
						}
					}
				}
				//Save File
				OptFlowName = OptFlowPath;
				OptFlowName.AppendFormat("u_%d_%d.raw", iFrmIdx, iFrmIdx - 1);
				pFile = fopen(OptFlowName.GetBuffer(), "wb");
				if(pFile)
				{
					fwrite(&U[0], sizeof(float), iWidth * iHeight, pFile);
					fclose(pFile);
				}
				std::cout<<OptFlowName<<std::endl;
				OptFlowName = OptFlowPath;
				OptFlowName.AppendFormat("v_%d_%d.raw", iFrmIdx, iFrmIdx - 1);
				pFile = fopen(OptFlowName.GetBuffer(),"wb");
				if(pFile)
				{
					fwrite(&V[0], sizeof(float), iWidth * iHeight, pFile);
					fclose(pFile);
				}
				std::cout<<OptFlowName<<std::endl;
				{
					CFloatImage FlowMap;
					FlowMap.ReAllocate(CShape(iWidth, iHeight, 2));
					for(int y = 0, p = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++, p++)
						{
							FlowMap.Pixel(x, iHeight - 1 - y, 0) = U[p];
							FlowMap.Pixel(x, iHeight - 1 - y, 1) = V[p];
						}
					}
					CByteImage FlowImg;
					MotionToColor(FlowMap, FlowImg, -1);
					CxImage OptFlowCxImg(iWidth, iHeight, 24);
					for(int y = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++)
						{
							RGBQUAD c;
							c.rgbRed = FlowImg.Pixel(x, y, 0);
							c.rgbGreen = FlowImg.Pixel(x, y, 1);
							c.rgbBlue = FlowImg.Pixel(x, y, 2);
							OptFlowCxImg.SetPixelColor(x, iHeight - 1 - y, c);
						}
					}
					OptFlowName = OptFlowPath;
					OptFlowName.AppendFormat("OptFlowMap_%d_%d.png", iFrmIdx, iFrmIdx - 1);
					OptFlowCxImg.Save((LPCTSTR)OptFlowName, CXIMAGE_FORMAT_PNG);
				}
			}

			LyrImg.Copy(RfLyrImg);
		}
	}
}

#include "RefineWithTrimap.h"
#include "TrimapWidthSetDlg.h"
void CVideoEditingView::OnRefineTrimap()
{
	CTrimapWidthSetDlg trimapDlg( this);
	int response = trimapDlg.DoModal();
	int window_size=10;
	if (response == IDOK)
		window_size = trimapDlg.GetTrimapWidth();

	CxImage* src = imageBuffer.GetImage();
	CxImage* label = alphaBuffer.GetImage();
	CxImage trimap;
	trimap.Load(trimapBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);

	RefineWithTrimap rwt;
	rwt.m_winsize = window_size;
	rwt.Refine(*src, *label, trimap);
	label->Save(alphaBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);

	CxImage result;
	result.Copy(*src);
	for (int j=0;j <label->GetHeight();++j)
	{
		for(int i=0; i<label->GetWidth(); ++i)
		{
			if (label->GetPixelGray(i,j)==0)
			{
				result.SetPixelColor(i,j,RGB(0,0,0));
			}
		}
	}
	result.Save(resImageBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);
}
