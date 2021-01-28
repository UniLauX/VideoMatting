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

void CVideoEditingView::OnEditMove()
{
	//editTool=MOVE;
}

void CVideoEditingView::OnUpdateEditMove(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(editTool==MOVE);
	pCmdUI->Enable(imageOpened);
}

void CVideoEditingView::OnEditFit()
{
	imageEditView->MoveToOrigin();
	imageResultView->MoveToOrigin();
	trimapView->MoveToOrigin();
	Invalidate();
	if ( IsToolManager && polygonManager.ExistPolygon( imageBuffer.GetFramePos() ) )
	{//added when exchange the layer, display the correspond polygon
		CPolygonSet polygon_set;
		polygonManager.GetPolygonSet( imageBuffer.GetFramePos(), polygon_set );
		imageEditView->ReDrawRectRegion( polygon_set );
	}
}

void CVideoEditingView::OnEditZoomin()
{
	if (!processing&&firstFrameOK)
	{
		imageEditView->Zoom(1, 0.5);
		imageResultView->Zoom(1, 0.5);
		Invalidate();
		if ( IsToolManager && polygonManager.ExistPolygon( imageBuffer.GetFramePos() ) )
		{//added when exchange the layer, display the correspond polygon
			CPolygonSet polygon_set;
			polygonManager.GetPolygonSet( imageBuffer.GetFramePos(), polygon_set );
			imageEditView->ReDrawRectRegion( polygon_set );
		}
	}
}

void CVideoEditingView::OnEditZoomout()
{
	if (!processing&&firstFrameOK)
	{
		imageEditView->Zoom(-1, 0.5);
		imageResultView->Zoom(-1, 0.5);
		Invalidate();
		if ( IsToolManager && polygonManager.ExistPolygon( imageBuffer.GetFramePos() ) )
		{//added when exchange the layer, display the correspond polygon
			CPolygonSet polygon_set;
			polygonManager.GetPolygonSet( imageBuffer.GetFramePos(), polygon_set );
			imageEditView->ReDrawRectRegion( polygon_set );
		}
	}

}

void CVideoEditingView::OnAddNewLayer()
{
	//MessageBox("Layer name already exists");
	CLayerEditDlg layerEditDlg;
	int nresponse=layerEditDlg.DoModal();
	if (nresponse==IDOK && layerEditDlg.GetLayerName()!="" )
	{
		bool re=AddLayer(layerEditDlg.GetLayerName(),layerEditDlg.GetLayerType());
		SetLayerPos(imageBuffer.GetFramePos());
	}
}

void CVideoEditingView::OnUpdateEditFit(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(imageOpened);
}

void CVideoEditingView::OnUpdateEditZoomin(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(imageOpened);
}

void CVideoEditingView::OnUpdateEditZoomout(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(imageOpened);
}