#pragma once
#include <vector>
#include "../VideoEditing/stdafx.h"
#include "../VideoEditing/PolygonManager.h"

using namespace std;



typedef struct np
{
	int x;
	int y;
	int z;

} Point_3D;
enum LayerType{STATIC_PLANE=0, STATIC_RIGID, 
PLANE_MOTION, RIGID_MOTION,NONRIGID_MOTION};
class LayerObject
{
public:
	LayerObject(void);
	~LayerObject(void);
	void SetName(CString name);
	CString GetName();
	void SetDepth(int depth);
	int GetDepth();
	void SetType(int type);
	int GetType();
	void SetDemision(int width,int height);
	int GetWidth();
	int GetHeight();
	void SetLabel(const int* label,int width,int height);
	int* GetLabel();
	int GetPos();
	void SetPos(int p);
	int IsKeyFrame(int pos);
	void AddKeyFrame(int pos);
	void AddPlanePoint(Point_3D p1,Point_3D p2,Point_3D p3);
	void UpdateKeyFrame(int index,Point_3D p1,Point_3D p2,Point_3D p3);

	int NumOfKeyFrame();
	
	void GetNeighbourOfKeyFrame(int keyframe,int &preKeyFrame,int &preKeyFrameIndex,
		int &nextKeyFrame,int &nextKeyFrameIndex);//preKeyFrame: last keyFrame in the sequence
	void GetPlanePoint(int frameIndex,Point_3D &p1,Point_3D &p2,Point_3D &p3);

	int GetKeyFrameIndex(int frame);
	void SetStartIndex(int index);
	int GetStartIndex();
	void SetPolygonManager( CPolygonManager polygonManager );
	CPolygonManager GetPolygonManager();
						
	int startIndex;//start frame index

	vector<int> keyFrame;//record  key frame
	vector<Point_3D> plane;//record points,3 points for one plane

private:
	int id;
	CString name;
	int depth;
	int layerType;
	int width;
	int height;
	int *label;
	int pos;
	CPolygonManager m_polygonManager;

};
