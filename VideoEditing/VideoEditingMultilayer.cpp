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

void CVideoEditingView::SetLayerIndex(CString name)
{
	current_layerName=name;
	if (name=="")
	{
		current_layer=-1;
		return;
	}
	printf("current_layer=%d\n",current_layer);
	CString lname;
	for (int i=0;i<vec_layer.size();++i)
	{
		lname=vec_layer[i].GetName();
		if (lname==current_layerName)
		{
			printf("find layer %d\n",i);
			if ( current_layer != -1 )
			{
				SavePolygonManager( current_layer );
				polygonManager.Clear();
			}
				polygonManager = vec_layer[i].GetPolygonManager();
			current_layer=i;
			break;
		}
	}
}

void CVideoEditingView::UpdateLayerInfo(CString oldname,CString newname,int newDepth)
{
	for (int i=0;i<vec_layer.size();++i)
	{
		if (vec_layer[i].GetName()==oldname)
		{
			//vec_layer[i].SetName(newname);
			vec_layer[i].SetDepth(newDepth);
			break;
		}
	}
}

bool CVideoEditingView::AddLayer(CString name,int depth)
{
	if(name=="")
		return false;

	CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
	if (!firstLayerCreated)
	{
		firstLayerCreated=true;
		LayerObject newlayer;
		//newlayer.SetName("background");
		//newlayer.SetDepth(0);
		//vec_layer.push_back(newlayer);

		//newlayer.SetName("editLayer");
		//newlayer.SetDepth(0);
		//vec_layer.push_back(newlayer);

		////update listView
		//pMain->addLayer("background",0);
		//pMain->addLayer("editLayer",0);

	}

	for (int i=0;i<vec_layer.size();++i)
	{
		if (vec_layer[i].GetName()==name)
		{
			MessageBox("Layer Name already exists!");
			return false;
		}
	}

	LayerObject newlayer;
	newlayer.SetName(name);
	//newlayer.SetDepth(depth);
	newlayer.SetType(depth);
	newlayer.SetStartIndex(imageBuffer.GetFramePos());

	vec_layer.push_back(newlayer);
	SavePolygonManager( current_layer );
	polygonManager.Clear();
	current_layer=vec_layer.size()-1;
	current_layerName=name;
	this->SetRLTPath(filePath,name);
	pMain->addLayer(name,depth);
	vec_layer[current_layer].SetPos(imageBuffer.GetFramePos());
	return true;
}


void CVideoEditingView::UpdateImageBuffers(CString layerName)
{

	if (layerName!="background"&&layerName!="editLayer")
	{
		this->SetRLTPath(filePath,layerName);
	}
	//int pos=imageBuffer.GetFramePos();
	int pos=vec_layer[current_layer].GetPos();

	imageBuffer.Goto(pos);
	alphaBuffer.Goto(pos); 
	trimapBuffer.Goto(pos);
	resImageBuffer.Goto(pos);
	this->setProcess();
	CString str;
	str=editLayerPath;
	str.AppendFormat("%s",filename);
	layerBuffer.SetName(str, imageExt);
	
	if(!layerBuffer.Goto(pos))
	{
		layerBuffer.SetName(backGroundLayerPath, imageExt);
		layerBuffer.Goto(pos);
	}

	AfxGetMainWnd()->SetWindowText(imageBuffer.GetFrameName());
	GetDocument()->RestoreFrame(imageBuffer.GetImage(),alphaBuffer.GetImage());
	Invalidate();
	if ( IsToolManager && polygonManager.ExistPolygon( imageBuffer.GetFramePos() ) )
	{//added when exchange the layer, display the correspond polygon
		CPolygonSet polygon_set;
		polygonManager.GetPolygonSet( imageBuffer.GetFramePos(), polygon_set );
		imageEditView->ReDrawRectRegion( polygon_set );
	}

}


void CVideoEditingView::GetEditLayer(CxImage *frame, CxImage *label,CxImage& layer)
{
	int w,h;
	layer.Copy(*frame);
	RGBQUAD rgb;
	RGBQUAD rgb2;
	rgb2.rgbBlue=rgb2.rgbGreen=rgb2.rgbRed=0;
	for (int j=0;j<frame->GetHeight();++j)
	{
		for (int i=0;i<frame->GetWidth();++i)
		{
			rgb=label->GetPixelColor(i,j);
			if(rgb.rgbBlue==255)
				layer.SetPixelColor(i,j,rgb2);
		}
	}

}

CString CVideoEditingView::GetEditLayerPath()
{
	return editLayerPath;
}

void CVideoEditingView::SetLayerPos(int p)
{
	this->vec_layer[current_layer].SetPos(p);
}

void CVideoEditingView::GenerateImage(CxImage &im)
{
	CxImage *imtemp=imageBuffer.GetImage();
	im.Copy(*imtemp);

	CxImage labelImage;

	CString str;
	RGBQUAD rgb1;
	RGBQUAD rgb2;
	rgb2.rgbBlue=rgb2.rgbGreen=rgb2.rgbRed=0;

	CString tframename;
	tframename=imageBuffer.GetFileName();
	int index=tframename.Find(".");
	tframename.Delete(index,4);

	for (int i=0;i<vec_layer.size();++i)
	{
		str=filePath;
		if (i!=current_layer)
		{	
			str.AppendFormat("%s%s%s%s%s",vec_layer[i].GetName(),"\\VELabel","\\",tframename,".bmp");

			labelImage.Load(str);

			for (int j=0;j<labelImage.GetWidth();++j)
			{
				for (int k=0;k<labelImage.GetHeight();++k)
				{
					rgb1=labelImage.GetPixelColor(j,k);

					if (rgb1.rgbBlue!=0)
					{
						im.SetPixelColor(j,k,rgb2);
					}
				}
			}

		}
	}

}

void CVideoEditingView::ReUpdateResult(CxImage &resultIm,CxImage &alphaIm)
{
	CxImage labelImage;

	CString str;
	RGBQUAD rgb1;
	RGBQUAD rgb2;
	rgb2.rgbBlue=rgb2.rgbGreen=rgb2.rgbRed=0;

	CString tframename;
	tframename=imageBuffer.GetFileName();
	int index=tframename.Find(".");
	tframename.Delete(index,4);

	/*resultIm.Save("D:\\re.jpg",CXIMAGE_FORMAT_JPG);*/

	for (int i=0;i<vec_layer.size();++i)
	{
		str=filePath;
		if (i!=current_layer)
		{	
			str.AppendFormat("%s%s%s%s%s",vec_layer[i].GetName(),"\\VELabel","\\",tframename,".bmp");

			labelImage.Load(str);

			for (int j=0;j<labelImage.GetWidth();++j)
			{
				for (int k=0;k<labelImage.GetHeight();++k)
				{
					rgb1=labelImage.GetPixelColor(j,k);

					if (rgb1.rgbBlue!=0)
					{
						alphaIm.SetPixelColor(j,k,rgb2);
						resultIm.SetPixelColor(j,k,rgb2);
					}
				}
			}

		}
	}

	//alphaIm.Dilate(3);
	//alphaIm.Erode(3);
}
