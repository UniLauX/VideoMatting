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

void CVideoEditingView::OnGrabberRectanglegrab()
{
	editTool=GRAB;
	firstFrameOK=false;
	if (firstLayerCreated)
	{
		CString str;
		str=editLayerPath;
		str.AppendFormat("%s",filename);
		//imageBuffer.SetName(str);
		imageBuffer.SetName(str, imageExt);
		layerBuffer.SetName(str, imageExt);

		if(!imageBuffer.Forward())
		{
			imageBuffer.SetName(backGroundLayerPath, imageExt);
			layerBuffer.SetName(backGroundLayerPath, imageExt);
		}
		else
		{
			imageBuffer.Back();
			int pos=imageBuffer.GetFramePos();
			imageBuffer.Goto(pos);
			layerBuffer.Goto(pos);
			Invalidate();
		}
	}
	imageEditView->SetRectGrab();

	if ( IsToolManager )
	{
		CMainFrame *pMain=( CMainFrame *)AfxGetApp()->m_pMainWnd;
		pMain->OnToolsBaseRange( ID_BUTTON_POLYGON );
		//pMain->OnToolsBaseRange(ID_BUTTON_NURBS);
	}
}

void CVideoEditingView::OnGrabberForegroundbrush()
{
	editTool=GRABUPDATE;
	fgBrushDown=true;
	bgBrushDown=false;
	imageEditView->SetFgBrush();
	imageResultView->SetFgBrush();
	if ( IsToolManager )
	{
		CMainFrame *pMain=( CMainFrame *)AfxGetApp()->m_pMainWnd;
		pMain->OnToolsBaseRange( ID_BUTTON_POLYGON );
		//pMain->OnToolsBaseRange(ID_BUTTON_NURBS);
	}
}

void CVideoEditingView::OnGrabberBackgroundbrush()
{
	editTool=GRABUPDATE;
	fgBrushDown=false;
	bgBrushDown=true;
	imageEditView->SetBgBrush();
	imageResultView->SetBgBrush();
	if ( IsToolManager )
	{
		CMainFrame *pMain=( CMainFrame *)AfxGetApp()->m_pMainWnd;
		pMain->OnToolsBaseRange( ID_BUTTON_POLYGON );
		//pMain->OnToolsBaseRange(ID_BUTTON_NURBS);
	}
}

void CVideoEditingView::OnUpdateGrabberRectanglegrab(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(imageEditView->IsDrawingRect() && !IsToolManager );
	pCmdUI->Enable(GetDocument()->mode==GRABCUT && imageOpened && (!processing)&&(!allLayerGenerated)&&(!calDepthStarted) && !IsToolManager);
}

void CVideoEditingView::OnUpdateGrabberForegroundbrush(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(imageEditView->IsDrawingPoint() && 
		imageEditView->DrawingType()==1 &&
		(editTool==GRAB||editTool==GRABUPDATE) && !IsToolManager );
	pCmdUI->Enable(imageOpened && (!processing)&&(!allLayerGenerated)&&(!calDepthStarted)&& !IsToolManager);

}

void CVideoEditingView::OnUpdateGrabberBackgroundbrush(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(imageEditView->IsDrawingPoint() && 
		imageEditView->DrawingType()==0 &&
		(editTool==GRAB||editTool==GRABUPDATE) && !IsToolManager );
	pCmdUI->Enable(imageOpened && (!processing)&&(!allLayerGenerated)&&(!calDepthStarted)&& !IsToolManager);
}

void CVideoEditingView::OnGrabberGrabcut()
{
	GetDocument()->mode=GRABCUT;
	if ( IsToolManager )
	{
		CMainFrame *pMain=( CMainFrame *)AfxGetApp()->m_pMainWnd;
		pMain->OnToolsBaseRange( ID_BUTTON_POLYGON );
		//pMain->OnToolsBaseRange(ID_BUTTON_NURBS);
	}
}

void CVideoEditingView::OnGrabberClosedformmatting()
{
	GetDocument()->mode=CLOSEDFORM;
	if ( IsToolManager )
	{
		CMainFrame *pMain=( CMainFrame *)AfxGetApp()->m_pMainWnd;
		pMain->OnToolsBaseRange( ID_BUTTON_POLYGON );
		//pMain->OnToolsBaseRange(ID_BUTTON_NURBS);
	}
}

void CVideoEditingView::OnUpdateGrabberGrabcut(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetDocument()->mode==GRABCUT && !IsToolManager );
}

void CVideoEditingView::OnUpdateGrabberClosedformmatting(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetDocument()->mode==CLOSEDFORM && !IsToolManager);
}


void CVideoEditingView::OnPenForeground()
{
	editTool=DRAW;
	imageEditView->SetFgBrush();
	imageResultView->SetFgBrush();
	trimapView->SetFgBrush();
	if ( IsToolManager )
	{
		CMainFrame *pMain=( CMainFrame *)AfxGetApp()->m_pMainWnd;
		pMain->OnToolsBaseRange( ID_BUTTON_POLYGON );
		//pMain->OnToolsBaseRange(ID_BUTTON_NURBS);
	}
}

void CVideoEditingView::OnPenBackground()
{
	editTool=DRAW;
	imageEditView->SetBgBrush();
	imageResultView->SetBgBrush();
	trimapView->SetBgBrush();
	if ( IsToolManager )
	{
		CMainFrame *pMain=( CMainFrame *)AfxGetApp()->m_pMainWnd;
		pMain->OnToolsBaseRange( ID_BUTTON_POLYGON );
		//pMain->OnToolsBaseRange(ID_BUTTON_NURBS);
	}
}


void CVideoEditingView::OnUpdatePenForeground(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(imageEditView->DrawingType()==1 && editTool==DRAW && !IsToolManager);
	pCmdUI->Enable(imageOpened && (!processing)&& !IsToolManager);//&&trimapBuffer.flag==false);
}

void CVideoEditingView::OnUpdatePenBackground(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(imageEditView->DrawingType()==0 && editTool==DRAW && !IsToolManager);
	pCmdUI->Enable(imageOpened && (!processing)&& !IsToolManager);//&&trimapBuffer.flag==false);
}
ImageEditView* CVideoEditingView::GetImageEditingView()const
{
	return imageEditView;
}

void CVideoEditingView::OnPenUnknown(){
	editTool=DRAW;
	imageEditView->SetUnknownBrush();
	imageResultView->SetUnknownBrush();
	trimapView->SetUnknownBrush();
}
void CVideoEditingView::OnUpdatePenUnknown(CCmdUI *pCmdUI){
	pCmdUI->SetCheck(imageEditView->DrawingType()==2  &&editTool==DRAW);
	pCmdUI->Enable(imageOpened &&(!processing) &&  layout ==&gridLayoutTrimap);//&&trimapBuffer.flag==false);
}