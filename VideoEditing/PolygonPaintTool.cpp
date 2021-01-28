#include "PolygonPaintTool.h"
#include "PolygonGenerator.h"
#include <time.h>
namespace VideoActiveTool{
	int CPolygonPaintTool::m_iPointRadius = 2;
	float CPolygonPaintTool::m_fLineDiff = 0.5f;
	double CPolygonPaintTool::m_fCosThreshold = 0.8;
	double CPolygonPaintTool::m_fDisThreshold = 8;

	CPolygonPaintTool::CPolygonPaintTool(void)
	{
		m_bIsMouseDown = false;
		m_bIsDrawingPolygon = false;
		m_bIsAdjustingPolygon = false;
		//m_bIsDragPolygon = false;
		m_bIsDragImage = false;
		m_iCurrentFrame = -1;
		m_iCurrentLayer = -1;
		m_iCurrentPolygon = -1;
	}

	CPolygonPaintTool::~CPolygonPaintTool(void)
	{
		printf("~CPolygonPaintTool\n");
	}
	//virtual func for override
	CTool* CPolygonPaintTool::Clone() const
	{
		return new CPolygonPaintTool;
	}

	bool CPolygonPaintTool::Activate()
	{
		m_bIsDrawingPolygon = true;
		return true;
	}

	void CPolygonPaintTool::Deactivate()
	{
		printf("Deactivate\n");
		m_bIsDrawingPolygon = false;
		if ( !m_bIsDrawingPolygon && !m_cPolygonSet.m_polygon.empty() )
		{
			pView->polygonManager.AddPolygon( m_iCurrentFrame, m_cPolygonSet );
		}
	}

	void CPolygonPaintTool::ResetState()
	{
		m_bIsMouseDown = false;
		m_bIsDrawingPolygon = false;
		m_bIsAdjustingPolygon = false;
		//m_bIsDragPolygon = false;
	}

	int CPolygonPaintTool::WindowProc( CWnd * pWnd, UINT Message, WPARAM wParam, LPARAM lParam )
	{
		//CPoint point=MakePoint(lParam);
		if (pWnd->GetRuntimeClass()->IsDerivedFrom(RUNTIME_CLASS(CVideoEditingView)))
		{
			pView=dynamic_cast<CVideoEditingView*>(pWnd);
			imageView = pView->GetImageEditingView();
			imageView->SetPolygonBrush();
			//printf("%d %d\n",m_iCurrentLayer,pView->GetCurrentLayerNum() );
			if ( m_iCurrentLayer != pView->GetCurrentLayerNum() )
			{//added for multi layer
				printf("layer changed\n");
				m_iCurrentLayer = pView->GetCurrentLayerNum();
				if ( m_iCurrentLayer == -1 )
				{
					return 0;
				}
				m_cPolygonSet.Clear();
				m_iCurrentFrame = -1;
			}
			if ( m_iCurrentFrame != pView->imageBuffer.GetFramePos() ) 
			{//added for multi frame change
				ChangePolygon();
				//added if else for drawing or adjust if there is no polygon or a polygon
				if ( !m_cPolygonSet.m_polygon.empty() )
				{
					printf("m_cPolygonSet not empty\n");
					m_bIsDrawingPolygon = false;
					m_cPolygon = &m_cPolygonSet.m_polygon[0];
					m_iCurrentPolygon = -1;
				}
				else 
				{
					printf("m_cPolygonSet empty\n");
					m_bIsDrawingPolygon = true;
					CPolygon polygon;
					m_cPolygonSet.m_polygon.push_back( polygon );
					m_iCurrentPolygon = 0;
					m_cPolygon = &m_cPolygonSet.m_polygon[m_iCurrentPolygon];
				}
				//GetRectRegion();
				if ( m_cPolygon->m_vPointSet.empty() )
				{
					printf("m_vPointSet empty\n");
					m_bIsDrawingPolygon = true;
				}
			}
			switch(Message)
			{
			case WM_LBUTTONDOWN:
				{
					//printf("WM_LBUTTONDOWN\n");
					if ( m_bIsDrawingPolygon )
					{
						//printf("m_bIsDrawingPolygon\n");
						LButtonDownDraw( lParam );
					}
					else
					{
						LButtonDownAdjust( lParam );
					}

				}
				break;
			case WM_LBUTTONUP:
				{
					LButtonUp();
				}
				break;
			case  WM_LBUTTONDBLCLK:
				{//the end draw button
					//printf("WM_LBUTTONDBLCLK\n");
					LButtonDBLCLK( lParam );
				}
				break;
			case WM_RBUTTONDOWN:
				{
					//printf("WM_RBUTTONDOWN\n");
					m_bIsDragImage = true;
				}
				break;
			case WM_RBUTTONUP:
				{
					//printf("WM_RBUTTONUP\n");
					m_bIsDragImage = false;
				}
				break;
			case WM_MOUSEMOVE:
				{
					MouseMove( lParam );
				}
				break;
			case WM_MOUSEWHEEL:
				{//added to ensure the polygon drawing effect when wheel mouse 
					MouseWheel();
				}
				break;
			case WM_KEYDOWN:
				printf("WM_KEYDOWN %c\n",wParam);
				if ( wParam == 'p'||  wParam == 'P' )
				{
					printf("SaveMaskFromPolygon\n");
					SaveImageFromPolygon();
				}
				else if ( wParam == 'o'||  wParam == 'O' )
				{
					printf("GetPolygonFromMask\n");
					GetPolygonFromMask( pView->imageBuffer.GetFramePos() );
					m_iCurrentPolygon = -1;
					pView->polygonManager.AddPolygon( m_iCurrentFrame, m_cPolygonSet );
					imageView->ReDrawRectRegion( m_cPolygonSet );
				}
				break;
			default:
				//printf("%d\n",Message);
				pView->polygonManager.AddPolygon( m_iCurrentFrame, m_cPolygonSet );
				//printf("default\n");
				break;
			}
		}
		return 1;
	}
	//customized func 
	CPoint CPolygonPaintTool::MakePoint( LPARAM lparam )
	{
		int x = GET_X_LPARAM( lparam );
		int y = GET_Y_LPARAM( lparam );
		pView->AdjustPos( x, y, imageView->GetRegion() );
		return CPoint( x, y );
		//return CPoint( GET_X_LPARAM( lparam ), GET_Y_LPARAM( lparam ) );
	}
	POINTTYPE CPolygonPaintTool::DetectPointType( const CPoint &point )
	{
		if ( m_iCurrentPolygon == -1 )
		{
			return UnknownPoint;
		}
		if ( DectectPolygonIndex( point ) != m_iCurrentPolygon )
		{
			return UnknownPoint;
		}
		for ( unsigned int i = 0; i < m_cPolygon->m_vPointSet.size(); ++i )
		{//detect the point on point 
			CPoint t = m_cPolygon->m_vPointSet[i];

			if ( point.x >= t.x - m_iPointRadius && point.x <= t.x + m_iPointRadius
				&& point.y >= t.y - m_iPointRadius && point.y <= t.y + m_iPointRadius )
			{
				return PointOnPoint;
			}
		}

		for ( unsigned int i = 0; i < m_cPolygon->m_vEdgeSet.size(); ++i )
		{//detect the point on point 
			CPoint p1 = m_cPolygon->m_vEdgeSet[i].first;
			CPoint p2 = m_cPolygon->m_vEdgeSet[i].second;

			if ( ( DisPointToPoint( p1, point ) + DisPointToPoint( p2, point ) 
				- DisPointToPoint( p1, p2 ) ) <= m_fLineDiff )
			{

				return PointOnLine;
			}
		}
		if ( PtInPolygon( point, m_cPolygon->m_vPointSet ) )
		{
			return PointInPolygon;
		}	
		////printf("%d %d %d %d\n", bound_rect.left,bound_rect.right,bound_rect.bottom,bound_rect.top);
		//if ( bound_rect.left <= point.x && bound_rect.right >= point.x
		//	&& bound_rect.bottom <= point.y && bound_rect.top >= point.y )
		//{//detect the point in polygon
		//	return PointInPolygon;
		//}
		return UnknownPoint;

	}

	void CPolygonPaintTool::PointToEdge()
	{
		m_cPolygon->m_vEdgeSet.clear();
		for ( unsigned int i = 0; i < m_cPolygon->m_vPointSet.size(); ++i )
		{
			m_cPolygon->m_vEdgeSet.push_back( pair<CPoint ,CPoint>( m_cPolygon->m_vPointSet[i], m_cPolygon->m_vPointSet[( i + 1 ) % m_cPolygon->m_vPointSet.size() ] ) );
		}

	}
	float CPolygonPaintTool::DisPointToPoint( const CPoint& p1, const CPoint& p2 )
	{
		return sqrt( float( ( p1.x - p2.x ) * ( p1.x - p2.x ) + ( p1.y - p2.y ) * ( p1.y - p2.y ) ) );
	}
	//void CPolygonPaintTool::GetRectRegion()
	//{
	//	if ( m_cPolygon->m_vPointSet.empty() )
	//	{
	//		return;
	//	}
	//	int min_x, min_y, max_x, max_y;
	//	min_x = max_x = m_cPolygon->m_vPointSet[0].x;
	//	min_y = max_y = m_cPolygon->m_vPointSet[0].y;

	//	for ( unsigned int i = 1; i < m_cPolygon->m_vPointSet.size(); ++i )
	//	{
	//		min_x = __min( min_x, m_cPolygon->m_vPointSet[i].x );
	//		min_y = __min( min_y, m_cPolygon->m_vPointSet[i].y );
	//		max_x = __max( max_x, m_cPolygon->m_vPointSet[i].x );
	//		max_y = __max( max_y, m_cPolygon->m_vPointSet[i].y );

	//	}
	//	bound_rect.SetRect( min_x-4, max_y+4, max_x+4, min_y-4 );
	//}
	unsigned int CPolygonPaintTool::GetPointIndex( const CPoint& point )
	{
		for ( unsigned int i = 0; i < m_cPolygon->m_vPointSet.size(); ++i )
		{//detect the point on point 
			CPoint t = m_cPolygon->m_vPointSet[i];

			if ( point.x >= t.x - m_iPointRadius && point.x <= t.x + m_iPointRadius
				&& point.y >= t.y - m_iPointRadius && point.y <= t.y + m_iPointRadius )
			{
				return i;
			}
		}
		return -1;
	}
	unsigned int CPolygonPaintTool::GetLineIndex( const CPoint& point )
	{
		for ( unsigned int i = 0; i < m_cPolygon->m_vEdgeSet.size(); ++i )
		{//detect the point on point 
			CPoint p1 = m_cPolygon->m_vEdgeSet[i].first;
			CPoint p2 = m_cPolygon->m_vEdgeSet[i].second;

			if ( ( DisPointToPoint( p1, point ) + DisPointToPoint( p2, point ) 
				- DisPointToPoint( p1, p2 ) ) <= m_fLineDiff )
			{

				return i;
			}
		}
	}
	CPoint CPolygonPaintTool::EraseLineLinkToPoint( const CPoint& point )
	{
		for ( unsigned int i = 0; i < m_cPolygon->m_vEdgeSet.size(); ++i )
		{//detect the point on point 
			CPoint p1 = m_cPolygon->m_vEdgeSet[i].first;
			CPoint p2 = m_cPolygon->m_vEdgeSet[i].second;

			if ( p1 == point || p2 == point )
			{
				if (  m_cPolygon->m_vEdgeSet.size() == i + 1 )
				{
					m_cPolygon->m_vEdgeSet.pop_back();
				}
				else
				{
					m_cPolygon->m_vEdgeSet.erase( m_cPolygon->m_vEdgeSet.begin() + i );
				}

				return ( p1 == point )?p2:p1; 
			}

		}
		printf("no link line found\n");
	}

	void CPolygonPaintTool::ChangePolygon()
	{
		printf("Change polygon\n");
		pView->polygonManager.AddPolygon( m_iCurrentFrame, m_cPolygonSet );
		m_cPolygonSet.Clear();
		int k = pView->imageBuffer.GetFramePos();
		if ( !pView->polygonManager.GetPolygonSet( k, m_cPolygonSet ) )
		{
			printf("no exist polygon in this frame\n");
			if ( !GetPolygonFromMask( k ) )
			{//由前一帧的polygon得到现在的polygon
				printf("GetPolygonFromPrevious\n");
				GetPolygonFromPrevious( k );
						}
					}

		imageView->ReDrawRectRegion( m_cPolygonSet );
		ResetState();
		if ( m_iCurrentFrame == -1 )
		{
			m_bIsDrawingPolygon = true;
			m_bIsAdjustingPolygon = false;
		}
		m_iCurrentFrame = pView->imageBuffer.GetFramePos();
	}

	void CPolygonPaintTool::MovePolygon( int x, int y )
	{
		for ( unsigned int i = 0; i < m_cPolygon->m_vPointSet.size(); ++i )
		{
			m_cPolygon->m_vPointSet[i].Offset( x, y );
		}
		for ( unsigned	int i = 0; i < m_cPolygon->m_vEdgeSet.size(); ++i )
		{
			m_cPolygon->m_vEdgeSet[i].first.Offset( x, y );
			m_cPolygon->m_vEdgeSet[i].second.Offset( x, y );
		}
	}

	void CPolygonPaintTool::LButtonDownDraw( LPARAM lParam )
	{
		m_bIsMouseDown = true;
		curr_Point = MakePoint(lParam);
		if ( !m_cPolygon->m_vPointSet.empty() )
		{
			imageView->DrawPolygonLine( prev_Point, m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1], false );
			imageView->DrawPolygonLine( curr_Point, m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1] );
			prev_Point = curr_Point; 
			imageView->DrawPolygonPoint( m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1], m_cPolygon->m_vPointSet, false );
			imageView->DrawPolygonPoint( curr_Point, m_cPolygon->m_vPointSet );
		}
		else
		{
			//printf("first point\n");
			imageView->DrawPolygonPoint( curr_Point, m_cPolygon->m_vPointSet );
			prev_Point = curr_Point;
		}
	}

	void CPolygonPaintTool::LButtonDownAdjust( LPARAM lParam )
	{
		mobile_point = MakePoint(lParam);
		POINTTYPE point_type = DetectPointType( mobile_point );
		int line_index = GetLineIndex( mobile_point );
		int point_index = m_iPointIndex = GetPointIndex( mobile_point );
		switch( point_type )
		{

		case PointOnPoint:
			printf("PointOnPoint\n");
			AdjustPointOnPoint( point_index );
			break;
		case PointOnLine://PointOnline
			printf("PointOnLine\n");
			AdjustPointOnLine( line_index );
			break;
		case PointInPolygon:
			printf("PointInPolygon\n");
			//m_bIsDragPolygon = true;
			//prev_Point = mobile_point;
			break;
		case UnknownPoint:
			int polygon_index =  DectectPolygonIndex( mobile_point );
			printf("polygon_index=%d\n",polygon_index);
			imageView->ReDrawRectRegion( m_cPolygonSet );
			if ( m_iCurrentPolygon !=  polygon_index || m_iCurrentPolygon == -1 )
			{
				//printf("m_iCurrentPolygon=%d, polygon_index=%d\n",m_iCurrentPolygon, polygon_index);
				m_iCurrentPolygon = polygon_index;
				if ( polygon_index != -1 )
				{
					m_cPolygon = &m_cPolygonSet.m_polygon[polygon_index];
					imageView->ReDrawRectRegion( m_cPolygonSet );
					imageView->SetLineColor( RGB(0,255,0) );
					imageView->ReDrawPolygon( m_cPolygon );
					imageView->SetLineColor( RGB(255,0,0) );
				}
				else
				{
					int iRet = ::MessageBox( NULL, "Do you want to creat a new polygon?\n ", "Remind", 
						MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1|MB_TASKMODAL );
					if ( iRet == IDYES ) 
					{ 
					CPolygon polygon;
					m_cPolygonSet.m_polygon.push_back( polygon );
					m_iCurrentPolygon = m_cPolygonSet.m_polygon.size() - 1;
					m_cPolygon = &m_cPolygonSet.m_polygon[m_iCurrentPolygon];
					m_bIsDrawingPolygon = true;
				}
			}
			}
			printf("UnknownPoint\n");
			break;
		}
	}

	void CPolygonPaintTool::AdjustPointOnPoint( int point_index )
	{
		mobile_point = m_cPolygon->m_vPointSet[point_index];
		printf("before erase edge num = %d\n",m_cPolygon->m_vEdgeSet.size() );
		fixed_point1 = EraseLineLinkToPoint( mobile_point ); 
		fixed_point2 = EraseLineLinkToPoint( mobile_point ); 
		printf("after erase edge num = %d\n",m_cPolygon->m_vEdgeSet.size() );
		if ( m_cPolygon->m_vPointSet.size() == point_index + 1 )
		{//because the last element of vector can not be erased
			m_cPolygon->m_vPointSet.pop_back();
		}
		else
		{
			m_cPolygon->m_vPointSet.erase( m_cPolygon->m_vPointSet.begin() + point_index );
		}
		//redraw
		imageView->ReDrawRectRegion( m_cPolygonSet );
		m_bIsAdjustingPolygon = true;
		imageView->DrawTwoDynamicLine( mobile_point, fixed_point1, fixed_point2 );

	}

	void CPolygonPaintTool::AdjustPointOnLine( int line_index )
	{
		fixed_point1 = m_cPolygon->m_vEdgeSet[line_index].first;
		fixed_point2 = m_cPolygon->m_vEdgeSet[line_index].second;
		if ( __max( GetPointIndex(fixed_point1), GetPointIndex(fixed_point2) )
			- __min( GetPointIndex(fixed_point1), GetPointIndex(fixed_point2) ) 
			== m_cPolygon->m_vPointSet.size() - 1 )
		{
			m_iPointIndex = 0;
		}	
		else
		{
			m_iPointIndex = __max( GetPointIndex(fixed_point1), GetPointIndex(fixed_point2) );
		}

		if ( m_cPolygon->m_vEdgeSet.size() == line_index + 1 )
		{//because the last element of vector can not be erased
			m_cPolygon->m_vEdgeSet.pop_back();
		}
		else
		{
			m_cPolygon->m_vEdgeSet.erase( m_cPolygon->m_vEdgeSet.begin() + line_index );
		}
		imageView->ReDrawRectRegion( m_cPolygonSet );
		m_bIsAdjustingPolygon = true;
		imageView->DrawTwoDynamicLine( mobile_point, fixed_point1, fixed_point2 );
	}

	void CPolygonPaintTool::LButtonDBLCLK( LPARAM lParam )
	{
		if ( m_bIsDrawingPolygon )
		{
			m_bIsDrawingPolygon = false;
			m_bIsMouseDown = false;
			if ( m_cPolygon->m_vPointSet.size() >= 2 )
			{
				//xor the line 
				curr_Point = MakePoint(lParam);
				imageView->DrawPolygonLine( prev_Point, m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1], false );
				prev_Point = curr_Point; 

				imageView->DrawPolygonLine(  m_cPolygon->m_vPointSet[0], m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1] );
				if ( m_cPolygon->m_vPointSet.size() >= 2 )
				{
					m_cPolygon->m_vEdgeSet.push_back( pair<CPoint, CPoint>(m_cPolygon->m_vPointSet[0],m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1] ) );
				}
				imageView->DrawPolygonPoint( m_cPolygon->m_vPointSet[0], m_cPolygon->m_vPointSet, false );
				imageView->DrawPolygonPoint( m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1], m_cPolygon->m_vPointSet, false );
				pView->polygonManager.AddPolygon( m_iCurrentFrame, m_cPolygonSet );
			}
			else
			{
				m_cPolygon->m_vPointSet.clear();
				PointToEdge();
				imageView->ReDrawPolygon( m_cPolygon );
			}
			//generate edge according to the point set
			//PointToEdge();
			//GetRectRegion();
		}
		else  
		{//for deleting the polygon point
			mobile_point = MakePoint( lParam );
			int point_index = GetPointIndex( mobile_point );
			if ( point_index != -1 )
			{
				if ( point_index < m_cPolygon->m_vPointSet.size() && m_cPolygon->m_vPointSet.size() > 3 )
				{
					m_cPolygon->m_vPointSet.erase( m_cPolygon->m_vPointSet.begin() + point_index );
				}
			}
			else
			{
				int point_index = DectectPolygonIndex( mobile_point );
				if ( point_index != -1 && point_index == m_iCurrentPolygon )
				{//delete the polygon
					int iRet = ::MessageBox( NULL, "Do you want to delete this polygon?\n ", "Remind", 
						MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1|MB_TASKMODAL );
					if ( iRet == IDYES ) 
					{
						m_cPolygon->Clear();
						m_bIsDrawingPolygon = false;
					}
				}
			}
			PointToEdge();
			imageView->ReDrawRectRegion( m_cPolygonSet );
			pView->polygonManager.AddPolygon( m_iCurrentFrame, m_cPolygonSet );
		}
		//SaveMaskFromPolygon();
	}

	void CPolygonPaintTool::LButtonUp()
	{
		if ( m_bIsAdjustingPolygon )//after the adjusting 
		{
			//printf("m_bIsAdjustingPolygon\n");
			//erasing the dynamic line
			imageView->DrawTwoDynamicLine( mobile_point, fixed_point1, fixed_point2 );
			//adding the red fixed line 
			imageView->DrawPolygonLine( mobile_point, fixed_point1, true );
			imageView->DrawPolygonLine( mobile_point, fixed_point2, true );
			imageView->DrawPolygonPoint( mobile_point, m_cPolygon->m_vPointSet, false );
			//keep the point set orderly
			if ( m_iPointIndex == 0 )
			{
				m_cPolygon->m_vPointSet.push_back( mobile_point );
			}
			else
				m_cPolygon->m_vPointSet.insert( m_cPolygon->m_vPointSet.begin() + m_iPointIndex, mobile_point );
			//make the drawing point good
			imageView->DrawPolygonPoint( fixed_point1, m_cPolygon->m_vPointSet, false );
			imageView->DrawPolygonPoint( fixed_point2, m_cPolygon->m_vPointSet, false );
			//add two increased line into edge vector
			m_cPolygon->m_vEdgeSet.push_back( pair<CPoint, CPoint>(fixed_point1,m_cPolygon->m_vPointSet[m_iPointIndex]) );
			m_cPolygon->m_vEdgeSet.push_back( pair<CPoint, CPoint>(fixed_point2,m_cPolygon->m_vPointSet[m_iPointIndex]) );
			//update the rect
			//GetRectRegion();
			m_bIsAdjustingPolygon = false;
			PointToEdge();
			pView->polygonManager.AddPolygon( m_iCurrentFrame, m_cPolygonSet );
		}
		/*else if ( m_bIsDragPolygon )
		{
			m_bIsDragPolygon = false;
			pView->polygonManager.AddPolygon( m_iCurrentFrame, m_cPolygonSet );
		}*/
		else if ( m_bIsDrawingPolygon && m_bIsMouseDown )
		{//added to ensure the polygon drawing effect when wheel mouse 
			if ( m_cPolygon->m_vPointSet.size() >= 2 )
			{
				m_cPolygon->m_vEdgeSet.push_back( pair<CPoint, CPoint>(m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1] ,m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 2] ) );
			}
			
		}
		
	}

	void CPolygonPaintTool::MouseMove( LPARAM lParam )
	{
		if ( m_bIsDrawingPolygon && m_bIsMouseDown )
		{
			//printf("WM_MOUSEMOVE\n");
			if ( !m_bIsDragImage )
			{   
				curr_Point = MakePoint(lParam);
				imageView->DrawPolygonLine( prev_Point, m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1], false );
				imageView->DrawPolygonLine( curr_Point, m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1], false );
				prev_Point = curr_Point; 
			}
			else
			{//when drawing and drag the image
				imageView->ReDrawRectRegion( m_cPolygonSet );
				imageView->DrawPolygonLine( prev_Point, m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1], false );
			}

		}
		else if ( m_bIsAdjustingPolygon )
		{
			if ( !m_bIsDragImage )
			{
				imageView->DrawTwoDynamicLine( mobile_point, fixed_point1, fixed_point2 );
				mobile_point = MakePoint(lParam);
				imageView->DrawTwoDynamicLine( mobile_point, fixed_point1, fixed_point2 );
			}
			else
			{////when adjusting and drag the image
				imageView->ReDrawRectRegion( m_cPolygonSet );
				imageView->DrawTwoDynamicLine( mobile_point, fixed_point1, fixed_point2 );
			}
		}
		//else if ( m_bIsDragPolygon )
		//{//drag the polygon
		//	curr_Point = MakePoint(lParam);
		//	MovePolygon( curr_Point.x - prev_Point.x, curr_Point.y - prev_Point.y );
		//	bound_rect.OffsetRect( curr_Point.x - prev_Point.x, curr_Point.y - prev_Point.y );
		//	imageView->ReDrawRectRegion( m_cPolygonSet );
		//	prev_Point = curr_Point; 
		//}
		else if ( m_bIsDragImage )
		{
			//PointToEdge();
			imageView->ReDrawRectRegion( m_cPolygonSet );
		}
	}

	void CPolygonPaintTool::MouseWheel()
	{
		imageView->ReDrawRectRegion( m_cPolygonSet );
		if ( m_bIsDrawingPolygon && m_bIsMouseDown )
		{
			imageView->DrawPolygonLine( prev_Point, m_cPolygon->m_vPointSet[m_cPolygon->m_vPointSet.size() - 1], false );
		}
	}
	int CPolygonPaintTool::GetRightMostX( vector<CPoint>& point_set )
	{
		int  rightx= 0;
		for ( int i = 0; i < point_set.size(); ++i )
		{
			rightx = __max( rightx, point_set[i].x );
		}
		return rightx;
	}
	int CPolygonPaintTool::DectectPolygonIndex( const CPoint& point )
	{
		int polygon_num = m_cPolygonSet.m_polygon.size();
		int polygon_index = -1;
		int  minx= -1;
		//printf("minx = %d\n",minx);
		//printf("polygon_num=%d\n",polygon_num);
		for ( int i = 0; i < polygon_num; ++i )
		{
			if ( PtInPolygon( point, m_cPolygonSet.m_polygon[i].m_vPointSet ) )
			{
				int x = GetRightMostX( m_cPolygonSet.m_polygon[i].m_vPointSet );
				if ( minx > x || minx == -1 )
				{
					minx = x;
					polygon_index = i;
				}
			}
			if ( PtOnPolygon( point, m_cPolygonSet.m_polygon[i].m_vPointSet, m_cPolygonSet.m_polygon[i].m_vEdgeSet, m_iPointRadius, m_fLineDiff ) )
			{
				int x = GetRightMostX( m_cPolygonSet.m_polygon[i].m_vPointSet );
				if ( minx > x || minx == -1 )
				{
					minx = x;
					polygon_index = i;
				}
			}
		}
		return polygon_index;
	}
	bool CPolygonPaintTool::PtOnPolygon( CPoint point, const vector<CPoint>& point_set, 
										 const vector< pair<CPoint, CPoint> >& edge_set, 
										 int point_radius, float line_diff )
	{
		int point_num = point_set.size();
		for ( unsigned int i = 0; i < point_num; ++i )
		{//detect the point on point 
			CPoint t = point_set[i];

			if ( point.x >= t.x - point_radius && point.x <= t.x + point_radius
				&& point.y >= t.y - point_radius && point.y <= t.y + point_radius )
			{
				return true;
			}
		}
		int edge_num = edge_set.size();
		for ( unsigned int i = 0; i < edge_num; ++i )
		{//detect the point on point 
			CPoint p1 = edge_set[i].first;
			CPoint p2 = edge_set[i].second;

			if ( ( DisPointToPoint( p1, point ) + DisPointToPoint( p2, point ) 
				- DisPointToPoint( p1, p2 ) ) <= line_diff )
			{

				return true;
			}
		}
		return false;
	}
	bool CPolygonPaintTool::PtInPolygon( CPoint point, const vector<CPoint>& point_set )
	{
		int nCross = 0;
		int nCount = point_set.size();
		if ( nCount < 3 )
		{
			return false;
		}
		for ( int i = 0; i < nCount; ++i ) 
		{
			CPoint p1 = point_set[i];  
			CPoint p2 = point_set[(i + 1) % nCount];

			// 求解 y=p.y 与 p1p2 的交点  

			if ( p1.y == p2.y )      // p1p2 与 y=p0.y平行
				continue;

			if ( point.y <  min(p1.y, p2.y) )   // 交点在p1p2延长线上
				continue;
			if ( point.y >= max(p1.y, p2.y) )   // 交点在p1p2延长线上
				continue;

			// 求交点的 X 坐标 --------------------------------------------------------------
			double x = (double)(point.y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) + p1.x;

			if ( x > point.x ) 
				nCross++;       // 只统计单边交点
		}

		// 单边交点为偶数，点在多边形之外 ---
		return (nCross % 2 == 1);
	}
	CPoint CPolygonPaintTool::GetPolygonLeftBottomPoint( vector<CPoint>& point_set )
	{//min x & min y 
		CPoint point;
		point = point_set[0];
		for ( int i = 1; i < point_set.size(); ++i )
		{
			if ( point.x > point_set[i].x )
			{
				point.x = point_set[i].x;
			}
			if ( point.y > point_set[i].y )
			{
				point.y = point_set[i].y;
			}
		}
		return point;
	}
	CPoint CPolygonPaintTool::GetPolygonRightTopPoint( vector<CPoint>& point_set )
	{//max x & max y
		CPoint point;
		point = point_set[0];
		for ( int i = 1; i < point_set.size(); ++i )
		{
			if ( point.x < point_set[i].x )
			{
				point.x = point_set[i].x;
			}
			if ( point.y < point_set[i].y )
			{
				point.y = point_set[i].y;
			}
		}
		return point;
	}
	void CPolygonPaintTool::SaveImageFromPolygon()
	{
		int width = pView->imageBuffer.GetImage()->GetWidth();
		int height = pView->imageBuffer.GetImage()->GetHeight();
		CxImage* color_image = pView->imageBuffer.GetImage();
		unsigned char* label = new unsigned char[width*height];
		printf("GetLabelFromPolygon\n");
		GetLabelFromPolygon( label );
		printf("ContourBoundRefine\n");
		ContourBoundRefine( color_image, label );
		printf("After ControuBoundRefine\n");
		//pView->firstFrameOK=true;
		pView->GetDocument()->cal=true;
		pView->GetDocument()->updateMode=LOCAL_UPDATE;
		pView->GetDocument()->UpdateImages(pView->imageBuffer.GetImage());
		pView->GetDocument()->RestoreFrame(pView->imageBuffer.GetImage(),pView->alphaBuffer.GetImage());
		delete []label;
	}

	void CPolygonPaintTool::UpdatePolygonByFlow()
	{
		//获得光流路径 view 类中有个filepath 即使path的内容
		int width = pView->imageBuffer.GetImage()->GetWidth();
		int height = pView->imageBuffer.GetImage()->GetHeight();
		int frame = pView->imageBuffer.GetFramePos();
		CString uflowpath = pView->GetFilePath();
		//uflowpath.AppendFormat("opt\\u_%d_%d.raw", frame - 1, frame );//x 方向光流路径
		uflowpath.AppendFormat("optical flow\\flowu%d.raw", frame - 1 );

		//std::cout<<uflowpath.GetBuffer()<<std::endl;
		CString vflowpath = pView->GetFilePath();
		vflowpath.AppendFormat("optical flow\\flowv%d.raw", frame - 1 );
		//vflowpath.AppendFormat("opt\\v_%d_%d.raw", frame - 1, frame );//y 方向光流路径

		//加载光流
		float* u = new float[width*height];
		float* v = new float[width*height];
		memset(u, 0, sizeof(float)*width*height);
		memset(v, 0, sizeof(float)*width*height);
		//load optical flow
		FILE* pfile = NULL;
		pfile = fopen(uflowpath, "rb");
		printf("uflowpath = %s\n",uflowpath);
		if (pfile)
		{
			printf("yes\n");
			fread(u, sizeof(float), width*height, pfile);
			fclose(pfile);
		}
		pfile = fopen(vflowpath, "rb");
		//printf("vflowpath = %s\n",vflowpath);
		if (pfile)
		{
			printf("yes\n");
			fread(v, sizeof(float), width*height, pfile);
			fclose(pfile);
		}
		//利用光流设置新位置
		for ( int p = 0; p < m_cPolygonSet.m_polygon.size(); ++p )
		{
			for ( int i = 0; i < m_cPolygonSet.m_polygon[p].m_vPointSet.size(); ++i )
			{
				int x = m_cPolygonSet.m_polygon[p].m_vPointSet[i].x;
				int y = m_cPolygonSet.m_polygon[p].m_vPointSet[i].y;
				int index = x + y * width;
				//printf("before flow x = %d, y = %d\n", m_cPolygonSet.m_polygon[p].m_vPointSet[i].x, m_cPolygonSet.m_polygon[p].m_vPointSet[i].y);
				m_cPolygonSet.m_polygon[p].m_vPointSet[i].x += u[index];//MeanOpticalFlow( x, y, u, 100 );
				m_cPolygonSet.m_polygon[p].m_vPointSet[i].y += v[index];//MeanOpticalFlow( x, y, v, 100 );
				//printf("after flow x = %d, y = %d\n", m_cPolygonSet.m_polygon[p].m_vPointSet[i].x, m_cPolygonSet.m_polygon[p].m_vPointSet[i].y);
			}
		}

	}

	float CPolygonPaintTool::MeanOpticalFlow( int x, int y, float* a, int radius )
	{
		float mean = 0.0;
		int cnt = 0;
		int width = pView->imageBuffer.GetImage()->GetWidth();
		int height = pView->imageBuffer.GetImage()->GetHeight();
		int xS = __max( 0, x - radius );
		int yS = __max( 0, y - radius );
		int xE = __min( width, x + radius );
		int yE = __min( height, y + radius );
		for ( x = xS; x < xE; ++x )
		{
			for ( y = yS; y < yE; ++y )
			{
				mean += a[x + y * width];
				cnt++;
			}
		}
		if ( cnt != 0 )
		{
			mean /= cnt;
		}
		return mean;
	}

	void CPolygonPaintTool::ContourBoundRefine( CxImage* color_image, unsigned char* label )
	{
		int width = pView->imageBuffer.GetImage()->GetWidth();
		int height = pView->imageBuffer.GetImage()->GetHeight();
		CxImage mask;
		CxImage result;
		mask.Create( width, height, 8, CXIMAGE_FORMAT_PNG );
		TRANS_CLASSIFIER_PARAM param;
		param.mode = 3;
		Contour boundseg;
		boundseg.SetFrameInfo(color_image,label);
		boundseg.SetLocalWinSize(30);
		boundseg.GetLocalClassifier(param, 0);
		boundseg.GetBeta_WholeImg();//修改过的边界refine
		boundseg.GetCutResultUC(label);
		boundseg.GetAlphaResult(mask);
		result.Copy(mask);
		boundseg.GetCutResult(result);
		//save the image
		pView->alphaBuffer.SetPos(pView->imageBuffer.GetFramePos());
		printf("save polygon mask to %s\n",pView->alphaBuffer.GetFrameName() );
		mask.Save( pView->alphaBuffer.GetFrameName(), CXIMAGE_FORMAT_PNG );
		pView->alphaBuffer.OpenImage(mask);
		//Save the result Image
		pView->resImageBuffer.SetPos(pView->imageBuffer.GetFramePos());
		printf("save polygon result to %s\n",pView->resImageBuffer.GetFrameName() );
		result.Save( pView->resImageBuffer.GetFrameName(), CXIMAGE_FORMAT_JPG );
		pView->resImageBuffer.OpenImage(result);
		pView->Invalidate();
		pView->GetImageEditingView()->ReDrawRectRegion( m_cPolygonSet );
	}

	void CPolygonPaintTool::GetLabelFromPolygon( unsigned char* label )
	{
		int width = pView->imageBuffer.GetImage()->GetWidth();
		int height = pView->imageBuffer.GetImage()->GetHeight();
		for ( int j = 0; j < height; ++j )
		{
			for ( int i = 0; i < width; ++i )
			{
				label[j*width + i] = 0;
			}
		}
		//set the point in the polygon !flag
		for ( int ip = 0; ip < m_cPolygonSet.m_polygon.size(); ++ip )
		{
			m_cPolygonSet.m_polygon[ip].m_vEdgeSet.clear();
			if ( m_cPolygonSet.m_polygon[ip].m_vPointSet.size() >= 3 )
			{
				for ( int i = 0; i < m_cPolygonSet.m_polygon[ip].m_vPointSet.size(); ++i )
				{
					m_cPolygonSet.m_polygon[ip].m_vEdgeSet.push_back( 
						pair<CPoint, CPoint>( m_cPolygonSet.m_polygon[ip].m_vPointSet[i], 
						m_cPolygonSet.m_polygon[ip].m_vPointSet[ (i + 1) % m_cPolygonSet.m_polygon[ip].m_vPointSet.size() ] ) );
				}
				CPoint lb = GetPolygonLeftBottomPoint( m_cPolygonSet.m_polygon[ip].m_vPointSet );
				CPoint rt = GetPolygonRightTopPoint( m_cPolygonSet.m_polygon[ip].m_vPointSet );
				for ( int i = lb.x; i < rt.x; ++i )
				{
					for ( int j = lb.y; j < rt.y; ++j )
					{
						if ( PtOnPolygon( CPoint( i, j ), m_cPolygonSet.m_polygon[ip].m_vPointSet, m_cPolygonSet.m_polygon[ip].m_vEdgeSet ) 
							|| PtInPolygon( CPoint( i, j ), m_cPolygonSet.m_polygon[ip].m_vPointSet ) )
						{
							label[j*width + i] = ( label[j*width + i] + 1 ) % 2;
						}		
					}
				}
			}
		}
	}

	bool CPolygonPaintTool::GetPolygonFromMask( const int& k )
	{
		CxImage mask_image;
		if ( mask_image.Load( pView->alphaBuffer.GetFrameName( k ), CXIMAGE_FORMAT_PNG ) )
		{//由label得到polygon
			PolygonGenerator polygon_generator;
			clock_t start, finish;
			double   duration;
			start = clock();
			m_cPolygonSet.Clear();
			polygon_generator.Slove( mask_image, m_fCosThreshold, m_fDisThreshold, m_cPolygonSet );
			finish = clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			printf( "%lf, total get polygon from image %f seconds\n", m_fCosThreshold,duration );
			return true;
		}
		else return false;
	}

	void CPolygonPaintTool::GetPolygonFromPrevious( int k )
	{
		if ( pView->polygonManager.GetPolygonSet( k - 1, m_cPolygonSet ) )
		{	
			UpdatePolygonByFlow();
			for ( int p = 0; p < m_cPolygonSet.m_polygon.size(); ++p )
			{
				m_cPolygonSet.m_polygon[p].m_vEdgeSet.clear();
				for ( int i = 0; i < m_cPolygonSet.m_polygon[p].m_vPointSet.size(); ++i )
				{
					m_cPolygonSet.m_polygon[p].m_vEdgeSet.push_back( 
						pair<CPoint, CPoint>( m_cPolygonSet.m_polygon[p].m_vPointSet[i], 
						m_cPolygonSet.m_polygon[p].m_vPointSet[ (i + 1) % m_cPolygonSet.m_polygon[p].m_vPointSet.size() ] ) );
				}
			}
			imageView->ReDrawRectRegion( m_cPolygonSet );
		}
	}

}