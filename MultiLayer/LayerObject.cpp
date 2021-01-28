#include "LayerObject.h"

LayerObject::LayerObject(void)
:name("")
,depth(0)
,width(0)
,height(0)
,pos(0)
{
	label=NULL;
	keyFrame.clear();
}

LayerObject::~LayerObject(void)
{
}

void LayerObject::SetName(CString name)
{
	this->name=name;
}

CString LayerObject::GetName()
{
	return this->name;
}

void LayerObject::SetDepth(int depth)
{
	this->depth=depth;
}

int LayerObject::GetDepth()
{
	return this->depth;
}
int LayerObject::GetType()
{
	return this->layerType;
}
void LayerObject::SetType(int type)
{
	this->layerType = type;
}

void LayerObject::SetDemision(int width, int height)
{
	this->width=width;
	this->height=height;
}

int LayerObject::GetHeight()
{

	return this->height;
}

int LayerObject::GetWidth()
{
	 
	return this->width;
}

void LayerObject::SetLabel(const int* label,int width,int height)
{
	if(label==NULL)
		label=new int[width*height];
	memcpy(this->label,label,sizeof(int)*width*height);
}

int* LayerObject::GetLabel()
{
	return label;
}

int LayerObject::GetPos()
{
	return pos;
}

void LayerObject::SetPos(int p)
{
	this->pos=p;
}

int LayerObject::IsKeyFrame(int pos)//return -1 if frame pos is not key frame
{

	for (int i=0;i<keyFrame.size();++i)
	{
		if (keyFrame[i]==pos)
		{
			return i;
		}
	}
	return -1;
}

void LayerObject::AddKeyFrame(int pos)
{
	keyFrame.push_back(pos);
}

void LayerObject::AddPlanePoint(Point_3D p1, Point_3D p2, Point_3D p3)
{
	plane.push_back(p1);
	plane.push_back(p2);
	plane.push_back(p3);
}

void LayerObject::UpdateKeyFrame(int index,Point_3D p1,Point_3D p2,Point_3D p3)
{

	plane[index*3]=p1;
	plane[index*3+1]=p2;
	plane[index*3+2]=p3;
}

int LayerObject::NumOfKeyFrame()
{
	return keyFrame.size();
}

void LayerObject::GetNeighbourOfKeyFrame(int keyframe,int &preKeyFrame,int &preKeyFrameIndex,int &nextKeyFrame,int &nextKeyFrameIndex)
{
	int lastFrame=keyFrame[0],nextFrame=keyFrame[0];
	int lastIndex=-1,nextIndex=-1;

	for (int i=0;i<keyFrame.size();++i)
	{
		if (keyFrame[i]<keyframe)
		{
			if (keyFrame[i]>=lastFrame)
			{
				lastFrame=keyFrame[i];
				lastIndex=i;
			}
			
		}

		else if (keyFrame[i]>keyframe)
		{
			if (keyFrame[i]<=nextFrame)
			{
				nextFrame=keyFrame[i];
				nextIndex=i;
			}
		}
	}


	preKeyFrame=lastFrame;
	preKeyFrameIndex=lastIndex;
	nextKeyFrame=nextFrame;
	nextKeyFrameIndex=nextIndex;
}

void LayerObject::GetPlanePoint(int frameIndex,Point_3D &p1,Point_3D &p2,Point_3D &p3)
{
	p1=plane[frameIndex*3];
	p2=plane[frameIndex*3+1];
	p3=plane[frameIndex*3+2];
}

int LayerObject::GetKeyFrameIndex(int frame)
{
	for (int i=0;i<keyFrame.size();++i)
	{
		if (keyFrame[i]==frame)
		{
			return i;
		}
	}

	return -1;
}

void LayerObject::SetStartIndex(int index)
{
	startIndex=index;
}

int LayerObject::GetStartIndex()
{
	return startIndex;
}

void LayerObject::SetPolygonManager( CPolygonManager polygonManager )
{
	m_polygonManager = polygonManager;
}

CPolygonManager LayerObject::GetPolygonManager()
{
	return m_polygonManager;
}
