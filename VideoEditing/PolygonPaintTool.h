#pragma once
#include "stdafx.h"
#include "CTool.h"
#include "VideoEditingView.h"

namespace VideoActiveTool{

	enum POINTTYPE
	{
		PointOnPoint,
		PointOnLine,
		PointInPolygon,
		UnknownPoint
	};

	class CPolygonPaintTool:
		public CTool
	{
	public:
		CPolygonPaintTool(void);
		~CPolygonPaintTool(void);
		//for new
		virtual CTool* Clone() const;
		virtual const std_string GetToolClassName() const{return std_string(_T("Polygon Paint tool"));};	
		//added function for control the tool
		virtual int WindowProc(CWnd * pWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		virtual bool Activate();
		virtual void Deactivate();

	private:
		//added func for drawing polygon
		CPoint MakePoint(LPARAM lparam);
		POINTTYPE DetectPointType( const CPoint &point );
		//to generate the vector of edge according to the point set
		void PointToEdge();
		//calculate the distance between two point
		float DisPointToPoint( const CPoint& p1, const CPoint& p2 );
		//get the rect region containing all the point
		//void GetRectRegion();
		unsigned int GetLineIndex( const CPoint& point );
		unsigned int GetPointIndex( const CPoint& point );
		CPoint EraseLineLinkToPoint( const CPoint& point );
		//added func for muti frame
		void ResetState();
		void ChangePolygon();
		//added func for draging the polygon
		void MovePolygon( int x, int y );

		void LButtonDownDraw( LPARAM lParam );
		void LButtonDownAdjust( LPARAM lParam );
		void AdjustPointOnPoint( int point_index );
		void AdjustPointOnLine( int line_index );
		void LButtonUp();
		void LButtonDBLCLK( LPARAM lParam );
		void MouseMove( LPARAM lParam );
		void MouseWheel();
		int DectectPolygonIndex( const CPoint& point );
		bool PtInPolygon( CPoint point, const vector<CPoint>& point_set );
		bool PtOnPolygon( CPoint point, const vector<CPoint>& point_set, const vector< pair<CPoint, CPoint> >& edge_set, int point_radius = 0, float line_diff = 0.f );
		int GetRightMostX( vector<CPoint>& point_set );
		CPoint GetPolygonLeftBottomPoint( vector<CPoint>& point_set );
		CPoint GetPolygonRightTopPoint( vector<CPoint>& point_set );
		void SaveImageFromPolygon();
		void UpdatePolygonByFlow();
		float MeanOpticalFlow( int x, int y, float* a, int radius );
		void ContourBoundRefine( CxImage* color_image, unsigned char* label );
		void GetLabelFromPolygon( unsigned char* label );
		bool GetPolygonFromMask( const int& k );
		void GetPolygonFromPrevious( int k );
		//added data member for drawing polygon 
	public:
		static int m_iPointRadius;//the deviation on point
		static float m_fLineDiff;//the deviation on line
		static double m_fCosThreshold;//threshold for genrating the polygon
		static double m_fDisThreshold;
	private:
		CPolygonSet m_cPolygonSet;
		CPolygon* m_cPolygon;
		bool m_bIsMouseDown;//to control the mouse move response
		bool m_bIsDrawingPolygon;//to control draw the polygon
		bool m_bIsAdjustingPolygon;//to control adjust the polygon
		bool m_bIsDragPolygon;//to control drag the polygon
		bool m_bIsDragImage;//to control drag the image
		CPoint prev_Point,curr_Point;//to achieve the effect of drag a line
		//for adjusting the polygon
		CPoint mobile_point, fixed_point1, fixed_point2;
		int m_iPointIndex;
		CRect bound_rect;
		int m_iCurrentFrame;//for multi frame
		int m_iCurrentLayer;
		int m_iCurrentPolygon;
		CVideoEditingView *pView;
		ImageEditView *imageView;
	};
}

