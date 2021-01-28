#pragma once


#include "stdafx.h"
#include "CTool.h"
#include "VideoEditingView.h"

#include "BeizerCurveDataManager.h"
#include <set>

class RotoScopeInterface;
class Beizer;
class CvMat;
class ImageSequenceManager;

namespace VideoActiveTool{

	class CBeizerCurvePaintTool : public CTool
	{
	public:
		CBeizerCurvePaintTool(void);
		~CBeizerCurvePaintTool(void);
		virtual CTool* Clone() const;
		virtual const std_string GetToolClassName() const{return std_string(_T("BeizerCurve Paint tool"));};	
		//added function for control the tool
		virtual int WindowProc(CWnd * pWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		virtual bool Activate();
		virtual void Deactivate();


		void setDraw();
		CPoint MakePoint(LPARAM lparam);
		void setLeftButton();
		void leftButtonDown(LPARAM lParam);
		void mouseMove(LPARAM lParam);
		void leftButtonUp(LPARAM lParam);
		void leftButtonDoubleClick();
		bool isMovingPoint(LPARAM lparam);
		void ajudstControlPointTanslation(LPARAM lparam); //ƽ����������
		void ajustControlPointoffLine(LPARAM lparam, float distance);
		float seachClosestPoint(CPoint& p, float maxdis);
		void rotoScope();
		void rotoScope(int ta, int tb);
		void initializeRotoScopeParameter(RotoScopeInterface * rotointerface, int ta, int tb);
		void insertPointPropagate(double length);
		void addInteractiveMask(int i);
		void updateInteractiveMask(int form, int cur);
		void insertMask2InteractiveMask(int index);






		

		
		

	private:

		

		bool m_bIsDrawBeizer;
		bool m_bisDrawing;
		bool m_bIsLeftbuttonDown;
		bool m_bIsMovingPoint;
		bool m_bIsDrawingDirect;
		bool m_bIsInsertPoint;
		bool m_bNeedInitRotoscope;
		bool m_bNeedRotoScope;
		bool m_bIsNeedInitialize;
		bool m_bIsDrawOver;//�պ����л�ͼ���Ǳ��� ����������ϣ����ǿ��Լ�������
		
		CVideoEditingView *pView;
		ImageEditView *imageView;
		CPoint curr_Point;
		CPoint prev_Point;
		CPoint start_Point;


		float insertPointIndex;
		int insertCurveId;

		int movePointId;
		int movePointFlag;
		int curruntFrameIndex;
		int * keyFrameFlag;//��������
		vector<bool> * keyFrameUpdateFlag;//��������

		BeizerCurveDataManager * CurveManager;//��������
		Beizer * beizer;
		RotoScopeInterface * rotoScopeInterface;
		ImageSequenceManager * imageSequence;//��������

		float distance;//��ʱ����
		CvMat * MaskMat; //�����ϵ��mask���������أ�mask�����������ߵ�index��
		//��������

		set<int> * interactiveMask;

	};
}