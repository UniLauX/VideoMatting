// VideoEditingView.h : interface of the CVideoEditingView class
//

#ifndef VIDEOEDITINGVIEW_H
#define VIDEOEDITINGVIEW_H

#include "ImageBuffer.h"
#include "PolygonManager.h"
#include "../include/CxImage/ximage.h"
#include "atltypes.h"
#include "GridLayout.h"
#include "ImageEditView.h"
#include <afxmt.h>
#include "../MultiLayer/LayerObject.h"
#include "../Bayesianmatting/ProImage.h"
//#include "TemporalRefiner.h"
#include "MultiFrameRefine.h"
#include "ZImage.h"

class RotoScopeInterface;

#define EDITVIEW    0
#define RESVIEW     1
#define PICVIEW     2
#define  DEPTHVIEW 3
#define TRIMAPVIEW 4 //定义为trimapview，可以及时判断当前视图类型

#define GRAB        10
#define MOVE        11
#define DRAW        12
#define GRABUPDATE  13
#define NOTOOL      14


/**********************************************
for sequence:D:\data\qxs\test0000.jpg 
variable values are:
filename: test
filepath: D:\data\qxs\
backGroundLayerPath: D:\data\qxs\test
editLayerPath: D:\data\qxs\VEData\
***********************************************/

class CVideoEditingView : public CView
{
protected: // create from serialization only
	CVideoEditingView();
	void InitMember();
	DECLARE_DYNCREATE(CVideoEditingView)

// Attributes
public:
	CVideoEditingDoc* GetDocument() const;

// Operations
public:
	 int fileRange;   //num of frames
// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CVideoEditingView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	//added func for tool manage
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
private:
     void EditView(CMemDC *pDC);
     void InitRegion();

// Render the video
public:

         void    Idle();
         LRESULT OnRender(WPARAM wParam, LPARAM lParam);
	 //set parameter
	 void setResultPath(CString rp);
	 void setAlphaPath(CString  ap);
	 void setTrimapPath(CString tp);
	 void setMaskImagePath(CString tp);
	 
	 CString GetFilePath() const;
	 void setProcess();
	 void calstop();
	 int getStartIndex();
	 ImageBuffer      imageBuffer;                // image buffer
	 ImageBuffer      alphaBuffer;//for getting the polygon
	 ImageBuffer      resImageBuffer;
	 CPolygonManager   polygonManager;              // polygon buffer for UI

	 float m_fTriMpAlpha;
	 enum TriMapViewMode {TRIMAP = 0, RESULT};
	 int m_TriMpVwMode;

	 RotoScopeInterface * rotoScopeInterface;
	 bool b_IsRotoScopingReadData;
	 bool b_IsClosed;

private:
     
     CString resultPath;  // Path
     CString alphaPath;
     CString trimapPath;
	 CString maskImagePath;
     CString mattingAlpPath;
     CString imageExt;  //file extension
     CString alphaExt;
     CString trimapExt;
	 CString maskImageExt;


     
     ImageBuffer      trimapBuffer;
     ImageBuffer      layerBuffer;
	 ImageBuffer      maskImageBuffer;
     COLORREF         bgColor;                    // background color

     GridLayout *     layout;
     GridLayout       gridLayout;
     GridLayout       gridLayoutTrimap;
     ImageEditView *  imageEditView;
     ImageEditView *  imageResultView;
     ImageEditView *  trimapView;

     // closedform parameters
     int              level;
     int              alevel;
     double           ep;
     double           alphath;
     int              erodWinSize;


	 // Mode
	 int              mode;        // Edit mode
	 bool           hasMoved;    // Frame
	 int              editTool;

	 // Move
	 bool             hasStartPos;

	 CEvent           exitThread;
	 CEvent           finishExiting;
	 CEvent           mutex;

	 int startIndex;                //first file index
	 CString imagePath;//full path name;
	 int trimapWidth;

	 //mouse statuc
	 bool rButtonDown;
	 bool lButtonDown;

	 CString filePath;

	 ProImage pi;//matting

/************************************************************************/

	// structure of multilayer
    vector<LayerObject> vec_layer;// layer object, identified by "name"
	int current_layer;//index of vec_layer
	CString current_layerName;

	CString editLayerPath;
	CString filename;
	bool backGroundLayerSelected;
	bool editLayerSelected;
	bool firstLayerCreated;
	bool layerGenerated;
	bool allLayerGenerated;

	//depth calculation structure
	bool calDepthStarted;
	Point_3D plane_point[3];
	int z_depth;
	float m_fDspMin;
	float m_fDspMax;

	CString GetName();
	BOOL DeleteFolder(LPCTSTR lpszPath);//must delete VEData folder
	void GetPlaneParam(Point_3D p1,Point_3D p2,Point_3D p3, double &a,double &b,double &c,double &d);//calculate plane param
	void AddKeyFrame(int pos,Point_3D p1,Point_3D p2,Point_3D p3);//key frame for depth calculation
	void calDepth(CxImage &label,CxImage &depthImage,CxImage &alpha, CxImage& layerdepth,double &a,double &b,double &c,double &d);
	void calDepth(CxImage &label,CxImage &depthImage,CxImage &alpha, CxImage& layerdepth,Point_3D p1,Point_3D p2,Point_3D p3);
	void CalDepth2(CxImage &LabelMsk, CxImage &DepthImg, ZFloatImage& DspImg, CxImage &AlphaImg, CxImage &LyrDepth, ZFloatImage& LyrDsp, double &a, double &b, double &c, double &d);
	void GetNextFramePoints(CString firstFramePath,CString secondFramePath,Point_3D &p1,Point_3D &p2,Point_3D &p3);//feature points propagation
	//void CalculateDepthMap();
	void CalculateDepthMap(int startFrame,int startFrameIndex,int endFrame,int endFrameIndex);
	
	
public:
	CString backGroundLayerPath;
	bool clickToStop;
	void SetLayerIndex(CString name);//set current_layer according current_layerName
	void SavePolygonManager( int layer_index );
	void saveMaskImage(int startIndex, int endIndex, int lb, int ls, float bmin, float bmax);
	void sendRotoReadDataPara(bool close);
	int GetCurrentLayerNum();
	void UpdateLayerInfo(CString oldname,CString newname,int newDepth);
	bool AddLayer(CString name,int depth);
	void UpdateImageBuffers(CString layerName);
	void GetEditLayer(CxImage* frame,CxImage* label,CxImage &layer);
	CString GetEditLayerPath();
	void SetLayerPos(int p);
	void  GenerateImage(CxImage &im);
	void ReUpdateResult(CxImage &resultIm,CxImage &alphaIm);
	void BeginCalDepth(bool isAllLayerGenerated,bool isCalDepthStarted);
	void SetZDepth(int d);
//end of structure multilayer
/*************************************************************************/
     void Initial();
     void RestoreFrame(int pos);//when calculate thread stop ,we must restore some data
     void SetRLTPath(CString path);//set location of result label and trimap
     void SetRLTPath(CString path,CString layerName);//layer respectively
     //void Composite(CxImage &dest, CxImage &source, CxImage &alpha);
	 void Composite(CxImage &dest, CxImage &source, CxImage &alpha, float blend = 0.1);
	 int CheckTrimapExist();
	 int CheckLabelExist();


	 void RecompositeTrimap(void);
	 bool GenerateVEDepthMaps(void);

private:

     friend UINT RunVideo(LPVOID param);

     afx_msg void OnFileOpen();
     afx_msg BOOL OnEraseBkgnd(CDC* pDC);
     afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
     afx_msg void OnMouseMove(UINT nFlags, CPoint point);
     afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
     afx_msg void OnGrabberRectanglegrab();
     afx_msg void OnGrabberForegroundbrush();
     afx_msg void OnGrabberBackgroundbrush();
     afx_msg void OnUpdateGrabberRectanglegrab(CCmdUI *pCmdUI);
     afx_msg void OnUpdateGrabberForegroundbrush(CCmdUI *pCmdUI);
     afx_msg void OnUpdateGrabberBackgroundbrush(CCmdUI *pCmdUI);
     afx_msg void OnViewEditwindow();
     afx_msg void OnViewResultwindow();
     afx_msg void OnUpdateViewEditwindow(CCmdUI *pCmdUI);
     afx_msg void OnUpdateViewResultwindow(CCmdUI *pCmdUI);
     afx_msg void OnRunRun();
     afx_msg void OnModePicture();
     afx_msg void OnModeVideo();
     afx_msg void OnUpdateModePicture(CCmdUI *pCmdUI);
     afx_msg void OnUpdateModeVideo(CCmdUI *pCmdUI);
     afx_msg void OnGrabberGrabcut();
     afx_msg void OnGrabberClosedformmatting();
     afx_msg void OnUpdateGrabberGrabcut(CCmdUI *pCmdUI);
     afx_msg void OnUpdateGrabberClosedformmatting(CCmdUI *pCmdUI);

public:
	 afx_msg void OnSettingPreference();
     afx_msg void OnEditMove();
     afx_msg void OnUpdateEditMove(CCmdUI *pCmdUI);
     afx_msg void OnEditFit();
     afx_msg void OnEditZoomin();
	 afx_msg void OnAddNewLayer();
     afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
     afx_msg void OnEditZoomout();
     afx_msg void OnRunStop();
     afx_msg void OnRunRunvideo();
     afx_msg void OnUpdateGlobalupdate();
     afx_msg void OnUpdateLocalupdate();
     afx_msg void OnUpdateUpdateGlobalupdate(CCmdUI *pCmdUI);
     afx_msg void OnUpdateUpdateLocalupdate(CCmdUI *pCmdUI);
     afx_msg void OnPenForeground();
     afx_msg void OnPenBackground();
     afx_msg void OnUpdatePenForeground(CCmdUI *pCmdUI);
     afx_msg void OnUpdatePenBackground(CCmdUI *pCmdUI);
     afx_msg void OnUpdateRunRun(CCmdUI *pCmdUI);
     afx_msg void OnUpdateRunStop(CCmdUI *pCmdUI);
     afx_msg void OnUpdateRunRunvideo(CCmdUI *pCmdUI);
     afx_msg void OnUpdateEditFit(CCmdUI *pCmdUI);
     afx_msg void OnUpdateEditZoomin(CCmdUI *pCmdUI);
     afx_msg void OnUpdateEditZoomout(CCmdUI *pCmdUI);
     afx_msg void OnRunMatting();
     afx_msg void OnUpdateRunMatting(CCmdUI *pCmdUI);
     bool fgBrushDown;
     bool bgBrushDown;
     afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
     afx_msg void OnSettingLocalwin();
     afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
     afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
public:
    afx_msg void OnViewTrimap();
public:
    afx_msg void OnUpdateViewTrimap(CCmdUI *pCmdUI);
    afx_msg void OnRunGetfinallayer();
    afx_msg void OnViewDepth();
    afx_msg void OnUpdateViewDepth(CCmdUI *pCmdUI);
    afx_msg void OnFileOpenproject();
    afx_msg void OnFileSaveProject();
    afx_msg void OnUpdateRunGetfinallayer(CCmdUI *pCmdUI);
    afx_msg void OnRunRefine();
	ImageEditView* GetImageEditingView()const;
	void AdjustPos( int& x, int& y, RECT& rect );
	void AdjustPointToView( CPoint& point, RECT& rect );
	afx_msg void OnPreprocessOpticalflow();
	afx_msg void OnHelpTestfillholes();
	private:
		void SaveOpticalIm(float* flow, int width, int height, char* name);

	afx_msg void OnPenUnknown();
	afx_msg void OnUpdatePenUnknown(CCmdUI *pCmdUI);

public:
	afx_msg void OnHelpDepthcomposition();
	void SoftDepth(CxImage& depth, CxImage& label, CxImage& alpha, CxImage& newdepth);
	afx_msg void OnRunTemporalmatting();
	afx_msg void OnPostprocessLayeropticalflow();

	afx_msg void OnRefineTrimap();
	afx_msg void OnRunDefaultLayerDepth();
	afx_msg void OnRunUnknownOpticalFlow();
	afx_msg void OnRunRotoscoping();
	afx_msg void RotoScopingCurveImport();
	afx_msg void RotoScopingCurveExport();
};

#ifndef _DEBUG  // debug version in VideoEditingView.cpp
inline CVideoEditingDoc* CVideoEditingView::GetDocument() const
   { return reinterpret_cast<CVideoEditingDoc*>(m_pDocument); }
#endif

#endif
