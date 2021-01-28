
#include "Polygon.h"

CPolygon::CPolygon(void)
{
}

CPolygon::CPolygon( vector<CPoint>& polygon_point )
{
	m_vPointSet = polygon_point;
}

CPolygon::~CPolygon(void)
{
}

void CPolygon::Clear()
{
	m_vPointSet.clear();
	m_vEdgeSet.clear();
}

CPolygonSet::CPolygonSet(void)
{
}
CPolygonSet::~CPolygonSet(void)
{
}

void CPolygonSet::Clear()
{
	m_polygon.clear();
}
