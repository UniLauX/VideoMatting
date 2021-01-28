#ifndef ROTOSCOPEINTERFACE_H
#define ROTOSCOPEINTERFACE_H

#include <iostream>
#include <vector>
#include <set>
#include "simpleimage.h"
#include "ImageBuffer.h" 
#include "BeizerCurveDataManager.h"
#include "../RotoScoping/RotoScoping.h"

using namespace std;
class CxImage;
class BeizerCurveData;
class Beizer;
class ImageSequenceManager;
struct CvMat;


struct extremes
{
	bool IsMinimal;//�Ƿ�Ϊ��Сֵ
	int row;
	int col;
	int pointNum;
	//int orient;
};




class RotoScopeInterface
{
public:
	RotoScopeInterface();
	~RotoScopeInterface();
	void startRotoscope();
	void loadImage(CxImage * image);
	void loadControlPointAndMask(BeizerCurveDataManager & curveData, int ta, int tb);
	CSimpleImageb * CxImage2SimpleImage(CxImage * cximage);
	void initializeRotoscopeData(int fileRange, int imageHeight, int imageWidth, CString& pathname, 
		                         CString & extname, int startVedioIndex);
	static void setRotoScopeParameter(int kup, int klow, int startFrameIndex, int endFrameIndex, int pyr_level);
	void setAlphaSmoothParameter(int ls, int lb, float bmin, float bmax);
	void loadRotoScopeData();
	void generateMaskBoundary(int index);
	void generateMaskRegion(int index, vector<CPoint>& seedPoint);
	void generateMaskRegion(int index );
	void alphaSmooth(int index);
	void releaseMaskImage(int index);
	void showMaskImage(int index);
	void getResult();
	CxImage * getMaskImage(int index);


private:
	//rotoscoping
	float** x;
	float** y;
	int numofFrame;
	int numofCurve;
	int oldnf;
	int oldnc;

	float ** rx;
	float ** ry;

	bool ** mask; //�����Щ�㱻�û�������

	vector<CSimpleImageb*> pImage;

	//rotoscoping parameter
	static int kup;
	static int klow;
	static int startFrameIndex;
	static int endFrameIndex;
	static int pyramidLevel;

	//alphasmooth parameter
	static int Lb;
	static int Ls;
	static float bMin;
	static float bMax;



	//data ȫ����interface�г�ʼ��
	int imageHeight;
	int imageWidth;
	BeizerCurveDataManager CurveManager;
	ImageSequenceManager * imageSequence;
	
	
	CSimpleImageb ** maskImage; //�洢mask��ֻ���������洢���м�ý�飬�����ڱ�������
	CSimpleImageb ** maskImageAfterSmooth;
	CvMat * MaskMat;
	int * keyFrameFlag;
	vector<bool> * keyFrameUpdateFlag;
	int totalNumofImage;
public:
	inline BeizerCurveDataManager* getDataManager(){return &CurveManager;}
	inline ImageSequenceManager* getImageSquenceManager(){return imageSequence;}
	inline CvMat* getMaskMat(){return MaskMat;}
	inline int * getKeyFrameFlag(){return keyFrameFlag;}
	inline vector<bool> * getKeyFrameUpdateFlag(){return keyFrameUpdateFlag;}

	

};
#endif