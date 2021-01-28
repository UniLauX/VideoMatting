// VideoEditingDoc.h : interface of the CVideoEditingDoc class
//
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#ifndef CVIDEOEDITINGDOC_H
#define CVIDEOEDITINGDOC_H

#include "../ClosedFormMatting/Matting.h"
#include "../GrabCut/Cutting.h"
#include "../include/CxImage/ximage.h"
#include "../VideoCut/Contour.h"
#include "../MotionEstimate/MotionEstimate.h"
#include "../grab/Grabcut.h"
#include "UserScribble.h"
#include "../VideoCut/LocalClassifier.h"
#include "RotoScopeInterface.h"

#define CLOSEDFORM      0
#define GRABCUT         1
#define GLOBAL_UPDATE   0
#define LOCAL_UPDATE    1
#define MAT_FRAME 24//只做一帧的matting
#define MAT_SEQ 25// 做一个序列的matting
#define CLOSEDMAT 20//closed form matting
#define BAYESIAN 21//bayesian matting
#define MAT_PARTIMG 22//将图片分块做matting
#define MAT_WHOLEIMG 23//整张图片做matting

class CVideoEditingDoc : public CDocument
{
protected: // create from serialization only
	CVideoEditingDoc();
	DECLARE_DYNCREATE(CVideoEditingDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CVideoEditingDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
     int  mode;
     int  updateMode;
	 int mattingType;//matting_frame or Matting Sequence
	 int mattingMode;//whole img or part img
	 int mattingAlgo;//closed form or bayesian

	 bool Opened;                      
	 bool cal;
	Contour contour;


private:

	int videoWinSize;
	int updateWinSize;
	int iterateTime;
	CRect rect;// For grabcut
	CxImage image;
	CxImage scrbImage;// For closedform matting
	CxImage alpha;
	CxImage trimap;// For trimap editing
	CxImage lastImage;
	Matting matting;
	Cutting cutting;
	Grabcut grab;   
	UserScribble scrbLocal;
	MotionEstimate motion;
	RotoScopeInterface * roto;

private:
	void cropRect(RECT &rect,CRect &rectangleRegion);
public:
     void AdjustPos(int &x, int &y, RECT &region);
     void New();
     void Run(CxImage &result, CxImage &alpha);
     void UpdateImages(CxImage *image);
     void GetResult(CxImage &result, CxImage &alpha);
     
     void SetCutRegion(int x1, int y1, int x2, int y2, RECT &region);
     void DrawPoint(int x, int y, int radius, RECT &region, int mode);
     void DrawRectangle(int x1, int y1, int x2, int y2, RECT &region, int mode);
     void DrawLine(int x0, int y0, int x1, int y1, int radius, RECT &region, int mode);
     
     void SetParameter(int level, int activelevel, int winsize, double alphath, double epsilon);
     void GetCFParameter(int &level,int &activeLevel,int &winsize,double &thralpha,double & epsilon);
     void SetVideoWinSize(int vWinSize);
     void SetUpdateWinSize(int uWinSize);
     void SetIterateTime(int time);
     void SetLocalClassifierParam(CLASSIFIERPARAM &param);
     void GetLocalClassifierParam(CLASSIFIERPARAM &param);

     void InitVideoFirstFrame();
     bool InitVideoFirstFrame(CxImage &image,unsigned char *label);
     void RunVideo(CxImage &result, CxImage &alpha);
	 void RunVideo(CxImage &result, CxImage &alpha, CString path, int frame);
     void RunMatting(CxImage &result);
     void RunMatting(CxImage &result, CxImage &pic, CxImage &label, CxImage& trimap, CxImage& fgimg);
     void RestoreFrame(CxImage *frame, CxImage *alpha);

     void SetTrimap(CxImage &trimap);
     void GetTrimap(CxImage &trimap);
     void GetTrimap(CxImage &trimap, int dis);
     int GetWidth();
     int GetHeight();
     afx_msg void OnSettingColor();
     void ResetScribble(void);
	 RGBQUAD GetContourBG();
	 void CheckMemory();


	 //rotoscope
	 void setRotoScopeParameter(int kup, int klow, int startFrameIndex, int endFrameIndex, int pyr_level);


	 //Video SnapCut/ new add
	 RECT FindContourRect(CxImage srcImg,int lclWinSize);

};

#endif