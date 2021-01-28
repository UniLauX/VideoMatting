#include "BeizerCurveDataManager.h"
#include "Beizer.h"
#include <cmath>
#include <iostream>
using namespace std;

#define SDIS 5


BeizerCurveDataManager::BeizerCurveDataManager()
{
	curveData = new BeizerCurveData;
	numofCurves = 0;
	m_bIsClosed = false;
	m_ballinitialized = false;
}

BeizerCurveDataManager::~BeizerCurveDataManager()
{

	cout << "~BeizerCurveDataManager" << endl;
	if (curveData != NULL)
	{
		delete[] curveData;
	}
	curveData = NULL;
}


void BeizerCurveDataManager::initilaizeCurveData( int num )
{
	numofCurves = num;
	curveData = new BeizerCurveData[num];
	cout << "initilaizeCurveData over!" << endl;
}

BeizerCurveData& BeizerCurveDataManager::getCurveData( int index )
{
	int s = curveData[index].getNumofPoint();
	//cout << "flag 2"  << s << endl;
	return curveData[index];
}

void BeizerCurveDataManager::simplePropagate(int begin, int end, int key)
{
	for (int i = begin; i <= end; i++)
	{
		if (i != key)
		{
			getCurveData(i).copyData(getCurveData(key));
		}
	}
	m_ballinitialized = true;
}

void BeizerCurveDataManager::setCurvesClosed()
{
	if (getCurveData(0).getNumofCurve() <= 0)
	{
		return;
	}
	for (int i = 0; i < numofCurves; i++)
	{
		getCurveData(i).setCurveColsed();
	}
	m_bIsClosed = true;

}

bool BeizerCurveDataManager::isClosed()
{
	return m_bIsClosed;
}

void BeizerCurveDataManager::insertPointPropagate( double index, double preLength, double laterLength, int keyFrame )
{
	if (!m_ballinitialized)
	{
		return;
	}
	for (int i = 0; i < numofCurves; i++)
	{
		if ( i == keyFrame)
		{
			continue;
		}
		
		getCurveData(i).insertThreePoint(index, preLength, laterLength, false);
	}
}

int BeizerCurveDataManager::getNumOfCurves() const
{
	return numofCurves;
}

void BeizerCurveDataManager::distroyCurveData()
{
	if (curveData != NULL)
	{
		delete[] curveData;
	}
	curveData = NULL;
}



BeizerCurveData::BeizerCurveData()
{
	conPoint.clear();
	numofCurve = 0;
	m_bIsClosed = false;
}

BeizerCurveData::~BeizerCurveData()
{
	cout << "~BeizerCurveData" << endl;
	//conPoint.clear();
}


void BeizerCurveData::addPoint( CPoint& p, bool inter )
{
	conPoint.push_back(ControlPoint(p,inter));
	//cout << "conPoint.size()" << conPoint.size() << endl;
	numofCurve = (conPoint.size() - 2) / 3 + m_bIsClosed;
	//cout << numofCurve << endl;
}

int BeizerCurveData::getNumofCurve() const
{
	return numofCurve;
}

ControlPoint& BeizerCurveData::getPoint( int index )
{
	if (index > conPoint.size() )
	{
		cout << "out of range! can not get point." << endl;
	}
	return conPoint[index];
}

int BeizerCurveData::searchClosestPoint( CPoint& p )
{
	for (int i = 0; i< conPoint.size(); i++)
	{
		if (disntance2((CPoint)conPoint[i], p) < SDIS * SDIS)
		{
			return i;
		}
	}
	return -1;
}

float BeizerCurveData::disntance2( CPoint& p1, CPoint& p2 )
{
	return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

void BeizerCurveData::getNumofDisplayPoint( int * num )
{
	for (int i = 0; i < numofCurve; i++)
	{
		num[i] = 10;
	}
}

int BeizerCurveData::getNumofPoint() const
{
	//cout << conPoint.size() << endl;
	return conPoint.size();
}

void BeizerCurveData::updatePoint( CPoint& p, int index, bool inter)
{
	if (index > conPoint.size() )
	{
		cout << "out of range! can not update point." << endl;
	}
	conPoint[index].cp.x = p.x;
	conPoint[index].cp.y = p.y;
	conPoint[index].bIsInteractive = inter;
	
}

void BeizerCurveData::insertPoint( CPoint& p, int index, bool inter )
{
	if (index > conPoint.size() )
	{
		cout << "out of range! can not insert point." << endl;
	}
	conPoint.insert(conPoint.begin() + index, ControlPoint(p, inter));
	numofCurve = (conPoint.size() - 2) / 3 + m_bIsClosed;
}

void BeizerCurveData::copyData( BeizerCurveData& curveData )
{
	conPoint.clear();
	for (vector<ControlPoint>::iterator iter = curveData.conPoint.begin(); iter != curveData.conPoint.end(); iter++)
	{
		CPoint p = *(iter);
		conPoint.push_back(ControlPoint(p, false));
	}
	numofCurve = curveData.numofCurve;
}

void BeizerCurveData::pop_backPoint()
{
	conPoint.pop_back();
	numofCurve = (conPoint.size() - 2) / 3 + m_bIsClosed;
}

void BeizerCurveData::setCurveColsed()
{
	m_bIsClosed = true;
	numofCurve  = (conPoint.size() - 2) / 3 + m_bIsClosed;
}

void BeizerCurveData::setCurveUnColsed()
{
	m_bIsClosed = false;
	numofCurve = (conPoint.size() - 2) / 3 + m_bIsClosed;
}

void BeizerCurveData::insertThreePoint( double index, double preLength, double laterLength, bool inter )
{
	//cout << "insert 3 " << endl;
	if (index > conPoint.size() + 1)
	{
		cout << "insert three point failed!(propagate)" << endl;
		//cout << index << endl;
	}
	int int_Index = int(index);
	
	Beizer * beizer;
	beizer = new Beizer;
	//cout << index << endl;
	//cout << "conPoint.size()" << conPoint.size() << endl;
	int pointIndex = int_Index * 3 + 1;
	beizer->addControlPoint(conPoint[pointIndex].x(), conPoint[pointIndex].y());
	beizer->addControlPoint(conPoint[pointIndex + 1].x(), conPoint[pointIndex + 1].y());

	pointIndex = (pointIndex + 2) % conPoint.size();
	beizer->addControlPoint(conPoint[pointIndex].x(), conPoint[pointIndex].y());
	beizer->addControlPoint(conPoint[pointIndex + 1].x(), conPoint[pointIndex + 1].y());
	
	double preindex   = max(index - int_Index - 0.05, 0);//前面一个点的索引,用于计算斜率
	double laterindex = min(index - int_Index + 0.05, 1);//后一个点的索引，用于计算斜率

	ControlPoint precp   = beizer->getPoint(preindex);
	ControlPoint latercp = beizer->getPoint(laterindex);

	double dis = sqrt((double)((precp.x() - latercp.x()) * (precp.x() - latercp.x()) + (precp.y() - latercp.y()) * (precp.y() - latercp.y())));
	double ex  = (double)(latercp.x() - precp.x()) / dis;
	double ey  = (double)(latercp.y() - precp.y()) / dis;

	CPoint cp    = beizer->getPoint(index - (double)int_Index);
	CPoint pcp;
	pcp.x = cp.x - preLength * ex;
	pcp.y = cp.y - preLength * ey;
	CPoint lcp;
	lcp.x = cp.x + laterLength * ex;
	lcp.y =cp.y + laterLength * ey;

	insertPoint(lcp, 3 * int_Index + 3, inter);
	insertPoint(cp, 3 * int_Index + 3, inter);
	insertPoint(pcp, 3 * int_Index + 3, inter);
	delete beizer;

}

