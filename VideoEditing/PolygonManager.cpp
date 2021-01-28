#include "PolygonManager.h"

CPolygonManager::CPolygonManager(void)
{
}

CPolygonManager::~CPolygonManager(void)
{
}
bool CPolygonManager::ExistPolygon( int frame_pos )
{
	map< int, CPolygonSet  >::iterator it;
	it = m_PolygonMap.find( frame_pos );
	if ( it != m_PolygonMap.end() )
	{
		return true;
	}
	return false;
}

bool CPolygonManager::GetPolygonSet( int frame_pos, CPolygonSet& polygon_set )
{
	if ( ExistPolygon( frame_pos ) )
	{
		polygon_set = m_PolygonMap[frame_pos];
		for ( int i = 0; i < polygon_set.m_polygon.size(); ++i )
		{
			if ( polygon_set.m_polygon[i].m_vPointSet.size() != 0 )
			{
		return true;
	}
		}
	}
	return false;
}
bool CPolygonManager::AddPolygon( int frame_pos, CPolygonSet& polygon_set )
{
	if ( frame_pos < 0 )
	{
	return false;
}
	map< int, CPolygonSet  >::iterator it;
	it = m_PolygonMap.find( frame_pos );
	if ( it != m_PolygonMap.end() )
	{
		m_PolygonMap.erase( it );
		m_PolygonMap.insert(  pair< int, CPolygonSet >( frame_pos, polygon_set ) );
		return true;
	}
	else
	{
		m_PolygonMap.insert(  pair< int, CPolygonSet >( frame_pos, polygon_set ) );
		return true;
	}
	return false;
}

void CPolygonManager::Clear()
{
	m_PolygonMap.clear();
}