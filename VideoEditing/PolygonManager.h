#pragma once
#include "stdafx.h"
#include "Polygon.h"

class CPolygonManager
{
public:
	CPolygonManager(void);
	~CPolygonManager(void);
	bool ExistPolygon( int frame_pos );
	bool GetPolygonSet( int frame_pos, CPolygonSet& polygon_set );
	bool AddPolygon( int frame_pos, CPolygonSet& polygon_set );
	void Clear();
public:
	map< int, CPolygonSet > m_PolygonMap;

};
