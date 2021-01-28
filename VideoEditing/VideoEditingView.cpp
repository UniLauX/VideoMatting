// VideoEditingView.cpp : implementation of the CVideoEditingView class
//

#include "stdafx.h"
#include "VideoEditing.h"
#include "VideoEditingDoc.h"
#include "VideoEditingView.h"

#include "PlayControl.h"
#include "ParaDlg.h"
#include "LocalParam.h"
#include <iostream>
#include <fstream>
#include <WinAble.h>
#include "MainFrm.h"
#include <io.h>
#include "LayerEditDlg.h"
#include "Vision/Others/opencv10/include/cv.h"
#include "Vision/Others/OpenCV10/include/highgui.h"
#include "VEFileParser.h"
#include "VEProject.h"
#include "TrimapGenerator.h"
#include "Polygon.h"
#include "TrimapWidthSetDlg.h"
#include "ClosedFormParam.h"
#include "Disparity.h"
#include "ImageConvert.h"
#include "Inpainting.h"
#include "BeizerCurveDataManager.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// CVideoEditingView

IMPLEMENT_DYNCREATE(CVideoEditingView, CView)

BEGIN_MESSAGE_MAP(CVideoEditingView, CView)
    // Standard printing commands
    ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
    ON_MESSAGE(WM_COMMAND_RENDER, &CVideoEditingView::OnRender)
    ON_COMMAND(ID_FILE_OPEN, &CVideoEditingView::OnFileOpen)
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_COMMAND(ID_GRABBER_RECTANGLEGRAB, &CVideoEditingView::OnGrabberRectanglegrab)//draw rectangle 
    ON_COMMAND(ID_GRABBER_FOREGROUNDBRUSH, &CVideoEditingView::OnGrabberForegroundbrush)
    ON_COMMAND(ID_GRABBER_BACKGROUNDBRUSH, &CVideoEditingView::OnGrabberBackgroundbrush)
    ON_UPDATE_COMMAND_UI(ID_GRABBER_RECTANGLEGRAB, &CVideoEditingView::OnUpdateGrabberRectanglegrab)
    ON_UPDATE_COMMAND_UI(ID_GRABBER_FOREGROUNDBRUSH, &CVideoEditingView::OnUpdateGrabberForegroundbrush)
    ON_UPDATE_COMMAND_UI(ID_GRABBER_BACKGROUNDBRUSH, &CVideoEditingView::OnUpdateGrabberBackgroundbrush)
    ON_COMMAND(ID_VIEW_EDITWINDOW, &CVideoEditingView::OnViewEditwindow)
    ON_COMMAND(ID_VIEW_RESULTWINDOW, &CVideoEditingView::OnViewResultwindow)
    ON_UPDATE_COMMAND_UI(ID_VIEW_EDITWINDOW, &CVideoEditingView::OnUpdateViewEditwindow)
    ON_UPDATE_COMMAND_UI(ID_VIEW_RESULTWINDOW, &CVideoEditingView::OnUpdateViewResultwindow)
    ON_COMMAND(ID_RUN_RUN, &CVideoEditingView::OnRunRun)
    ON_COMMAND(ID_MODE_PICTURE, &CVideoEditingView::OnModePicture)
    ON_COMMAND(ID_MODE_VIDEO, &CVideoEditingView::OnModeVideo)
    ON_UPDATE_COMMAND_UI(ID_MODE_PICTURE, &CVideoEditingView::OnUpdateModePicture)
    ON_UPDATE_COMMAND_UI(ID_MODE_VIDEO, &CVideoEditingView::OnUpdateModeVideo)
    ON_COMMAND(ID_GRABBER_GRABCUT, &CVideoEditingView::OnGrabberGrabcut)
    ON_COMMAND(ID_GRABBER_CLOSEDFORMMATTING, &CVideoEditingView::OnGrabberClosedformmatting)
    ON_UPDATE_COMMAND_UI(ID_GRABBER_GRABCUT, &CVideoEditingView::OnUpdateGrabberGrabcut)
    ON_UPDATE_COMMAND_UI(ID_GRABBER_CLOSEDFORMMATTING, &CVideoEditingView::OnUpdateGrabberClosedformmatting)
    ON_COMMAND(ID_SETTING_PREFERENCE, &CVideoEditingView::OnSettingPreference)
    ON_COMMAND(ID_EDIT_MOVE, &CVideoEditingView::OnEditMove)
    ON_UPDATE_COMMAND_UI(ID_EDIT_MOVE, &CVideoEditingView::OnUpdateEditMove)
    ON_COMMAND(ID_EDIT_FIT, &CVideoEditingView::OnEditFit)
    ON_COMMAND(ID_EDIT_ZOOMIN, &CVideoEditingView::OnEditZoomin)
	ON_COMMAND(ID_BUTTON_AddNewLayer, &CVideoEditingView::OnAddNewLayer)
    ON_WM_MOUSEWHEEL()
    ON_COMMAND(ID_EDIT_ZOOMOUT, &CVideoEditingView::OnEditZoomout)
    ON_COMMAND(ID_RUN_STOP, &CVideoEditingView::OnRunStop)
    ON_COMMAND(ID_RUN_RUNVIDEO, &CVideoEditingView::OnRunRunvideo)
    ON_COMMAND(ID_UPDATE_GLOBALUPDATE, &CVideoEditingView::OnUpdateGlobalupdate)
    ON_COMMAND(ID_UPDATE_LOCALUPDATE, &CVideoEditingView::OnUpdateLocalupdate)
    ON_UPDATE_COMMAND_UI(ID_UPDATE_GLOBALUPDATE, &CVideoEditingView::OnUpdateUpdateGlobalupdate)
    ON_UPDATE_COMMAND_UI(ID_UPDATE_LOCALUPDATE, &CVideoEditingView::OnUpdateUpdateLocalupdate)
    ON_COMMAND(ID_PEN_FOREGROUND, &CVideoEditingView::OnPenForeground)
    ON_COMMAND(ID_PEN_BACKGROUND, &CVideoEditingView::OnPenBackground)
    ON_UPDATE_COMMAND_UI(ID_PEN_FOREGROUND, &CVideoEditingView::OnUpdatePenForeground)
    ON_UPDATE_COMMAND_UI(ID_PEN_BACKGROUND, &CVideoEditingView::OnUpdatePenBackground)
    ON_UPDATE_COMMAND_UI(ID_RUN_RUN, &CVideoEditingView::OnUpdateRunRun)
    ON_UPDATE_COMMAND_UI(ID_RUN_STOP, &CVideoEditingView::OnUpdateRunStop)
    ON_UPDATE_COMMAND_UI(ID_RUN_RUNVIDEO, &CVideoEditingView::OnUpdateRunRunvideo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FIT, &CVideoEditingView::OnUpdateEditFit)
    ON_UPDATE_COMMAND_UI(ID_EDIT_ZOOMIN, &CVideoEditingView::OnUpdateEditZoomin)
    ON_UPDATE_COMMAND_UI(ID_EDIT_ZOOMOUT, &CVideoEditingView::OnUpdateEditZoomout)
    ON_COMMAND(ID_RUN_MATTING, &CVideoEditingView::OnRunMatting)
    ON_UPDATE_COMMAND_UI(ID_RUN_MATTING, &CVideoEditingView::OnUpdateRunMatting)
    ON_WM_KEYDOWN()
    ON_COMMAND(ID_SETTING_LOCALWIN, &CVideoEditingView::OnSettingLocalwin)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
    ON_COMMAND(ID_VIEW_TRIMAP, &CVideoEditingView::OnViewTrimap)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TRIMAP, &CVideoEditingView::OnUpdateViewTrimap)
    ON_COMMAND(ID_RUN_GETFINALLAYER, &CVideoEditingView::OnRunGetfinallayer)
    ON_COMMAND(ID_VIEW_DEPTH, &CVideoEditingView::OnViewDepth)
    ON_UPDATE_COMMAND_UI(ID_VIEW_DEPTH, &CVideoEditingView::OnUpdateViewDepth)
    ON_COMMAND(ID_FILE_OPENPROJECT, &CVideoEditingView::OnFileOpenproject)
    ON_COMMAND(ID_FILE_SAVE, &CVideoEditingView::OnFileSaveProject)
    ON_UPDATE_COMMAND_UI(ID_RUN_GETFINALLAYER, &CVideoEditingView::OnUpdateRunGetfinallayer)
    ON_COMMAND(ID_RUN_REFINE, &CVideoEditingView::OnRunRefine)
	//trimap view unknown pen
	ON_COMMAND(ID_PEN_UNKNOWN,&CVideoEditingView::OnPenUnknown   )  
	ON_UPDATE_COMMAND_UI(ID_PEN_UNKNOWN,&CVideoEditingView::OnUpdatePenUnknown)
	ON_COMMAND(ID_PREPROCESS_OPTICALFLOW, &CVideoEditingView::OnPreprocessOpticalflow)
	ON_COMMAND(ID_HELP_TESTFILLHOLES, &CVideoEditingView::OnHelpTestfillholes)
	ON_COMMAND(ID_PREPROCESS_OPTICALFLOW, &CVideoEditingView::OnPreprocessOpticalflow)
	ON_COMMAND(ID_HELP_TESTFILLHOLES, &CVideoEditingView::OnHelpTestfillholes)
	ON_COMMAND(ID_HELP_DEPTHCOMPOSITION, &CVideoEditingView::OnHelpDepthcomposition)
	ON_COMMAND(ID_RUN_TEMPORALMATTING, &CVideoEditingView::OnRunTemporalmatting)
	ON_COMMAND(ID_POSTPROCESS_LAYEROPTICALFLOW, &CVideoEditingView::OnPostprocessLayeropticalflow)
	ON_COMMAND(ID_BUTTON_TRIMAP_REFINE, &CVideoEditingView::OnRefineTrimap)
	ON_COMMAND(ID_RUN_GENERATEDEFLAYERDEPTH, &CVideoEditingView::OnRunDefaultLayerDepth)
	ON_COMMAND(ID_RUN_GENERATEUNKNOWNOPTICALFLOW, &CVideoEditingView::OnRunUnknownOpticalFlow)
	
	ON_COMMAND(ID_RUN_ROTOSCOPING, &CVideoEditingView::OnRunRotoscoping)
END_MESSAGE_MAP()

// CVideoEditingView construction/destruction
bool firstFrameOK=false;
bool processing=false;
bool rectangled=false;

CVideoEditingView::CVideoEditingView()
{
     InitMember();
     AllocConsole();
     freopen("CONOUT$", "w+t", stdout);
     std::cout<<"init OK"<<std::endl;
}

CVideoEditingView::~CVideoEditingView()
{
}
void CVideoEditingView::InitMember()
{
	fgBrushDown=false;
	bgBrushDown=false;
	fileRange=0;
	startIndex=0;
	rButtonDown=false;
	lButtonDown=false;
	trimapWidth=2;
	current_layer=-1;
	current_layerName="";
	backGroundLayerPath="";
	editLayerPath="";
	backGroundLayerSelected=false;
	editLayerSelected=false;
	firstLayerCreated=false;
	layerGenerated=false;
	clickToStop=false;
	allLayerGenerated=false;
	calDepthStarted=false;
	b_IsRotoScopingReadData = false;
	b_IsClosed = false;
	rotoScopeInterface = NULL;

	m_fDspMin = 1e-6;
	m_fDspMax = 0.01;

	resultPath="";
	alphaPath="";
	trimapPath="";
	maskImagePath = "";
	imageExt="jpg";
	alphaExt="png";
	trimapExt="png";
	maskImageExt = "png";

	resImageBuffer.SetName(resultPath, imageExt);
	resImageBuffer.ClearBuffer();
	alphaBuffer.SetName(alphaPath, alphaExt);
	alphaBuffer.ClearBuffer();
	trimapBuffer.SetName(trimapPath,trimapExt);
	trimapBuffer.ClearBuffer();
	maskImageBuffer.SetName(maskImagePath, maskImageExt);
	maskImageBuffer.ClearBuffer();

	m_fTriMpAlpha = 0.1;
	m_TriMpVwMode = TRIMAP;

	mode=EDITVIEW;
	editTool=NOTOOL;
	hasMoved=false;
	hasStartPos=false;
	processing=false;
	bgColor=RGB(128, 128, 128);

	  Initial();

	  imageEditView=new ImageEditView(this);
	  imageEditView->SetImageBuffer(&imageBuffer);
	  imageEditView->Init();

	  imageResultView=new ImageEditView(this);
	  imageResultView->SetImageBuffer(&resImageBuffer);
	  imageResultView->Init();

	  gridLayout.SetGridSize(1, 2);
	  gridLayout.AddObject(imageEditView);
	  gridLayout.AddObject(imageResultView);

	  trimapView=new ImageEditView(this);
	  trimapView->SetImageBuffer(&trimapBuffer);
	  trimapView->Init();
	  gridLayoutTrimap.SetGridSize(1,1);
	  gridLayoutTrimap.AddObject(trimapView);
	  layout=&gridLayout;
}

void CVideoEditingView::InitRegion()
{
	RECT viewRect;
	this->GetClientRect(&viewRect);
	RECT rectTemp;
	rectTemp.left=viewRect.left;
	rectTemp.right=viewRect.left+(viewRect.right-viewRect.left)/2;
	rectTemp.top=viewRect.top;
	rectTemp.bottom=viewRect.bottom;

	imageEditView->SetBound(rectTemp);

	rectTemp.left=rectTemp.right;
	rectTemp.right=viewRect.right;
	rectTemp.top=viewRect.top;
	rectTemp.bottom=viewRect.bottom;

	imageResultView->SetBound(rectTemp);
}

BOOL CVideoEditingView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CView::PreCreateWindow(cs);
}

// CVideoEditingView drawing

void CVideoEditingView::OnDraw(CDC* pDC)
{
	CMemDC pMemDC(pDC);
	CVideoEditingDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	// Maybe extend it
     switch(mode)
     {
	 case EDITVIEW:
		 EditView(pMemDC);
		 break;
	 case RESVIEW:
		 EditView(pMemDC);
		 break;
	 case PICVIEW:
		 EditView(pMemDC);
		 break;
	 case DEPTHVIEW:
		 EditView(pMemDC);
		 break;
	 case TRIMAPVIEW:
		 EditView(pMemDC);
		 break;
     }
     if(processing)
          mutex.SetEvent();
	
}

// CVideoEditingView printing

BOOL CVideoEditingView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CVideoEditingView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CVideoEditingView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CVideoEditingView diagnostics

#ifdef _DEBUG
void CVideoEditingView::AssertValid() const
{
	CView::AssertValid();
}

void CVideoEditingView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CVideoEditingDoc* CVideoEditingView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CVideoEditingDoc)));
	return (CVideoEditingDoc*)m_pDocument;
}
#endif //_DEBUG


// CVideoEditingView message handlers
LRESULT CVideoEditingView::OnRender(WPARAM wParam, LPARAM lParam)
{
    CString dir;
    cout<<"in render"<<endl;
	
	CxImage result;
	CxImage trimap;
    switch(mode)
	{
	case EDITVIEW:
		imageBuffer.Goto((int)wParam+startIndex);
		resImageBuffer.Goto((int)wParam+startIndex);
		alphaBuffer.Goto((int)wParam+startIndex);
		trimapBuffer.Goto((int)wParam+startIndex);//还是要修改帧序号的
		//因为trimap只保存了0,128，和255的结果，没有保存结果图，所以每次切换的时候要生成结果图放入trimapbuffer中
		//trimap.Copy( *trimapBuffer.GetImage());
		////利用trimap和原始图像生成显示的结果图，
		//result.Copy(*imageBuffer.GetImage()); 
		//Composite(result, result, trimap);
		//trimapBuffer.OpenImage(result);
		layerBuffer.Goto((int)wParam+startIndex);
		GetDocument()->RestoreFrame(imageBuffer.GetImage(), alphaBuffer.GetImage());

		cout << "Frame pos: " << (int)wParam+startIndex << endl;
		AfxGetMainWnd()->SetWindowText(imageBuffer.GetFrameName());


		hasMoved=true;
		break;
	case RESVIEW:
		imageBuffer.Goto((int)wParam+startIndex);
		resImageBuffer.Goto((int)wParam+startIndex);
		alphaBuffer.Goto((int)wParam+startIndex);
		trimapBuffer.Goto((int)wParam+startIndex);
		layerBuffer.Goto((int)wParam+startIndex);

		AfxGetMainWnd()->SetWindowText(imageBuffer.GetFrameName());
		break;
	case PICVIEW:
		break;
	case DEPTHVIEW:
		printf("startindex:%d\n",startIndex);
		imageBuffer.Goto((int)wParam+startIndex);
		resImageBuffer.Goto((int)wParam+startIndex);
		alphaBuffer.Goto((int)wParam+startIndex);
		trimapBuffer.Goto((int)wParam+startIndex);
		layerBuffer.Goto((int)wParam+startIndex);
		printf("before restore\n");
		GetDocument()->RestoreFrame(imageBuffer.GetImage(), alphaBuffer.GetImage());
		cout << "Frame pos: " << (int)wParam+startIndex << endl;
		AfxGetMainWnd()->SetWindowText(imageBuffer.GetFrameName());
		hasMoved=true;
		break;
	case TRIMAPVIEW:
		imageBuffer.Goto((int)wParam+startIndex);
		resImageBuffer.Goto((int)wParam+startIndex);
		alphaBuffer.Goto((int)wParam+startIndex);
		trimapBuffer.Goto((int)wParam+startIndex);//还是要修改帧序号的
		//if(trimapBuffer.flag)
		switch(m_TriMpVwMode)
		{
		case TRIMAP:
			//因为trimap只保存了0,128，和255的结果，没有保存结果图，所以每次切换的时候要生成结果图放入trimapbuffer中
			//if ((trimapBuffer.Goto((int)wParam+startIndex))==false)
			//{
				// trimapBuffer.SetPos((int)wParam+startIndex);
				// trimap.Create(imageBuffer.GetImage()->GetWidth(), imageBuffer.GetImage()->GetHeight(),24);
				// trimap.Clear(0);
			//}
			//else
			trimap.Copy(*trimapBuffer.GetImage());
			////利用trimap和原始图像生成显示的结果图，
			result.Copy(*imageBuffer.GetImage()); 
			Composite(result, result, trimap, m_fTriMpAlpha);
			trimapBuffer.OpenImage(result);
			break;
		case RESULT:
			trimapBuffer.OpenImage(*resImageBuffer.GetImage());
			break;
		//case SOURCE:
		//	trimapBuffer.OpenImage(*imageBuffer.GetImage());
		//	break;
		default:
			break;
		}
		layerBuffer.Goto((int)wParam+startIndex);
		GetDocument()->RestoreFrame(imageBuffer.GetImage(), alphaBuffer.GetImage());
		cout << "Frame pos: " << (int)wParam+startIndex << endl;
		AfxGetMainWnd()->SetWindowText(imageBuffer.GetFrameName());
		hasMoved=true;
		break;
	default:
		break;
    }
    Invalidate();
    return 0;
}

void CVideoEditingView::Idle()
{
     switch(mode)
     {
          case EDITVIEW:
               //Update scribe image and alpha image
               if(hasMoved)                    
               {
                    //GetDocument()->UpdateImages(imageBuffer.GetImage());
                    hasMoved=false;
               }
               break;
          case RESVIEW:
               break;
          case PICVIEW:
               break;
     }
}

void CVideoEditingView::EditView(CMemDC* pDC)
{
	CRect rect;
	CDC DC;
	CBitmap bitmap;
	
    // Test
	RECT viewRect;
	this->GetClientRect(&viewRect);
	trimapView->SetBound(viewRect);
	RECT rectTemp;
	rectTemp.left=viewRect.left;
	rectTemp.right=viewRect.left+(viewRect.right-viewRect.left)/2;
	rectTemp.top=viewRect.top;
	rectTemp.bottom=viewRect.bottom;
	imageEditView->SetBound(rectTemp);

	rectTemp.left=rectTemp.right;
	rectTemp.right=viewRect.right;
	rectTemp.top=viewRect.top;
	rectTemp.bottom=viewRect.bottom;
	imageResultView->SetBound(rectTemp);


	GetClientRect(&rect);
	CMemDC pMemDC(&DC);
	pMemDC.CreateCompatibleDC(pDC);
	bitmap.CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());
	
	pMemDC.SelectObject(&bitmap);
	
	pMemDC.FillSolidRect(rect.left, rect.top, rect.Width(), rect.Height(), bgColor);

	layout->SetInfo(rect);
	layout->Draw(pMemDC.GetSafeHdc());

	pDC->BitBlt(rect.left, rect.top, 
		        rect.Width(), rect.Height(), 
		        &pMemDC, rect.left, rect.top, SRCCOPY);
	pMemDC.DeleteDC();
	bitmap.DeleteObject();

}

void CVideoEditingView::Initial()
{
     level=1;
     alevel=1;
     erodWinSize=1;
     alphath=0.02;
     ep=0.0000001;
}

// Message handlers
void CVideoEditingView::OnFileOpen()
{
     InitRegion();
     if(mode==RESVIEW)
          return;
     else
     if(mode==PICVIEW)
     {
          LPCTSTR lpszFilter="Image Files (*.jpg,*.bmp,*.png)|*.jpg;*.bmp;*.png|JPG Files (*.jpg)|*.jpg|BMP Files (*.bmp)|*.bmp|PNG Files (*.png)|*.png||";//_T("JPG (*.jpg)|*.jpg|BMP (*.bmp)|*.bmp|PNG (*.png)|*.png|");
          CFileDialog Dlg(TRUE, NULL, NULL, 
                          OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
                          lpszFilter, NULL);
          if(Dlg.DoModal()==IDOK)
          {
               if(imageBuffer.OpenImage((LPCTSTR)Dlg.GetPathName()))
               {
                    GetDocument()->New();
                    GetDocument()->Opened=true;
                    GetDocument()->UpdateImages(imageBuffer.GetImage());
                    Invalidate();
               }
			   imageOpened=true;
          }
     }
	 else
	 {
		 LPCTSTR lpszFilter="Image Files (*.jpg,*.bmp,*.png)|*.jpg;*.bmp;*.png|JPG Files (*.jpg)|*.jpg|BMP Files (*.bmp)|*.bmp|PNG Files (*.png)|*.png||";//_T("JPG (*.jpg)|*.jpg|BMP (*.bmp)|*.bmp|PNG (*.png)|*.png|");
		 CFileDialog Dlg(TRUE, NULL, NULL, 
			 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
			 lpszFilter, NULL);
		 if(Dlg.DoModal()==IDOK)
		 {
			 CString fname;
			 CString pathname;
			 CString ext;
			 fname=Dlg.GetFileName();
			 pathname=Dlg.GetPathName();
			 imagePath=pathname;
			 int index=pathname.ReverseFind('\\');
			 pathname.Delete(index+1,pathname.GetLength()+1);//left only the path D:/data/
			 this->filePath=pathname;
			 editLayerPath=pathname;
			 editLayerPath.AppendFormat("%s","VEData");
			 DeleteFolder(editLayerPath);
			 CreateDirectory(editLayerPath,0);
			editLayerPath.AppendFormat("%s","\\");


			 index=fname.Find('.');
			 ext=fname.Mid(index+1,3);//file ext name
			 CString temp;
			 temp=fname.Mid(0,fname.GetLength()-4);
			 int i;
			 for (i=temp.GetLength()-1;i>=0;--i)
			 {
				 if (!isdigit(temp[i]))
				 {
					 break;
				 }
			 }
			 fname=temp.Mid(0,i+1);//文件名的非数字部分
			 filename=fname;
			 temp=temp.Right(temp.GetLength()-i-1);//文件名的数字部分
			 pathname.AppendFormat("%s",fname);//D:/data/image
			 namelength=temp.GetLength();

			 backGroundLayerPath=pathname;
			 
			 imageExt = ext;
			 imageBuffer.SetName(pathname,imageExt);
			 layerBuffer.SetName(pathname,ext);

			 startIndex=_ttoi(temp);
			 int tempindex=startIndex;
			 fileRange=0;
			 CString tempnum;
			 tempnum.Format("%d",namelength);
			 while(true)
			 {
				
				 CString temppath;
				 temppath.Format("%s%."+tempnum+"d%s",pathname,tempindex,"."+ext);
				 if (access(temppath.GetBuffer(),0)==-1)
				 {
					 break;
				 }
				 fileRange++;
				 tempindex++;
			 }
			 ((CMainFrame *)(AfxGetApp()->m_pMainWnd))->setRange(fileRange);

			 layerBuffer.OpenVideo(_ttoi(temp));
			 if(imageBuffer.OpenVideo(_ttoi(temp)))
			 {
				 hasMoved=true;
				 GetDocument()->New();
				 GetDocument()->Opened=true;
				 Invalidate();
			 }
			 imageOpened=true;
			 //multilayer
			 vec_layer.clear();
			 current_layer=-1;
			 current_layerName="";
			 allLayerGenerated=false;
		 }		
	 }
	 AfxGetMainWnd()->SetWindowText(imageBuffer.GetFrameName());
	 resImageBuffer.ClearBuffer();
	 alphaBuffer.ClearBuffer();
	 trimapBuffer.ClearBuffer();

	 GenerateVEDepthMaps();
}

BOOL CVideoEditingView::OnEraseBkgnd(CDC* pDC)
{
	//printf("erase!\n");
     if(!GetDocument()->Opened)
     {
          CRect rect;
          CBrush backBrush(bgColor);
          CBrush *pOldBrush=pDC->SelectObject(&backBrush);

          GetClientRect(&rect);
          pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
          pDC->SelectObject(pOldBrush);
     }
     return true;
}


void CVideoEditingView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (IsToolManager)
	{
		CView::OnLButtonDown(nFlags, point);
		return;
	}
    lButtonDown=true;
    if( ( editTool==GRAB || editTool==GRABUPDATE ) )
    {
		if (editTool==GRAB)
		{
			imageEditView->SetRectGrab();
		}
        imageEditView->OnLButtonDown(point);
        imageResultView->OnLButtonDown(point);
    }
    else
    if(editTool==DRAW && !IsToolManager)
    {
        if(layout!=&gridLayoutTrimap)
        {
            imageEditView->OnLButtonDown(point);
            imageResultView->OnLButtonDown(point);
        }
        else
        {
			//cout << "Trimap button down.posi : "<<point.x<<" "<<point.y << endl;
			if (trimapBuffer.OpenVideo(imageBuffer.GetFramePos())==false)//trimap 不存在
			{
				CxImage temp_trimap;
				temp_trimap.Create(this->imageBuffer.GetImage()->GetWidth(),this->imageBuffer.GetImage()->GetHeight(),24);
				temp_trimap.Clear(0);
				GetDocument()->SetTrimap(temp_trimap);
			}
			else
				GetDocument()->SetTrimap(*trimapBuffer.GetImage());
			trimapView->OnLButtonDown(point);
			CxImage trimap;
			GetDocument()->GetTrimap(trimap);
        }
    }
    else if (calDepthStarted )
    {
		CRect SrcImgRect = imageEditView->GetRegion();
		CRect ResImgRect = imageResultView->GetRegion();

		if(SrcImgRect.PtInRect(point))
		{
			int x=point.x;
			int y=point.y;
			GetDocument()->AdjustPos(x, y, SrcImgRect);

			static int count_point(0);
			plane_point[count_point].x=x;
			plane_point[count_point].y=y;
			plane_point[count_point].z=z_depth;
			count_point++;

			if(count_point == 3)
			{
				count_point = 0;

				CString str("Points coordinate are:\n");
				for (int i=0;i<=2;++i)
				{
					str.AppendFormat("%d %d %d\n",plane_point[i].x,plane_point[i].y,plane_point[i].z);
				}
				str.AppendFormat("Press OK to Add Key Frame ,or press Cancel");
				if(::MessageBox(NULL, str, "Add Key Frame", MB_OKCANCEL)!=IDOK)//
					return;
				else//start to calculate depth 
				{
					calDepthStarted=false;
					CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
					pMain->setCheckboxStatus(calDepthStarted);
					//add keyFrame
					AddKeyFrame(imageBuffer.GetFramePos(),plane_point[0],plane_point[1],plane_point[2]);
					//calculate depth;
					if (vec_layer[current_layer].NumOfKeyFrame()==1);//only current keyFrame
					//   CalculateDepthMap();
					else//exists other keyFrames
					{
						int lastKeyFrame,nextKeyFrame;
						int lastKeyFrameIndex,nextKeyFrameIndex;
						vec_layer[current_layer].GetNeighbourOfKeyFrame(imageBuffer.GetFramePos(),lastKeyFrame,lastKeyFrameIndex,nextKeyFrame,nextKeyFrameIndex);

						//process sequence range from lastKeyFrame to currentKeyFrame
						int currentPos=imageBuffer.GetFramePos();
						if (lastKeyFrameIndex!=-1)
						{
							CalculateDepthMap(lastKeyFrame,lastKeyFrameIndex,currentPos,vec_layer[current_layer].GetKeyFrameIndex(currentPos));
						}
						if (nextKeyFrameIndex!=-1)
						{
							CalculateDepthMap(currentPos,vec_layer[current_layer].GetKeyFrameIndex(currentPos),nextKeyFrame,nextKeyFrameIndex);
						}
						//process sequence range from currentKeyFrame to nextKeyFrame 
					}
				}
			}
	    }
		else if(ResImgRect.PtInRect(point))
		{
			int x = point.x;
			int y = point.y;
			GetDocument()->AdjustPos(x, y, ResImgRect);

			CxImage* pSrcImg = imageBuffer.GetImage();
			int iWidth = pSrcImg->GetWidth();
			int iHeight = pSrcImg->GetHeight();

			if(x >= 0 && x < iWidth && y >= 0 && y < iHeight)
			{
				CString DepthImgName;
				DepthImgName = filePath;
				DepthImgName.AppendFormat("%s", "VEDepthMap\\");
				DepthImgName.AppendFormat("%s", filename);
				ImageBuffer DepthImgBuf;
				DepthImgBuf.SetName(DepthImgName, trimapExt);
				CxImage DepthImg;
				DepthImg.Load((LPCTSTR)DepthImgBuf.GetFrameName(resImageBuffer.GetFramePos()));

				z_depth = DepthImg.GetPixelGray(x, y);
				CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
				CCoolTabCtrl::CPageItem *pPageItem = (CCoolTabCtrl::CPageItem*)pMainFrm->m_wndTabCont.GetPageItem(1);
				CMultiLayerDlg *pMulLyrDlg = (CMultiLayerDlg*)pPageItem->m_pWnd;
				pMulLyrDlg->depthCtrl.SetPos(z_depth);

				CString DepthText;
				DepthText.Format("%d", z_depth);
				SetDlgItemText(IDC_EDIT_DEPTH, DepthText);
				pMulLyrDlg->Invalidate(FALSE);
			}
		}
    }
    CView::OnLButtonDown(nFlags, point);
}
void CVideoEditingView::OnMouseMove(UINT nFlags, CPoint point)
{
     if((editTool==GRAB || editTool==GRABUPDATE) && lButtonDown &&!IsToolManager)         //grab user iteration
	 {
		 imageEditView->OnMouseMove(point);
		 imageResultView->OnMouseMove(point);
	 }
     else
     if(rButtonDown && hasStartPos)
     {
          imageEditView->MoveTo(point.x, point.y);
          imageResultView->MoveTo(point.x, point.y);
		  trimapView->MoveTo(point.x,point.y);
          Invalidate();
     }
     else
     if(editTool==DRAW && lButtonDown &&!IsToolManager)                                 //pen
	 {
         if(layout!=&gridLayoutTrimap)
         {
             imageEditView->OnMouseMove(point);
             imageResultView->OnMouseMove(point);
         }
         else
		 {
			 //cout << "Trimap move." << endl;
			 //调用该函数时会将对应的doc里面的所有文件都作相应的修改， 修改时根据当前笔刷的状态进行
			 trimapView->OnMouseMove(point);

			 // 					//修改trimapview的时候同时修改抠图时的前背景信息
			 // 					//因为editview是左右两边的，所以进行坐标变换，将trimap中的坐标变换到editview中左边
			 // 					//printf("SIZE :: %d %d \n",trimapBuffer.GetImage()->GetWidth(),trimapBuffer.GetImage()->GetHeight());
			 // 					//获得鼠标点在真实图片中的坐标
			 // 					int pointToImageX=point.x,pointToImageY=point.y;
			 // 					float z;
			 // 					z=(float)trimapBuffer.GetImage()->GetHeight()/(trimapView->GetRegion().bottom-trimapView->GetRegion().top ) ;
			 // 					pointToImageX=(int)((pointToImageX-trimapView->GetRegion().left)*z);
			 // 					pointToImageY=(int)((pointToImageY-trimapView->GetRegion().top)*z);
			 // 					//根据图像坐标计算相对于editview中的左上角的对应点的坐标
			 // 					RECT  regionForEditView=imageEditView->GetRegion();
			 // 					int imageWidth=trimapBuffer.GetImage()->GetWidth(),imageHeight=trimapBuffer.GetImage()->GetHeight();
			 // 					int xInEditViewImage=pointToImageX*(regionForEditView.right-regionForEditView.left)/imageWidth;
			 // 					int yInEditViewImage=pointToImageY*(regionForEditView.bottom-regionForEditView.top)/imageHeight;
			 // 					//计算在对应的editview中的坐标
			 // 					xInEditViewImage+=regionForEditView.left;
			 // 					yInEditViewImage+=regionForEditView.top;
			 // 
			 // 					CPoint correctedPoint;
			 // 					correctedPoint.x=xInEditViewImage;correctedPoint.y=yInEditViewImage;
			 // 					imageEditView->OnMouseMove(correctedPoint);
			 // 					imageResultView->OnMouseMove(correctedPoint);
         }
	 }
     //Status bar shows the position
     CString str;
     CStatusBar *statusBar;
     statusBar=(CStatusBar *)AfxGetApp()->m_pMainWnd->GetDescendantWindow(AFX_IDW_STATUS_BAR);
     str.Format("x=%3d,y=%3d", point.x, point.y);
     if(statusBar)
          statusBar->SetPaneText(0, str);
     CView::OnMouseMove(nFlags, point);
}

void CVideoEditingView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (IsToolManager||!lButtonDown)
	{
		CView::OnLButtonUp(nFlags,point);
		return;
	}
     if(editTool==GRAB)
	 {
		 imageEditView->OnLButtonUp(point);
		 imageResultView->OnLButtonUp(point);
		 //add layer
		 if (firstLayerCreated)
		 {
			 imageBuffer.SetName(backGroundLayerPath, imageExt);
			 imageBuffer.Goto(imageBuffer.GetFramePos());
			 layerBuffer.Goto(imageBuffer.GetFramePos());
		 }
		CLayerEditDlg layerEditDlg;
		int nresponse=layerEditDlg.DoModal();
		if (nresponse==IDOK&&layerEditDlg.GetLayerType()!=-1)
		{
			bool re=AddLayer(layerEditDlg.GetLayerName(),layerEditDlg.GetLayerType());
			if(re)
			{
				OnRunRun();
				editTool=NOTOOL;
			}
			SetLayerPos(imageBuffer.GetFramePos());
		}
		else
		{
			 imageEditView->Clear();
			Invalidate();
		}
	 }
	 else 
     if(editTool==GRABUPDATE)
	 {
		 imageEditView->OnLButtonUp(point);
		 imageResultView->OnLButtonUp(point);
		 CxImage result;
		 CxImage alpha;
		 CxImage trimap;
		 CxImage layer;

		 processing=false;
		 firstFrameOK=true;
		 imageEditView->Clear();
		 imageResultView->Clear();
		 GetDocument()->Run(result, alpha);
		 if(mode==PICVIEW)
		 {
			 resImageBuffer.OpenImage(result);
			 Invalidate();
		 }
		 else
		 if(mode==EDITVIEW)
		 {
			 resImageBuffer.OpenImage(result);
			 Invalidate();
			 result.Save((LPCTSTR)resImageBuffer.GetFrameName(), CXIMAGE_FORMAT_JPG);
			 alpha.Save((LPCTSTR)alphaBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);
		 }
		 if (fgBrushDown)
		 {
			imageEditView->SetFgBrush();
			imageResultView->SetFgBrush();
		 }
		 else 
         if (bgBrushDown)
		 {
			 imageEditView->SetBgBrush();
			 imageResultView->SetBgBrush();
		 }
	 }
     else if(editTool==DRAW)
     {
         CxImage result;
         CxImage alpha;
	     CxImage trimap;
   	     CxImage layer;
          if(layout!=&gridLayoutTrimap)
          {
			  imageEditView->OnLButtonUp(point);
			  imageResultView->OnLButtonUp(point);
			  GetDocument()->GetResult(result, alpha);
			  resImageBuffer.OpenImage(result);
			  result.Save((LPCTSTR)resImageBuffer.GetFrameName(), CXIMAGE_FORMAT_JPG);
			  alpha.Save((LPCTSTR)alphaBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);
			  GetDocument()->ResetScribble();
			  GetDocument()->GetTrimap(trimap);
			  trimap.Save((LPCTSTR)trimapBuffer.GetFrameName(),CXIMAGE_FORMAT_PNG);
			  result.Copy(*imageBuffer.GetImage());
			  Composite(result, result, trimap, m_fTriMpAlpha);
			  trimapBuffer.OpenImage(result);
          }
          else
		  {
			  //回复设置的状态
			  {
			  trimapView->OnLButtonUp(point);
			  //取得trimap图像
			  GetDocument()->GetTrimap(trimap);
			  //生成结果图并保存，因为trimap view中会修改前背景，所以会对editview中的结果进行修改，保存后切换视图时才会
			  //保证结果正确
			  /*GetDocument()->GetResult(result, alpha);
			  resImageBuffer.OpenImage(result);*/
			 // result.Save((LPCTSTR)resImageBuffer.GetFrameName(), CXIMAGE_FORMAT_JPG);
			  //利用trimap和原始图像生成显示的结果图，
			  trimap.Save((LPCTSTR)trimapBuffer.GetFrameName(),CXIMAGE_FORMAT_PNG);
			  result.Copy(*imageBuffer.GetImage());
			  Composite(result, result, trimap, m_fTriMpAlpha);
			  trimapBuffer.OpenImage(result);
			  }
          }
          Invalidate();
     }
	  lButtonDown=false;
     CView::OnLButtonUp(nFlags, point);
}

BOOL CVideoEditingView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (!processing&&firstFrameOK)
	{
		CRect rect;
		//editTool=MOVE;
		rect.left=rect.right=pt.x;
		rect.top=rect.bottom=pt.y;
		ScreenToClient(&rect);
		layout->Zoom(zDelta, 1.0f, rect.left, rect.top);
		Invalidate();
		return CView::OnMouseWheel(nFlags, zDelta, pt);
	}  
}
void CVideoEditingView::OnViewEditwindow()
{
     mode=EDITVIEW;
	//如果是unknown 笔刷，则只能在trimapview中使用，切换视图时使其失效
	if (layout == & gridLayoutTrimap && imageEditView->DrawingType()==2)
	{
		editTool=NOTOOL;
	}
     layout=&gridLayout;
     if (firstLayerCreated)
     {
	     CString str;
	     str=filePath;
	     str.AppendFormat("%s%s",vec_layer[current_layer].GetName(),"\\VEResult\\");
	     str.AppendFormat("%s",filename);
	     resImageBuffer.SetName(str,imageExt);
	     resImageBuffer.Goto(resImageBuffer.GetFramePos());
		// this->imageResultView->SetImageBuffer(&alphaBuffer);
     }
          Invalidate();
}

void CVideoEditingView::OnViewResultwindow()
{
     mode=RESVIEW;
	//如果是unknown 笔刷，则只能在trimapview中使用，切换视图时使其失效
	if (layout == & gridLayoutTrimap && imageEditView->DrawingType()==2)
	{
		editTool=NOTOOL;
	}
     resImageBuffer.SetName(resultPath, imageExt);
     resImageBuffer.OpenVideo();
	// this->imageResultView->SetImageBuffer(&resImageBuffer);
     Invalidate();
}

void CVideoEditingView::OnUpdateViewEditwindow(CCmdUI *pCmdUI)
{
     pCmdUI->SetCheck(mode==EDITVIEW && layout==&gridLayout);
}

void CVideoEditingView::OnUpdateViewResultwindow(CCmdUI *pCmdUI)
{
     pCmdUI->SetCheck(mode==RESVIEW);
}



UINT RunVideo(LPVOID param);

void CVideoEditingView::OnSettingPreference()
{
	CParaDlg  paraDlg(NULL);
	GetDocument()->GetCFParameter(paraDlg.level,paraDlg.activeLevel,
		paraDlg.winSize,paraDlg.thrAlpha,paraDlg.epsilon);
	UpdateData(false);
	INT_PTR nRet=-1;
	nRet=paraDlg.DoModal();
	UpdateData(TRUE);

	switch (nRet)
	{
	     case -1:
		     AfxMessageBox("Parameter box could not be created.");
		     break;
	     case IDOK:
		     level=paraDlg.level;
		     alevel=paraDlg.activeLevel;
		     ep=paraDlg.epsilon;
		     alphath=paraDlg.thrAlpha;
		     erodWinSize=paraDlg.winSize;
		     GetDocument()->SetParameter(level,alevel,erodWinSize,alphath,ep);
		     break;
	}
}


/************************************************************************/
/* Thread                                                               */
/************************************************************************/
#include <iostream>

UINT RunVideo(LPVOID param)
{

	cout<<"nimaya"<<endl;
	int exit;
	CxImage result;
	CxImage alpha;
	CxImage trimap;
	CxImage layer;
	CVideoEditingView *editingView;

	editingView=(CVideoEditingView *)param;
	editingView->resImageBuffer.SetName(editingView->resultPath, editingView->imageExt);
	editingView->alphaBuffer.SetName(editingView->alphaPath, editingView->alphaExt);
	editingView->trimapBuffer.SetName(editingView->trimapPath,editingView->trimapExt);
	editingView->maskImageBuffer.SetName(editingView->maskImagePath, editingView->maskImageExt);


	CString title;
	      // int n=15;
	 while(/*BlockInput(true) && */editingView->imageBuffer.Forward()/*&&n!=0*/)
	 { 
		 editingView->SetLayerPos(editingView->imageBuffer.GetFramePos());
		 editingView->layerBuffer.Goto(editingView->imageBuffer.GetFramePos());
		 title=editingView->imageBuffer.GetFrameName();
		 std::cout<<"Frame Name: "<<title<<std::endl;
		 ((AfxGetApp()->m_pMainWnd))->PostMessage(WM_SETTEXT,0,(LPARAM)title.GetBuffer());
		 //BlockInput(FALSE);

		 CxImage teimage;
		 editingView->GenerateImage(teimage);
		 //editingView->GetDocument()->UpdateImages(editingView->imageBuffer.GetImage());
		 editingView->GetDocument()->UpdateImages(&teimage);
		 

	///	editingView->GetDocument()->RunVideo(result, alpha);
		// editingView->GetDocument()->UpdateImages(&result);
		
         
		// result.Save("D://result.jpg",CXIMAGE_FORMAT_JPG);
		// alpha.Save("D://alpha.png",CXIMAGE_FORMAT_PNG);
         editingView->GetDocument()->RunVideo(result, alpha, editingView->filePath, editingView->imageBuffer.GetFramePos()-1);
		 editingView->ReUpdateResult(result,alpha);
		 // editingView->GetEditLayer(editingView->layerBuffer.GetImage(),&alpha,layer);
		 // editingView->GetDocument()->GetTrimap(trimap,editingView->trimapWidth);
		 // Show result
		 editingView->resImageBuffer.SetPos(editingView->imageBuffer.GetFramePos());
		 result.Save((LPCTSTR)editingView->resImageBuffer.GetFrameName(), CXIMAGE_FORMAT_JPG);
		 editingView->resImageBuffer.OpenImage(result);
		 editingView->alphaBuffer.SetPos(editingView->imageBuffer.GetFramePos());
		 alpha.Save(editingView->alphaBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG);
		 editingView->alphaBuffer.OpenImage(alpha);
		 // editingView->trimapBuffer.SetPos(editingView->imageBuffer.GetFramePos());
		 //  trimap.Save(editingView->trimapBuffer.GetFrameName(),CXIMAGE_FORMAT_BMP);
		 // editingView->trimapBuffer.OpenImage(trimap);
		 // CString temp;
		 // temp=editingView->GetEditLayerPath();
		 //temp.AppendFormat("%s",editingView->imageBuffer.GetFileName());

		 // layer.Save((LPCTSTR)temp,CXIMAGE_FORMAT_JPG);
		 //保存trimap
		// BlockInput(FALSE);
		 BlockInput(TRUE);
		 editingView->Invalidate();
		 editingView->setProcess();
		 BlockInput(FALSE);

		 WaitForSingleObject(editingView->mutex.m_hObject, INFINITE);

		 exit=::WaitForSingleObject(editingView->exitThread.m_hObject, 0);
		 if(exit==WAIT_OBJECT_0)
		 {
			 editingView->finishExiting.SetEvent();
			 break;
		 }
		 if (!editingView->layerBuffer.Forward())
		 {
			 //editingView->imageBuffer.SetName(editingView->backGroundLayerPath);
			 editingView->layerBuffer.SetName(editingView->backGroundLayerPath, editingView->imageExt);
			 editingView->layerBuffer.Goto(editingView->imageBuffer.GetFramePos());
		 }
		 else
			 editingView->layerBuffer.Back();
		// n--;
     }
	 editingView->GetDocument()->contour.Clear();
	 editingView->GetDocument()->CheckMemory();
     //editOut.AppendString(CString("Exit the computing thread."));
     //std::cout << "Exit the computing thread." << endl;
	 if (logText)
	 {
		 logText<<"Exit the computing thread."<<endl;
	 }
     processing=false;
	 CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	 editingView->SendMessage(WM_COMMAND_RENDER, pMain->GetPos());
	 return 0;
}

void CVideoEditingView::setResultPath(CString rp)
{
	resultPath=rp;
	resultPath.AppendFormat("%s",filename);
	resImageBuffer.SetName(resultPath,imageExt);
}

void CVideoEditingView::setAlphaPath(CString ap)
{
	alphaPath=ap;

	alphaPath.AppendFormat("%s",filename);
	alphaBuffer.SetName(alphaPath,alphaExt);
}

void CVideoEditingView::setTrimapPath(CString tp)
{
	trimapPath=tp;
	trimapPath.AppendFormat("%s",filename);
	trimapBuffer.SetName(trimapPath,trimapExt);
}

void CVideoEditingView::setMaskImagePath( CString tp )
{
	maskImagePath=tp;
	maskImagePath.AppendFormat("%s",filename);
	maskImageBuffer.SetName(maskImagePath,maskImageExt);
}


void CVideoEditingView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	int currentPos=pMain->GetPos();
	switch(nChar)
	{
		//修改笔刷大小
	case 'n':case 'N': //减小笔刷
		//首先判断当前是哪个视图，然后将对应视图的笔刷大小进行修改，因为editview中笔刷的大小要和doc修改时的大小对应，所以这里要保证两者对应

		if(editTool==DRAW)//已经选中笔刷
		{
			if(layout!=&gridLayoutTrimap)//如果是label修改视图
			{
				imageEditView->DecreasepenSizeScale();
				imageResultView->DecreasepenSizeScale();
			}
			else//trimap修改视图
			{
				trimapView->DecreasepenSizeScale();
			}
		}

		break;
	case 'm':case 'M'://增大笔刷
		if(editTool==DRAW)//已经选中笔刷
		{
			if(layout!=&gridLayoutTrimap)//如果是label修改视图
			{
				imageEditView->AddpenSizeScale();
				imageResultView->AddpenSizeScale();
			}
			else//trimap修改视图
			{
				trimapView->AddpenSizeScale();
			}
		}

		break;
	case 'd'://step one
	case 'D':
		if(!processing)
		{
			++currentPos;
			pMain->SendMessage(WM_COMMAND_UPDATE_TIMELINE,currentPos);
			this->SendMessage(WM_COMMAND_RENDER,currentPos);
		}
		break;
	case 'A'://step minus one
	case 'a':
		if(!processing)
		{
			--currentPos;
			if (currentPos>=0)
			{
				pMain->SendMessage(WM_COMMAND_UPDATE_TIMELINE,currentPos);
				this->SendMessage(WM_COMMAND_RENDER,currentPos);
			}

		}
		break;
	case 'Z'://start
	case 'z':
		if(!processing)
		CPlayControl::GetInstance()->Seek(0);
		break;	
	case 'C'://end
	case 'c':
		if(!processing)
		CPlayControl::GetInstance()->Seek(fileRange-1);
		break;

	case 'f':
	case 'F':
		if(imageOpened&&!processing)
		this->OnGrabberForegroundbrush();
		break;

	case 'g':
	case 'G':
		if(imageOpened&&!processing)
			this->OnGrabberBackgroundbrush();
		break;

	case 'u':case 'U'://trimap view 中可以使用unknown pen
		if(imageOpened && ! processing && (layout== & gridLayoutTrimap)){
			this->OnPenUnknown();
		}
		break;

	case 'v':
	case 'V':
		if (imageOpened&&!processing&&firstFrameOK)
		this->OnPenForeground();
		break;

	case 'b':
	case 'B':
		if (imageOpened&&!processing&&firstFrameOK)
			this->OnPenBackground();
		break;

	case 'e'://run
	case 'E':
		if(rectangled&&!processing)
			this->OnRunRun();
		break;

	case 'r'://runvideo
	case 'R':
		if (firstFrameOK&&!processing)
		{
			this->OnRunRunvideo();
		}
		break;

	case 't'://stop
	case 'T':
		if(processing)
			this->OnRunStop();
		break;
	case 's':
	case 'S':
		if(!processing)
			CPlayControl::GetInstance()->Pause();
			break;
	case VK_SPACE: 
		if(mode == TRIMAPVIEW)
		{
			//if(trimapBuffer.flag==false)
			m_TriMpVwMode = (m_TriMpVwMode + 1) % 2;
			switch(m_TriMpVwMode)
			{
			case RESULT:
				if(resImageBuffer.GetImage() && resImageBuffer.GetImage()->IsValid())
					trimapBuffer.OpenImage(*resImageBuffer.GetImage());
				//trimapBuffer.flag = !trimapBuffer.flag;
				break;
			case TRIMAP:
				{
					CxImage result;
					if(imageBuffer.GetImage())
						result.Copy(*imageBuffer.GetImage());
					CxImage trimaptemp;
					trimaptemp.Load(trimapBuffer.GetFrameName());
					if(result.IsValid() && trimaptemp.IsValid())
						Composite(result, result, trimaptemp, m_fTriMpAlpha);
					trimapBuffer.OpenImage(result);
					//trimapBuffer.flag = !trimapBuffer.flag;
				}
				break;
			//case SOURCE:
			//	if(imageBuffer.GetImage() && imageBuffer.GetImage()->IsValid())
			//		trimapBuffer.OpenImage(*imageBuffer.GetImage());
			//	//trimapBuffer.flag = !trimapBuffer.flag;
			//	break;
			default:
				break;
			}
			Invalidate();
		}
		break;
	//case 'i':
	//case 'I':
	//	if(mode == TRIMAPVIEW)
	//	{
	//		if(imageBuffer.GetImage() && imageBuffer.GetImage()->IsValid())
	//			trimapBuffer.OpenImage(*imageBuffer.GetImage());
	//		Invalidate(FALSE);
	//		//////////////////////////////////////////////////////////////////////////
	//		//Uncomment this part, pressing "I/i" will change m_fTriMpAlpha and Slider Control to 0!
	//		//CMainFrame* pMainFrm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	//		//CCoolTabCtrl::CPageItem* pPageItem = (CCoolTabCtrl::CPageItem*)(pMainFrm->m_wndTabCont.GetPageItem(2));
	//		//CClosedFormParam* pDlg = dynamic_cast<CClosedFormParam*>(pPageItem->m_pWnd);
	//		//m_fTriMpAlpha = pDlg->m_fTriMpAlpha = 0;
	//		//pDlg->m_ctrlTriMpAlpha.SetPos(0);
	//		//pDlg->Invalidate(FALSE);
	//	}
	//	break;
	//case 'l':
	//case 'L':
	//	if(mode == TRIMAPVIEW)
	//	{
	//		float fTriMpAlpha = m_fTriMpAlpha;
	//		m_fTriMpAlpha = 1;
	//		RecompositeTrimap();
	//		m_fTriMpAlpha = fTriMpAlpha;
	//		Invalidate(FALSE);
	//		//////////////////////////////////////////////////////////////////////////
	//		//Uncomment this part, pressing "L/l" will change m_fTriMpAlpha and Slider Control to 1!
	//		//CMainFrame* pMainFrm = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	//		//CCoolTabCtrl::CPageItem* pPageItem = (CCoolTabCtrl::CPageItem*)(pMainFrm->m_wndTabCont.GetPageItem(2));
	//		//CClosedFormParam* pDlg = dynamic_cast<CClosedFormParam*>(pPageItem->m_pWnd);
	//		//m_fTriMpAlpha = pDlg->m_fTriMpAlpha = 1;
	//		//pDlg->m_ctrlTriMpAlpha.SetPos(pDlg->m_ctrlTriMpAlpha.GetRangeMax());
	//		//pDlg->Invalidate(FALSE);
	//	}
	//	break;
	default:
		break;
	}
}

void CVideoEditingView::setProcess()
{
	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	pMain->PostMessage(WM_COMMAND_UPDATE_TIMELINE,imageBuffer.GetFramePos()-startIndex);
}

void CVideoEditingView::SetRLTPath(CString path)
{
	CString newresultpath;
	newresultpath=path;
	newresultpath.AppendFormat("%s","VEResult");
	CreateDirectory(newresultpath,0);
	newresultpath.AppendFormat("%s","\\");
	this->setResultPath(newresultpath);

	CString newLabelPath;
	newLabelPath=path;
	newLabelPath.AppendFormat("%s","VELabel");
	CreateDirectory(newLabelPath,0);
	newLabelPath.AppendFormat("%s","\\");
	this->setAlphaPath(newLabelPath);

	
	CString newTrimapPath;
	newTrimapPath=path;
	newTrimapPath.AppendFormat("%s","VETrimap");
	CreateDirectory(newTrimapPath,0);
	newTrimapPath.AppendFormat("%s","\\");
	this->setTrimapPath(newTrimapPath);

	CString newMaskImagePath;
	newMaskImagePath=path;
	newMaskImagePath.AppendFormat("%s","VEMask");
	CreateDirectory(newMaskImagePath,0);
	newMaskImagePath.AppendFormat("%s","\\");
	this->setMaskImagePath(newMaskImagePath);
	
}

void CVideoEditingView::SetRLTPath(CString path,CString layerName)
{
	CString newresultpath;
	newresultpath=path;
	newresultpath.AppendFormat("%s",layerName);
	CreateDirectory(newresultpath,0);

	newresultpath.AppendFormat("%s%s","\\","VEResult");
	CreateDirectory(newresultpath,0);
	newresultpath.AppendFormat("%s","\\");
	
	this->setResultPath(newresultpath);

	CString newlabelpath;
	newlabelpath=path;
	cout<<newlabelpath<<endl;
	newlabelpath.AppendFormat("%s",layerName);
	cout<<newlabelpath<<endl;
	newlabelpath+="\\VELabel";
	cout<<newlabelpath<<endl;
	CreateDirectory(newlabelpath,0);
	newlabelpath.AppendFormat("%s","\\");


	this->setAlphaPath(newlabelpath);
	
	CString newTrimapPath;
	newTrimapPath=path;
	newTrimapPath.AppendFormat("%s",layerName);
	newTrimapPath.AppendFormat("%s%s","\\","VETrimap");
	CreateDirectory(newTrimapPath,0);
	newTrimapPath.AppendFormat("%s","\\");
	this->setTrimapPath(newTrimapPath);

	CString newMaskImagePath;
	newMaskImagePath=path;
	newMaskImagePath.AppendFormat("%s",layerName);
	newMaskImagePath.AppendFormat("%s%s","\\","VEMask");
	CreateDirectory(newMaskImagePath,0);
	newMaskImagePath.AppendFormat("%s","\\");
	this->setMaskImagePath(newMaskImagePath);

	CString fgpath;
	fgpath=path;
	fgpath.AppendFormat("%s", layerName);
	fgpath.AppendFormat("%s%s","\\", "FG");
	CreateDirectory(fgpath,0);

	CString bgpath;
	bgpath = path;
	bgpath.AppendFormat("%s", layerName);
	bgpath.AppendFormat("%s%s", "\\", "BG");
	CreateDirectory(bgpath,0);

	CString optpath;//optical flow
	optpath = path;
	optpath.AppendFormat("%s", layerName);
	optpath.AppendFormat("%s%s", "\\", "opt");
	CreateDirectory(optpath, 0);

	CString depthpath;
	depthpath = path;
	depthpath.AppendFormat("%s", layerName);
	depthpath.AppendFormat("%s%s","\\", "Depth");
	CreateDirectory(depthpath,0);

}

void CVideoEditingView::OnSettingLocalwin()
{
    CLocalParam dialog;
    GetDocument()->GetLocalClassifierParam(dialog.param);
    if(dialog.DoModal()==IDOK)
    {
        cout << "bound size: " << dialog.param.boundDis << endl;
        cout << "fcutoff:    " << dialog.param.fcutoff << endl;
        cout << "epsilon:    " << dialog.param.eMin << endl;
        cout << "fore thres: " << dialog.param.foreThres << endl;
        cout << "back thres: " << dialog.param.backThres << endl;
        GetDocument()->SetLocalClassifierParam(dialog.param);
    }
}

void CVideoEditingView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (!processing)
	{
		rButtonDown=true;
		hasStartPos=true;
		imageEditView->SetMoveStartPos(point.x, point.y);
		imageResultView->SetMoveStartPos(point.x, point.y);
		trimapView->SetMoveStartPos(point.x,point.y);
		CView::OnRButtonDown(nFlags, point);
	}
 
}

void CVideoEditingView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	rButtonDown=false;
	hasStartPos=false;
	CView::OnRButtonUp(nFlags, point);
}

void CVideoEditingView::OnViewTrimap()//切换到trimap视图，需要产生trimap
{
	CTrimapWidthSetDlg trimapDlg( this);

	int response = trimapDlg.DoModal();
	if (response == IDOK)
		trimapWidth = trimapDlg.GetTrimapWidth();
	else
		return;
	mode = TRIMAPVIEW;
	TrimapGenerator trigenerator(filePath.GetBuffer(),vec_layer[current_layer].GetName().GetBuffer(),trimapBuffer.GetExt().GetBuffer(),vec_layer[current_layer].startIndex,trimapWidth);
	trigenerator.GeneratorTrimap(&trimapBuffer, &alphaBuffer);
	//mode=TRIMAPVIEW;//加上这句后选中trimapview不会显示结果，
    RECT viewRect;
    layout=&gridLayoutTrimap;
    this->GetClientRect(&viewRect);
    trimapView->SetBound(viewRect);
	CxImage result;
	result.Copy(*imageBuffer.GetImage());
	CxImage trimaptemp;
	if((trimaptemp.Load(trimapBuffer.GetFrameName()))==false)
	//if(_access(trimapBuffer.GetFrameName().GetBuffer(),0)==-1)
		trimaptemp.Clear(0);

	Composite(result, result, trimaptemp, m_fTriMpAlpha);
	trimapBuffer.OpenImage(result);
	m_TriMpVwMode = TRIMAP;

    Invalidate();
}

void CVideoEditingView::OnUpdateViewTrimap(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(layout==&gridLayoutTrimap);
}

//void CVideoEditingView::Composite(CxImage &dest, CxImage &source, CxImage &alpha)
//{
//    int i,j;
//    int w,h;
//    RGBQUAD rgb,alp;
//    RGBQUAD color;
//
//    color.rgbRed=0;
//    color.rgbGreen=255;
//    color.rgbBlue=0;
//    w=source.GetWidth();
//    h=source.GetHeight();
//	//dest.Copy(alpha);
//    for(i=0;i<h;++i)
//    {
//        for(j=0;j<w;++j)
//        {
//            rgb=source.GetPixelColor(j, i);
//            alp=alpha.GetPixelColor(j, i);
//            if(alp.rgbRed==0 && alp.rgbGreen==0 && alp.rgbBlue==0)
//            {
//				//rgb.rgbBlue  = 64;
//				//rgb.rgbGreen = rgb.rgbRed = 0;
//			 //   dest.BlendPixelColor(j,i,rgb, 0.3);
//            }
//            else
//            if(alp.rgbRed==255 && alp.rgbGreen==255 && alp.rgbBlue==255)
//            {
//				rgb.rgbRed = 255;
//				rgb.rgbGreen = rgb.rgbBlue = 0;
//				dest.BlendPixelColor(j,i,rgb,0.1);
//            }
//            else
//            {
//				rgb.rgbGreen =255;
//				rgb.rgbRed = rgb.rgbBlue = 0;
//				dest.BlendPixelColor(j,i,rgb, 0.1);
//            }
//        }
//    }
//	//static int count=0; 
//	//CString path = "H:/test/trimap_com";
//	//path.AppendFormat("%d.png", count);
//	//count++;
//	//dest.Save(path.GetBuffer(), CXIMAGE_FORMAT_PNG);
//}

void CVideoEditingView::Composite(CxImage &dest, CxImage &source, CxImage &alpha, float blend)
{
    int i,j;
    int w,h;
    RGBQUAD rgb,alp;
    RGBQUAD color;

    color.rgbRed=0;
    color.rgbGreen=255;
    color.rgbBlue=0;
    w=source.GetWidth();
    h=source.GetHeight();
	//dest.Copy(alpha);
    for(i=0;i<h;++i)
    {
        for(j=0;j<w;++j)
        {
            rgb=source.GetPixelColor(j, i);
            alp=alpha.GetPixelColor(j, i);
            if(alp.rgbRed==0 && alp.rgbGreen==0 && alp.rgbBlue==0)
            {
				rgb.rgbBlue  = 0;
				rgb.rgbGreen = rgb.rgbRed = 0;
			    //dest.BlendPixelColor(j,i,rgb,blend);
            }
            else
            if(alp.rgbRed==255 && alp.rgbGreen==255 && alp.rgbBlue==255)
            {
				rgb.rgbRed = 255;
				rgb.rgbGreen = rgb.rgbBlue = 0;
				dest.BlendPixelColor(j,i,rgb,blend);
            }
            else
            {
				rgb.rgbGreen =255;
				rgb.rgbRed = rgb.rgbBlue = 0;
				dest.BlendPixelColor(j,i,rgb,blend);
            }
        }
    }
	//static int count=0; 
	//CString path = "H:/test/trimap_com";
	//path.AppendFormat("%d.png", count);
	//count++;
	//dest.Save(path.GetBuffer(), CXIMAGE_FORMAT_PNG);
}

void CVideoEditingView::calstop()//stop calculation
{
	if(processing)
	{
		imageBuffer.SetName(backGroundLayerPath, imageExt);
		imageBuffer.Goto(imageBuffer.GetFramePos());

		exitThread.SetEvent();
		mutex.SetEvent();
		::WaitForSingleObject(finishExiting.m_hObject, INFINITE);
		processing=false;
		// Also for moving the slide...
		GetDocument()->RestoreFrame(imageBuffer.GetImage(), alphaBuffer.GetImage());
        this->setProcess();
		Invalidate();
	}
}

CString CVideoEditingView::GetName()
{
	CString pathName;
	CString dirName=this->imageBuffer.GetName();
	CString frameIndex;
	int currentFrame=this->imageBuffer.GetFramePos();
	frameIndex.Format("%d",currentFrame);

	if (frameIndex.GetLength()<namelength)
	{
		CString temps="%.";
		temps.AppendFormat("%d%s",namelength,"d");
		frameIndex.Empty();
		frameIndex.Format(temps,currentFrame);
		
	}
	pathName.Format("%s%s%s.%s",dirName,frameIndex,current_layerName,imageBuffer.GetExt());
	return pathName;
}
BOOL CVideoEditingView::DeleteFolder(LPCTSTR lpszPath)
{
        //SHFILEOPSTRUCT FileOp;
		//ZeroMemory((void*)&FileOp,sizeof(SHFILEOPSTRUCT));
		//FileOp.fFlags = FOF_NOCONFIRMATION;
		//FileOp.hNameMappings = NULL;
		//FileOp.hwnd = NULL;
		//FileOp.lpszProgressTitle = NULL;
		//FileOp.pFrom = lpszPath;
		//FileOp.pTo = NULL;
		//FileOp.wFunc = FO_DELETE;

		//return SHFileOperation(&FileOp) == 0;

	return true;
	
}
void CVideoEditingView::OnViewDepth()
{
	// TODO: Add your command handler code here
	mode=DEPTHVIEW;
	 layout=&gridLayout;
	CString str;
	str=filePath;
	str.AppendFormat("%s","VEDepthMap\\");
	str.AppendFormat("%s",filename);
	printf("before: %s \n ", trimapExt.GetBuffer());
	resImageBuffer.SetName(str,trimapExt);
	printf("pos:%d\n",resImageBuffer.GetFramePos());
	resImageBuffer.Goto(resImageBuffer.GetFramePos());
	printf("before invalidate\n");
	Invalidate();
}

void CVideoEditingView::OnUpdateViewDepth(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(mode==DEPTHVIEW&& layout==&gridLayout);
}

void CVideoEditingView::OnFileOpenproject()
{
	// TODO: Add your command handler code here
	CVEProject pro;
	CVEFileParser parser(&pro);
	LPCTSTR lpszFilter="Project Files (*.ve)||";//_T("JPG (*.jpg)|*.jpg|BMP (*.bmp)|*.bmp|PNG (*.png)|*.png|");
	CFileDialog Dlg(TRUE, "ve",filename, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		lpszFilter, NULL);
	InitRegion();
	if (Dlg.DoModal()==IDOK)
	{
		CString path=Dlg.GetPathName();
		int result = parser.LoadProject(path.GetBuffer());
		if (result==0)
				return;
		this->imagePath=pro.m_sequencePath;
		std::cout<<imagePath<<"  this is path"<<std::endl;
		std::cout<<path<<"this is path3"<<std::endl;

		CString fname;
		CString pathname;

		CString ext;
		pathname=pro.m_sequencePath;
		
		int index=pathname.ReverseFind('\\');
		fname=pathname.Mid(index+1,pathname.GetLength());

		index=pathname.ReverseFind('\\');
		pathname.Delete(index+1,pathname.GetLength()+1);//left only the path D:/data/

		this->filePath=pathname;
		std::cout<<filePath<<"this is path2"<<std::endl;
	
		editLayerPath=pathname;
		editLayerPath.AppendFormat("%s","VEData");
		DeleteFolder(editLayerPath);
		CreateDirectory(editLayerPath,0);
		editLayerPath.AppendFormat("%s","\\");

		index=fname.Find('.');
		ext=fname.Mid(index+1,3);//file ext name
		CString temp;
		temp=fname.Mid(0,fname.GetLength()-4);
		int i;
		for (i=temp.GetLength()-1;i>=0;--i)
		{
			if (!isdigit(temp[i]))
			{
				break;
			}
		}
		fname=temp.Mid(0,i+1);//文件名的非数字部分
		filename=fname;


		temp=temp.Right(temp.GetLength()-i-1);//文件名的数字部分
		pathname.AppendFormat("%s",fname);//D:/data/image
		namelength=temp.GetLength();

		backGroundLayerPath=pathname;

		imageExt = ext;
		imageBuffer.SetName(pathname,ext);
		layerBuffer.SetName(pathname,ext);

		startIndex=_ttoi(temp);
		printf("startindex:%d\n",startIndex);
		int tempindex=startIndex;
		fileRange=0;
		CString tempnum;
		tempnum.Format("%d",namelength);

		cout<<"before while true"<<endl;
		while(true)
		{
			CString temppath;
			temppath.Format("%s%."+tempnum+"d%s",pathname,tempindex,"."+ext);
			//MessageBox(temppath);
			if (access(temppath.GetBuffer(),0)==-1)
			{
				break;
			}
			fileRange++;
			tempindex++;
		}
		((CMainFrame *)(AfxGetApp()->m_pMainWnd))->setRange(fileRange);

		cout<<"before open video"<<endl;

		layerBuffer.OpenVideo(_ttoi(temp));
		if(imageBuffer.OpenVideo(_ttoi(temp)))
		{
			hasMoved=true;
			GetDocument()->New();
			GetDocument()->Opened=true;
			Invalidate();
		}

		CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
		this->vec_layer=pro.vec_layer;
		this->polygonManager = this->vec_layer[0].GetPolygonManager();

		if(pro.layerNum>0)
			firstLayerCreated=true;

		for (int i=0;i<pro.layerNum;++i)
		{
			pMain->addLayer(vec_layer[i].GetName(),vec_layer[i].GetType());
		}
		cout<<"after addlayer"<<endl;
		imageOpened=true;
		//allLayerGenerated=true;
		if (vec_layer.size())
		{
			current_layer=0;
			resImageBuffer.SetPos(pro.GetStartFrame());
			trimapBuffer.SetPos(pro.GetStartFrame());
			alphaBuffer.SetPos(pro.GetStartFrame());
			printf("%s\n",vec_layer[current_layer].GetName());
			SetRLTPath(filePath,vec_layer[current_layer].GetName());
			resImageBuffer.OpenImage(resImageBuffer.GetFrameName());
			printf("%s\n",resImageBuffer.GetFrameName());
			trimapBuffer.OpenImage(trimapBuffer.GetFrameName());
			printf("%s\n",trimapBuffer.GetFrameName());
			alphaBuffer.OpenImage(alphaBuffer.GetFrameName());
			printf("%s\n",alphaBuffer.GetFrameName());
			//mode=RESVIEW;
			//控制变量
			firstFrameOK=true;
			GetDocument()->cal=true;
			GetDocument()->updateMode=LOCAL_UPDATE;
			GetDocument()->UpdateImages(imageBuffer.GetImage());
			GetDocument()->RestoreFrame(imageBuffer.GetImage(),alphaBuffer.GetImage());
		}

		m_fDspMin = pro.m_fDspMin;
		m_fDspMax = pro.m_fDspMax;

		GenerateVEDepthMaps();
	}
}

bool CVideoEditingView::GenerateVEDepthMaps(void)
{
	if(!imageOpened)
		return false;

	ImageBuffer DepthImgBuf;
	ImageBuffer DepthRawBuf;

	CString DepthFileName = filePath;
	DepthFileName.AppendFormat("VEDepthMap\\", filename);
	CreateDirectory((LPCTSTR)DepthFileName, 0);
	DepthFileName.AppendFormat("%s", filename);

	DepthImgBuf.SetName(DepthFileName, trimapExt);
	DepthRawBuf.SetName(DepthFileName, CString("raw"));

	CString DspFilePath = filePath;
	DspFilePath.AppendFormat("Data\\");

	CxImage* pCxImg = imageBuffer.GetImage();
	int iWidth = pCxImg->GetWidth();
	int iHeight = pCxImg->GetHeight();

	int iStartFrmIdx = startIndex;
	int iEndFrmIdx = startIndex + fileRange - 1;
	for(int f = iStartFrmIdx; f <= iEndFrmIdx; ++f)
	{
		CString DepthRawName = DepthRawBuf.GetFrameName(f);
		if(_access(DepthRawName.GetBuffer(), 0) == -1)
		{
			ZFloatImage DspMap;

			CString DspFileName;
			DspFileName.Format("%s_depth%d.raw", DspFilePath, f - iStartFrmIdx);
			if(_access(DspFileName.GetBuffer(), 0) == -1)
			{
				DspMap.CreateAndInit(iWidth, iHeight, 1, m_fDspMin);
				CDisparity::SaveDspMap(DspMap, DepthRawName);
			}
			else
			{
				CopyFile((LPCTSTR)DspFileName, (LPCTSTR)DepthRawName, FALSE);
				DspMap = CDisparity::LoadDspMap(iWidth, iHeight, DepthRawName);
			}

			ZByteImage DepthMap = CDisparity::DspMap2ByteImg(DspMap, m_fDspMin, m_fDspMax);

			CxImage DepthImg;
			ByteToCxImg2(DepthMap, DepthImg);
			DepthImg.Save(DepthImgBuf.GetFrameName(f), CXIMAGE_FORMAT_PNG);
		}
	}

	return true;
}

void CVideoEditingView::OnFileSaveProject()
{
	// TODO: Add your command handler code here

	//为保证算法结果的一致性，我们检测是否有些layer没有alpha或者mask，进行信息补全
	//for (int i=0; i<vec_layer.size(); ++i)
	//{
	//	this->SetRLTPath(filePath, vec_layer[i].GetName());
	//	ImageBuffer softAlphaBuffer;
	//	CString path = filePath;
	//	path.AppendFormat("%s\\%s", vec_layer[i].GetName(),"VEAlpha");
	//	//CreateDirectory(path,0);
	//	path.AppendFormat("\\%s", filename);
	//	softAlphaBuffer.SetName(path, alphaExt);
	//	bool alphaexist = softAlphaBuffer.OpenVideo(startFrame);
	//	if (alphaBuffer.OpenVideo(vec_layer[i].GetStartIndex())==false)//不存在mask
	//	{
	//		if(alphaexist)
	//		{
	//			do 
	//			{
	//				CxImage mask;
	//				mask.Copy(*(softAlphaBuffer.GetImage()));
	//				for (int jtemp=0;jtemp<mask.GetHeight(); ++jtemp)
	//				{
	//					for (int itemp=0; itemp<mask.GetWidth(); ++itemp)
	//					{
	//						BYTE gray = mask.GetPixelGray(itemp,jtemp);
	//						if (gray!=255)
	//							mask.SetPixelColor(itemp, jtemp, RGB(0,0,0));
	//					}
	//				}
	//				alphaBuffer.SetPos(softAlphaBuffer.GetFramePos());
	//				mask.Save(alphaBuffer.GetFrameName().GetBuffer(),CXIMAGE_FORMAT_PNG);
	//			} while (softAlphaBuffer.Forward());
	//		}
	//	}
	//}
	vec_layer[current_layer].SetPolygonManager( polygonManager );
	CVEProject pro;
	pro.m_sequencePath=imagePath;
	pro.vec_layer=this->vec_layer;
	pro.SetStartFrame(startIndex);
	pro.SetEndFrame(startIndex+fileRange-1);
	pro.SetFrameCount(fileRange);
	pro.m_fDspMin = m_fDspMin;
	pro.m_fDspMax = m_fDspMax;
	printf("path: %s\n", pro.m_sequencePath.GetBuffer());

	CVEFileParser fparser(&pro);
	LPCTSTR lpszFilter="Project Files (*.ve)||";//_T("JPG (*.jpg)|*.jpg|BMP (*.bmp)|*.bmp|PNG (*.png)|*.png|");
	CFileDialog Dlg(false, "ve",filename, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		lpszFilter, NULL);

	if (Dlg.DoModal()==IDOK)
	{
		CString path=Dlg.GetPathName();
		fparser.SaveProject(path.GetBuffer());
		pro.SavePolygon();
	}
}

void CVideoEditingView::OnUpdateRunGetfinallayer(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(firstLayerCreated);
}
//added func for tool manage
LRESULT CVideoEditingView::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( IsToolManager && !imageOpened )
	{//added to deal with when image is not loaded and the polygon tool button is on
		CMainFrame *pMain=( CMainFrame *)AfxGetApp()->m_pMainWnd;
		pMain->OnToolsBaseRange( ID_BUTTON_POLYGON );
		
	}
	if ( IsToolManager && imageOpened && GetCurrentLayerNum() == -1 )
	{//added to deal with when image is not loaded and the polygon tool button is on
		CMainFrame *pMain=( CMainFrame *)AfxGetApp()->m_pMainWnd;
		pMain->OnToolsBaseRange( ID_BUTTON_POLYGON );
		
		MessageBox("No layer Selected!\nPlease select a Layer Or Create a New Layer first!\n");
	}
	CMainFrame* pMainFrame=(CMainFrame*)AfxGetMainWnd();
	CTool* pTool=0;
	if (pMainFrame)
		pTool=pMainFrame->GetActiveTool();
	if (pTool)
	{
		if(!pTool->WindowProc(this,message,wParam,lParam))
			return 0;
	}
	return CView::DefWindowProc(message,wParam,lParam);	
} 

void CVideoEditingView::AdjustPos( int& x, int& y, RECT& rect )
{
	double z, zx;
	z=( (double)imageBuffer.GetImage()->GetHeight())/( rect.bottom - rect.top + 1 );
	zx = ( (double)imageBuffer.GetImage()->GetWidth())/( rect.right - rect.left + 1 );
	x=(int)( ( x - rect.left + 1 ) * zx );
	y=imageBuffer.GetImage()->GetHeight() - 1 - (int)( ( y - rect.top + 1 ) * z );

	x = __max(0, x);
	x = __min(x, imageBuffer.GetImage()->GetWidth()-1);
	y=__max(0, y);
	y=__min(y, imageBuffer.GetImage()->GetHeight()-1);
	
}
void CVideoEditingView::AdjustPointToView( CPoint& point, RECT& rect )
{
	double z, zx;
	int height = imageBuffer.GetImage()->GetHeight();
	int width = imageBuffer.GetImage()->GetWidth();
	int bottom = rect.bottom;
	int top = rect.top;
	int left = rect.left;
	int right = rect.right;
	z = (double)height / ( bottom - top + 1 );
	zx = ( (double)width )/( right - left + 1 );
	point.x = (int)( (double)point.x / zx + left - 1 );
	point.y = (int)( (double)( height - 1 - point.y ) / z + top - 1 );

	point.x = __max(point.x, left);
	point.x = __min(point.x, right);
	point.y = __max(point.y, top);
	point.y = __min(point.y, bottom);

}

int CVideoEditingView::GetCurrentLayerNum()
{
	return current_layer;
}

void CVideoEditingView::SavePolygonManager( int layer_index )
{
	if ( layer_index >= 0 && layer_index < vec_layer.size() )
	{
		vec_layer[ layer_index ].SetPolygonManager( polygonManager );
	}
}

#include "../../CutRef/CutRef/Refiner.h"
#include "MattingWorkUnit.h"
void CVideoEditingView::OnHelpTestfillholes()
{
//	// TODO: Add your command handler code here
//	CString _filepath = "H:\\2D to 3D\\data\\woman";
//	CString _layerpath = _filepath;
//	_layerpath.AppendFormat("/woman/");
//	CString _layerlabelpath = _layerpath;
//	_layerlabelpath.AppendFormat("VELabel/");
//	CString _layeralphaPath = _layerpath;
//	_layeralphaPath.AppendFormat("VEAlpha/");
//	CString _depthPath = _filepath;
//	_depthPath.AppendFormat("/VEDepthMap/");
//	for (int i=18; i<=55;++i)
//	{
//		CString temp;
//		temp.Format("%d", i);
//		CString name;
//		if (temp.GetLength()<4)
//		{
//			name.Format("%.4d", i);
//		}
//		else
//			name.Format("%d", i);
//
//		CString depthPath = _depthPath;
//		//depthPath.AppendFormat("%d.png", i);
//		depthPath += name +".png";
//
//		CString labelPath = _layerlabelpath;
//		//labelPath.AppendFormat("%d.png", i);
//		labelPath += name + ".png";
//
//		CString alphapath = _layeralphaPath;
//		//alphapath.AppendFormat("%d.png", i);
//		alphapath += name + ".png";
//
//		CxImage depth, label, alpha;
//		depth.Load(depthPath.GetBuffer(), CXIMAGE_FORMAT_PNG);
//		label.Load(labelPath.GetBuffer(), CXIMAGE_FORMAT_PNG);
//		alpha.Load(alphapath.GetBuffer(), CXIMAGE_FORMAT_PNG);
//
//		CxImage newdepth;
//		newdepth.Copy(depth);
//		depth.Save("H:/td.png",CXIMAGE_FORMAT_PNG);
//		label.Save("H:/tl.png", CXIMAGE_FORMAT_PNG);
//		alpha.Save("H:/ta.png", CXIMAGE_FORMAT_PNG);
//		SoftDepth(depth, label, alpha, newdepth);
//		CString storepath = "H:/3dvideo/";
//		//storepath.AppendFormat("%d.png", i);
//		storepath += name+".png";
//		newdepth.Save(storepath.GetBuffer(), CXIMAGE_FORMAT_PNG);
	//}
}

CString CVideoEditingView::GetFilePath() const
{
	return filePath;
}

void CVideoEditingView::OnHelpDepthcomposition()
{
}

void CVideoEditingView::SoftDepth(CxImage& depth, CxImage& label, CxImage& alpha, CxImage& newdepth)
{
	int width = depth.GetWidth(); 
	int height = depth.GetHeight();

	double layerdepth=-1;
	for (int j=0; j<height; ++j)
	{
		for (int i=0; i<width; ++i)
		{
			if (label.GetPixelGray(i,j)&&alpha.GetPixelGray(i,j)==255)
			{
				layerdepth = ((int)depth.GetPixelGray(i,j))*1.0;
				break;
			}			 
		}
		if (layerdepth!=-1)
			break;		
	}

	for (int j=0; j<height; ++j)
	{
		for (int i=0; i<width; ++i)
		{
			double alphaValue = ((int)alpha.GetPixelGray(i,j))/255.0;
			double depthvalue = ((int)depth.GetPixelGray(i,j))*1.0;

			double newdepthvalue = layerdepth*alphaValue+(1-alphaValue)*depthvalue;
			BYTE col = (BYTE)newdepthvalue;
			newdepth.SetPixelColor(i,j, RGB(col, col, col));
		}
	}
	newdepth.Save("H:/test_depth.png", CXIMAGE_FORMAT_PNG);
}

void CVideoEditingView::RecompositeTrimap(void)
{
	CxImage result;
	if(imageBuffer.GetImage())
		result.Copy(*imageBuffer.GetImage());
	CxImage trimap;
	trimap.Load(trimapBuffer.GetFrameName());
	if(result.IsValid() && trimap.IsValid())
	{
		Composite(result, result, trimap, m_fTriMpAlpha);
		trimapBuffer.OpenImage(result);
	}
}

void CVideoEditingView::OnRunDefaultLayerDepth()
{
	// TODO: Add your command handler code here
	if(!imageOpened)
		return;

	CString DepthFileName = filePath;
	DepthFileName.AppendFormat("VEDepthMap\\", filename);
	CreateDirectory((LPCTSTR)DepthFileName, 0);
	DepthFileName.AppendFormat("%s", filename);
	ImageBuffer DepthRawBuf;
	DepthRawBuf.SetName(DepthFileName, CString("raw"));
	//ImageBuffer DepthImgBuf;
	//DepthImgBuf.SetName(DepthFileName, trimapExt);

	//CString DspFilePath = filePath;
	//DspFilePath.AppendFormat("Data\\");

	CxImage* pCxImg = imageBuffer.GetImage();
	int iWidth = pCxImg->GetWidth();
	int iHeight = pCxImg->GetHeight();

	int iStartFrmIdx = startIndex;
	int iEndFrmIdx = startIndex + fileRange - 1;

	for(int k = 0; k < vec_layer.size(); k++)
	{
		CString LyrDepthPath = filePath;
		LyrDepthPath.AppendFormat("%s\\%s", vec_layer[k].GetName(), "Depth");
		LyrDepthPath.AppendFormat("\\%s", filename);
		ImageBuffer LyrDepthBuf;
		LyrDepthBuf.SetName(LyrDepthPath, alphaExt);
		ImageBuffer LyrDspBuf;
		LyrDspBuf.SetName(LyrDepthPath, CString("raw"));

		CString LyrAlphaPath = filePath;
		LyrAlphaPath.AppendFormat("%s\\%s", vec_layer[k].GetName(), "VEAlpha");
		LyrAlphaPath.AppendFormat("\\%s", filename);
		ImageBuffer LyrAlphaBuf;
		LyrAlphaBuf.SetName(LyrAlphaPath, alphaExt);

		CString LyrTriMapPath = filePath;
		LyrTriMapPath.AppendFormat("%s\\%s", vec_layer[k].GetName(), "VETrimap");
		LyrTriMapPath.AppendFormat("\\%s", filename);
		ImageBuffer LyrTriMapBuf;
		LyrTriMapBuf.SetName(LyrTriMapPath, alphaExt);

		for(int f = iStartFrmIdx; f <= iEndFrmIdx; ++f)
		{
			CString DepthRawName = DepthRawBuf.GetFrameName(f);
			//CString DepthImgName = DepthImgBuf.GetFrameName(f);
			CString LyrDepthName = LyrDepthBuf.GetFrameName(f);
			CString LyrDspName = LyrDspBuf.GetFrameName(f);
			CString LyrAlphaName = LyrAlphaBuf.GetFrameName(f);
			CString LyrTriMapName = LyrTriMapBuf.GetFrameName(f);

			if(_access(LyrDepthName.GetBuffer(), 0) == -1 && _access(LyrDspName.GetBuffer(), 0) == -1 && _access(DepthRawName.GetBuffer(), 0) != -1/* && _access(DepthImgName.GetBuffer(), 0) != -1*/ && _access(LyrAlphaName.GetBuffer(), 0) != -1 && _access(LyrTriMapName.GetBuffer(), 0) != -1)
			{
				ZFloatImage DspMap = CDisparity::LoadDspMap(iWidth, iHeight, DepthRawName);
				//CxImage DepthImg;
				//DepthImg.Load((LPCTSTR)DepthImgName);
				//DepthImg.GrayScale();

				CxImage LyrAlphaImg;
				LyrAlphaImg.Load((LPCTSTR)LyrAlphaName);
				LyrAlphaImg.GrayScale();
				
				CxImage LyrTriMap;
				LyrTriMap.Load((LPCTSTR)LyrTriMapName);
				LyrTriMap.GrayScale();

				ZFloatImage LyrDspMap;
				LyrDspMap.CreateAndInit(iWidth, iHeight, 1, m_fDspMin);
				
				//CxImage LyrDepthImg(iWidth, iHeight, 8);
				//LyrDepthImg.SetGrayPalette();
				//LyrDepthImg.Clear(0);

				for(int y = 0; y < iHeight; ++y)
				{
					for(int x = 0; x < iWidth; ++x)
					{
						if(LyrTriMap.GetPixelIndex(x, iHeight - 1 - y) == 255)
						{
							LyrDspMap.at(x, y) = DspMap.at(x, y);
							//LyrDepthImg.SetPixelIndex(x, iHeight - 1 - y, DepthImg.GetPixelIndex(x, iHeight - 1 - y));
						}
						//else
						//	LyrDspMap.at(x, y) = m_fDspMin;
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
				LyrDspMap = CInpainting::DepthInpainting(LyrDspMap, IptMsk);

				CxImage LyrDepthImg = CDisparity::DspMap2CxImg(LyrDspMap, m_fDspMin, m_fDspMax);

				CDisparity::SaveDspMap(LyrDspMap, LyrDspName);
				LyrDepthImg.Save((LPCTSTR)LyrDepthName, CXIMAGE_FORMAT_PNG);
			}
		}
	}
}

void CVideoEditingView::OnRunUnknownOpticalFlow()
{
	// TODO: Add your command handler code here
	if(!imageOpened)
		return;

	CxImage* pCxImg = imageBuffer.GetImage();
	int iWidth = pCxImg->GetWidth();
	int iHeight = pCxImg->GetHeight();

	int iStartFrmIdx = startIndex;
	int iEndFrmIdx = startIndex + fileRange - 1;

	for(int k = 0; k < vec_layer.size(); k++)
	{
		CString LyrOptFlowPath = filePath;
		LyrOptFlowPath.AppendFormat("%s\\opt\\", vec_layer[k].GetName());

		CString LyrTriMapPath = filePath;
		LyrTriMapPath.AppendFormat("%s\\VETrimap\\", vec_layer[k].GetName());
		LyrTriMapPath.AppendFormat("%s", filename);
		ImageBuffer LyrTriMapBuf;
		LyrTriMapBuf.SetName(LyrTriMapPath, alphaExt);

		for(int f = iStartFrmIdx; f <= iEndFrmIdx; ++f)
		{
			if(f < iEndFrmIdx)
			{
				CString LyrOptFlowUName = LyrOptFlowPath;
				LyrOptFlowUName.AppendFormat("\\u_%d_%d.raw", f, f + 1);
				CString LyrOptFlowVName = LyrOptFlowPath;
				LyrOptFlowVName.AppendFormat("\\v_%d_%d.raw", f, f + 1);

				std::vector<float> U(iWidth * iHeight);
				std::vector<float> V(iWidth * iHeight);

				FILE* pFile1 = fopen(LyrOptFlowUName.GetBuffer(), "rb");
				FILE* pFile2 = fopen(LyrOptFlowVName.GetBuffer(), "rb");
				CString LyrTriMapName = LyrTriMapBuf.GetFrameName(f);

				if(pFile1 && pFile2 && _access(LyrTriMapName.GetBuffer(), 0) != -1)
				{
					fread(&U[0], iWidth * iHeight, sizeof(float), pFile1);
					fclose(pFile1);
					fread(&V[0], iWidth * iHeight, sizeof(float), pFile2);
					fclose(pFile2);

					ZFloatImage OptFlowMap(iWidth, iHeight, 2);
					for(int y = 0, p = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++, p++)
						{
							OptFlowMap.at(x, iHeight - 1 - y, 0) = U[p];
							OptFlowMap.at(x, iHeight - 1 - y, 1) = V[p];
						}
					}

					CxImage LyrTriMap;
					LyrTriMap.Load((LPCTSTR)LyrTriMapName);
					LyrTriMap.GrayScale();

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

					pFile1 = fopen(LyrOptFlowUName.GetBuffer(), "wb+");
					pFile2 = fopen(LyrOptFlowVName.GetBuffer(), "wb+");

					fwrite(&U[0], iWidth * iHeight, sizeof(float), pFile1);
					fclose(pFile1);
					fwrite(&V[0], iWidth * iHeight, sizeof(float), pFile2);
					fclose(pFile2);
				}
			}

			if(f > iStartFrmIdx)
			{
				CString LyrOptFlowUName = LyrOptFlowPath;
				LyrOptFlowUName.AppendFormat("\\u_%d_%d.raw", f, f - 1);
				CString LyrOptFlowVName = LyrOptFlowPath;
				LyrOptFlowVName.AppendFormat("\\v_%d_%d.raw", f, f - 1);

				std::vector<float> U(iWidth * iHeight);
				std::vector<float> V(iWidth * iHeight);

				FILE* pFile1 = fopen(LyrOptFlowUName.GetBuffer(), "rb");
				FILE* pFile2 = fopen(LyrOptFlowVName.GetBuffer(), "rb");
				CString LyrTriMapName = LyrTriMapBuf.GetFrameName(f);

				if(pFile1 && pFile2 && _access(LyrTriMapName.GetBuffer(), 0) != -1)
				{
					fread(&U[0], iWidth * iHeight, sizeof(float), pFile1);
					fclose(pFile1);
					fread(&V[0], iWidth * iHeight, sizeof(float), pFile2);
					fclose(pFile2);

					ZFloatImage OptFlowMap(iWidth, iHeight, 2);
					for(int y = 0, p = 0; y < iHeight; y++)
					{
						for(int x = 0; x < iWidth; x++, p++)
						{
							OptFlowMap.at(x, iHeight - 1 - y, 0) = U[p];
							OptFlowMap.at(x, iHeight - 1 - y, 1) = V[p];
						}
					}

					CxImage LyrTriMap;
					LyrTriMap.Load((LPCTSTR)LyrTriMapName);
					LyrTriMap.GrayScale();

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

					pFile1 = fopen(LyrOptFlowUName.GetBuffer(), "wb+");
					pFile2 = fopen(LyrOptFlowVName.GetBuffer(), "wb+");

					fwrite(&U[0], iWidth * iHeight, sizeof(float), pFile1);
					fclose(pFile1);
					fwrite(&V[0], iWidth * iHeight, sizeof(float), pFile2);
					fclose(pFile2);
				}
			}
		}
	}
}

int CVideoEditingView::getStartIndex()
{
	return startIndex;

}

void CVideoEditingView::OnRunRotoscoping()
{
	// TODO: Add your command handler code here
	if (rotoScopeInterface != NULL)
	{
		rotoScopeInterface->loadRotoScopeData();
		rotoScopeInterface->startRotoscope();
		rotoScopeInterface->getResult();
		cout << "rotoscoping over!" << endl;
	}
	
}

void CVideoEditingView::saveMaskImage( int sIndex, int eIndex, int lb, int ls, float bmin, float bmax )
{
	//printf("....\n");
	//printf("%s", maskImagePath);
	if (rotoScopeInterface != NULL)
	{
		for (int i = sIndex; i <= eIndex; i++)
		{
			rotoScopeInterface->setAlphaSmoothParameter(ls, lb, bmin, bmax);
			CxImage * img = rotoScopeInterface->getMaskImage(i);
			//printf("%s\n", maskImageBuffer.GetFrameName(startIndex + i));
			img->Save((LPCTSTR)(maskImageBuffer.GetFrameName(startIndex + i)), CXIMAGE_FORMAT_PNG);
			delete img;
		}
		
	}
}


void CVideoEditingView::RotoScopingCurveImport()
{
	// TODO: Add your control notification handler code here
	LPCTSTR lpszFilter="Roto Files (*.roto, *.txt)|*.roto; *.txt||";
	CFileDialog Dlg(TRUE, NULL, NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		lpszFilter, NULL);
	char * filename;
	
	
	if(Dlg.DoModal()==IDOK)
	{
		filename = (LPSTR)(LPCTSTR)Dlg.GetPathName();
		ifstream in(filename);
		if (in.fail())
		{
			cout << "open file failed!" << endl;
			return;
		}

		//ofstream ou("H:\\out.txt");

		

		string str;
		int numOfFrame;
		int startFrameIndex;
		bool isClosed;
		int numofPointPerFrame;
		int key;

		getline(in, str);//<Layer Name>
		getline(in, str);
		current_layerName = str.c_str();
		getline(in, str);//</Layer Name>

		//ou << str << endl;
		

		getline(in, str);//empty line

		getline(in, str);//<Frame Number>
		in >> numOfFrame;
		if (numOfFrame != fileRange)
		{
			cout << "The curves are not fit to vedio!" << endl;
			return;
		}
		getline(in, str);
		getline(in, str);//</Frame Number>

		//ou << numOfFrame << endl;

		getline(in, str); // <Start Index>
		in >> startFrameIndex;
		cout << startFrameIndex << endl;
		if (startFrameIndex != startIndex)
		{
			cout << "The curves are not fit to vedio!" << endl;
			return;
		}
		getline(in, str);
		getline(in, str); //</Start Index>

		//ou << startFrameIndex << endl;

		int temp;
		getline(in, str); //<Curve Is Closed?>
		in >> temp;
		isClosed = (bool)temp;
		if (temp == 1)
		{
			isClosed = true;
		}
		getline(in, str);
		getline(in, str); //</Curve Is Closed?>

		//ou << isClosed << endl;


		getline(in, str);//<Control Point Number(Perframe)>
		in >> numofPointPerFrame;
		getline(in, str);
		getline(in, str);//</Control Point Number(Perframe)>

		getline(in, str); //empty line

		//ou << numofPointPerFrame << endl;
		

		AddLayer(current_layerName, -1);
		//intialize
		int height = imageBuffer.GetImage()->GetHeight();
		int width  = imageBuffer.GetImage()->GetWidth();

		if (rotoScopeInterface != NULL)
		{
			delete rotoScopeInterface;
		}
		rotoScopeInterface = NULL;
		rotoScopeInterface = new RotoScopeInterface;

		rotoScopeInterface->initializeRotoscopeData(fileRange, height, width,  imageBuffer.GetName(),
			imageBuffer.GetExt(), getStartIndex());
	/*	rotoScopeInterface->getDataManager()->distroyCurveData();
		rotoScopeInterface->getDataManager()->initilaizeCurveData(fileRange);*/

		b_IsRotoScopingReadData = true;
		b_IsClosed = isClosed;
		
	
		for (int iFrame = 0; iFrame < numOfFrame; iFrame++)
		{
			BeizerCurveData * data = &(rotoScopeInterface->getDataManager()->getCurveData(iFrame));
			//data->setCurveColsed();
			float tx;
			float ty;
			int ti;
			getline(in, str); //empty line
			getline(in, str);//<Frame i>
			getline(in, str);//<key>
			in >> key;
			rotoScopeInterface->getKeyFrameFlag()[iFrame] = key;
			getline(in, str);
			getline(in, str);//</key>
			//ou << endl;
			for (int iPoint = 0; iPoint < numofPointPerFrame; iPoint++)
			{
				in >> tx >> ty >> ti;
				//ou << tx << ty << ti << endl;
				CPoint p(tx, ty);
				data->addPoint(p, (bool)ti);
			}
			getline(in, str);
			getline(in, str);//</Frame i>
		}

		if (b_IsClosed)
		{
			rotoScopeInterface->getDataManager()->setCurvesClosed();
		}
	}
}

void CVideoEditingView::RotoScopingCurveExport()
{

	// TODO: Add your control notification handler code here
	CString current_layerPath = filePath;
	current_layerPath.AppendFormat("%s", current_layerName);
	current_layerPath.AppendFormat("%s", "\\");
	CString curveFileName = current_layerName;
	curveFileName.AppendFormat("%s", ".roto");
	CString curveFilePath = current_layerPath;
	curveFilePath.AppendFormat("%s", curveFileName);

	ofstream out(curveFilePath);
	if (out.fail())
	{
		cout << "open file failed!" << endl;
		return;
	}

	out << "<Layer Name>" << endl;
	out << current_layerName << endl;
	out << "</Layer Name>" << endl;
	out << endl;

	out << "<Frame Number>" << endl;
	out << fileRange << endl;
	out << "</Frame Number>" << endl;

	out << "<Start Index>" << endl;
	out << startIndex << endl;
	out << "</Start Index>" << endl;

	out << "<Curve Is Closed?>" << endl;
	out << rotoScopeInterface->getDataManager()->isClosed() << endl;
	out << "</Curve Is Closed?>" << endl;
	

	out << "<Control Point Number(Perframe)>" << endl;
	out << rotoScopeInterface->getDataManager()->getCurveData(0).getNumofPoint() << endl;
	out << "</Control Point Number(Perframe)>" << endl;

	out << endl;
	out << endl;

	
	for (int i = 0; i < fileRange; i++)
	{
		out << "<Frame " << i << ">" << endl;
		out << "<key?>" << endl;
		out << rotoScopeInterface->getKeyFrameFlag()[i] << endl;
		out << "</key?>" << endl;
		for (int j = 0; j < rotoScopeInterface->getDataManager()->getCurveData(0).getNumofPoint(); j++)
		{
			out << rotoScopeInterface->getDataManager()->getCurveData(i).getPoint(j).x() << " ";
			out << rotoScopeInterface->getDataManager()->getCurveData(i).getPoint(j).y() << " ";
			out << rotoScopeInterface->getDataManager()->getCurveData(i).getPoint(j).bIsInteractive <<endl;
		}
		out << "</Frame " << i << ">" << endl;
		out << endl;
	}

	cout << "over" << endl;
	out.close();

}
