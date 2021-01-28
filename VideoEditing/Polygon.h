#pragma once
#include "stdafx.h"

class CPolygon
{
public:
	CPolygon(void);
	CPolygon( vector<CPoint>& polygon_point );
	~CPolygon(void);
	void Clear();
public:
	int m_iFramePos;
	vector<CPoint> m_vPointSet;
	vector< pair<CPoint, CPoint> > m_vEdgeSet;
};
class CPolygonSet
{
public:
	CPolygonSet(void);
	~CPolygonSet(void);
	void Clear();
public:
	vector<CPolygon> m_polygon;
};