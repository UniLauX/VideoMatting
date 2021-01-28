#pragma once
#include <vector>
#include "../MultiLayer/LayerObject.h"
using namespace std;


class CVEProject
{
public:
	CVEProject(void);
	~CVEProject(void);
	
	void SetStartFrame(int sIndex);
	void SetEndFrame(int eIndex);
	void SetFrameRange(int sIndex,int eIndex);

	void SetFrameCount(int count);
	int GetStartFrame();
	int GetEndFrame();
	int GetFrameCount();
	int GetLayerNum();
	void SavePolygon();
public:
	CString m_sequencePath;
	 vector<LayerObject> vec_layer;// layer object, identified by "name"
	 int layerNum;
	 float m_fDspMin;
	 float m_fDspMax;

private:
	 int startFrame;
	 int endFrame;
	 int frameCount;
	 

};
