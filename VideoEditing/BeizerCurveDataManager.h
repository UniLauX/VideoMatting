#ifndef BEIZERCURVEDATAMANAGER_H
#define BEIZERCURVEDATAMANAGER_H

#include "Beizer.h"
#include <vector>
#include "stdafx.h"
using namespace std;

class Beizer;

class ControlPoint // for Rotoscoping
{
public:
	ControlPoint():cp(CPoint(0,0)), bIsInteractive(false){}
	ControlPoint(CPoint& p): cp(p), bIsInteractive(false){}
	ControlPoint(CPoint& p, bool everInteractive): cp(p), bIsInteractive(everInteractive){}
	LONG x() const{return cp.x;}
	LONG y() const{return cp.y;}
	operator CPoint() const{return cp;}

	CPoint cp;
	bool bIsInteractive;
};

class BeizerCurveData
{
public:
	BeizerCurveData();
	~BeizerCurveData();
	void addPoint(CPoint& p, bool inter = false);
	int getNumofCurve() const;
	ControlPoint& getPoint(int index);
	int searchClosestPoint(CPoint& p);
	float disntance2(CPoint& p1, CPoint& p2);
	void getNumofDisplayPoint(int * num);
	int getNumofPoint( ) const;
	void updatePoint(CPoint& p, int index, bool inter = false);
	void insertPoint(CPoint& p, int index, bool inter = false);
	void insertThreePoint(double index, double preLength, double laterLength, bool inter);
	void copyData(BeizerCurveData& curveData);
	void pop_backPoint();
	void setCurveColsed();
	void setCurveUnColsed();

private:
	vector<ControlPoint> conPoint;//将各段曲线的控制点连续存储，对于相邻两段的控制点只存储一次。另外首尾各多出一个点。
	int numofCurve;
	bool m_bIsClosed;

};

class BeizerCurveDataManager
{

public:
	BeizerCurveDataManager();
	~BeizerCurveDataManager();
	void initilaizeCurveData(int num);
	BeizerCurveData& getCurveData(int index);
	void simplePropagate(int begin, int end, int key);
	void insertPointPropagate(double index, double preLength, double laterLength, int keyFrame);
	void setCurvesClosed();
	bool isClosed();
	int getNumOfCurves() const;
	void distroyCurveData();


private:
	BeizerCurveData * curveData;
	int numofCurves;//和帧数是相等的。
	bool m_bIsClosed;
	bool m_ballinitialized;
};

#endif