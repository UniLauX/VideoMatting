#include "StdAfx.h"
#include "PolygonGenerator.h"
#include <map>
#include <cmath>
#include <string>
#include <time.h>
PolygonGenerator::PolygonGenerator(void)
{
}
PolygonGenerator::~PolygonGenerator(void)
{
	delete [] m_alpha;
}
void PolygonGenerator::Init( CxImage& srcImage )
{
	m_image = srcImage;
	m_width = srcImage.GetWidth();
	m_height = srcImage.GetHeight();
	//get the foreground contour and background contour
	m_alpha =new unsigned char[m_width*m_height];
	for ( int j = 0; j < m_height; ++j )
	{
		for ( int i = 0; i < m_width; ++i )
		{
			if ( srcImage.GetPixelGray( i, j ) == 0 )
			{
				m_alpha[j*m_width + i] = 0;
			}
			else m_alpha[j*m_width + i] = 1;
		}
	}
}
double PolygonGenerator::Curvature(const CPoint &p0, const CPoint &p1, const CPoint &p2)
{
	double ma_x = p1.x - p0.x;  
	double ma_y = p1.y - p0.y;  
	double mb_x = p2.x - p1.x;  
	double mb_y = p2.y - p1.y;  
	double v1 = (ma_x * mb_x) + (ma_y * mb_y);  
	double ma_val = sqrt(ma_x*ma_x + ma_y*ma_y);  
	double mb_val = sqrt(mb_x*mb_x + mb_y*mb_y);  
	double cosM = fabs( v1 / (ma_val*mb_val) );  
	return cosM;

	/*int dx = p0.x + p2.x - 2*p1.x;
	int dy = p0.y + p2.y - 2*p1.y;
	return std::sqrt(double(dx*dx + dy*dy));*/
}
bool PolygonGenerator::Simplify(std::vector<CPoint> &contour)
{
	bool flag = true;
	if (contour.size() < 3)
	{
		return true;
	}
	std::vector<CPoint>temp = contour;

	contour.clear();
	contour.push_back(temp[0]);
	for ( int i = 1; i < temp.size(); ++i )
	{
		if ( DisSum( temp[i-1], temp[i], temp[(i+1)%temp.size()]) > m_disThreshold 
			&& Distance( temp[i], contour[ contour.size() -1] ) > ( m_disThreshold / 2.0 ) )
		{
			contour.push_back( temp[i] );
		}		
		else flag = false;
	}
	printf("before size = %d, after size = %d\n", temp.size(), contour.size());
	if ( temp.size() == contour.size() || flag )
	{
	return true;
}
}

//////////////////////////////////////////////////////////////////////////
/// generate the polygon, by m_cosThresholding the Curvature.
//////////////////////////////////////////////////////////////////////////
bool PolygonGenerator::generate_polygon_from_contour_simple(const std::vector<CPoint> &contour, std::vector<CPoint> &polygon)
{
	if (contour.size() < 3)
	{
		return true;
	}
	polygon.clear();
	int count = contour.size();
	polygon.push_back( contour[0] );
	//std::vector<double> Curvature;
	for (int i = 1; i < count; ++ i)
	{
		double curv = Curvature(polygon.at( polygon.size() - 1 ), contour[i], contour[(i+1)%count]);
		//printf("%f ", curv);

		if (curv < m_cosThreshold)
		{
			polygon.push_back(contour[i]);
		}
	}
	return Simplify( polygon );
}
int PolygonGenerator::DistanceSqr( const CPoint &A, const CPoint &B )
{
	return (A.x - B.x)*(A.x - B.x) + (A.y - B.y)*(A.y - B.y);
}
int PolygonGenerator::DotProduct( const CPoint &A, const CPoint &B )
{
	return A.x*B.x + A.y*B.y;
}
int PolygonGenerator::DotPerpendicularProduct( const CPoint &A, const CPoint &B )
{
	int x, y;
	x = -B.y;
	y = B.x;
	return x*A.x + y*A.y;
}
double PolygonGenerator::Position_U(const CPoint &X, const CPoint &P, const CPoint &Q)
{
	return (double)DotProduct( X-P, Q-P) / (double)DistanceSqr(P,Q);
}
double PolygonGenerator::Distance_V(const CPoint &X, const CPoint &P, const CPoint &Q)
{
	return (double)DotPerpendicularProduct( X-P, Q-P) / std::sqrt( (double)DistanceSqr(P,Q) );
}

double PolygonGenerator::distance_2_segment(const CPoint &X, const CPoint &P, const CPoint &Q)
{
	double u = Position_U(X, P, Q);

	if (u < 0)
	{
		return std::sqrt( (double)DistanceSqr(X,P) );
	}
	else if (u > 1)
	{
		return std::sqrt( (double)DistanceSqr(X,Q) );
	}
	else
	{
		return fabs(Distance_V(X, P, Q));
	}
	return 0.0;
}

double PolygonGenerator::distance_2_polygon(const CPoint &pt, std::vector<CPoint> &polygon)
{
	if (polygon.size() == 0)
	{
		return 0.0f;
	}
	else if (polygon.size() == 1)
	{
		return std::sqrt( (double)DistanceSqr( pt, polygon[0] ) );
	}
	else if (polygon.size() == 2)
	{
		return distance_2_segment(pt, polygon[0], polygon[1]);
	}
	//////////////////////////////////////////////////////////////////////////
	double mindist = distance_2_segment(pt, polygon[0], polygon[polygon.size() - 1]);

	for (int i = 0; i < polygon.size() - 1; ++ i)
	{
		double dist = distance_2_segment(pt, polygon[i], polygon[i + 1]);
		if (dist < mindist)
		{
			mindist = dist;
		}
	}

	return mindist;
}


//////////////////////////////////////////////////////////////////////////
/// generate the polygon like the Lazy Snapping.
//////////////////////////////////////////////////////////////////////////
bool PolygonGenerator::generate_polygon_from_contour(const std::vector<CPoint> &contour, std::vector<CPoint> &polygon)
{
	clock_t start, finish;
	double   duration;
	start = clock();
	if (contour.size() < 10)
	{
		return false;
	}

	polygon.clear();
	int count = contour.size();

	//////////////////////////////////////////////////////////////////////////
	// determine the first point.
	double maxCurvature = Curvature(contour[count - 1], contour[0], contour[1]);
	int firstpoint = 0;
	for (int i = 0; i < count; ++ i)
	{
		double curv = Curvature(contour[(i-1 + count)%count], contour[i], contour[(i+1)%count]);
		if (curv > maxCurvature)
		{
			maxCurvature = curv;
			firstpoint = i;
		}
	}

	std::map<int, CPoint> temppolygon;
	temppolygon.insert(std::make_pair(firstpoint, contour[firstpoint]));

	//////////////////////////////////////////////////////////////////////////
	polygon.clear();
	std::map<int, CPoint>::iterator it = temppolygon.begin();
	for (; it != temppolygon.end(); ++ it)
	{
		polygon.push_back(it->second);
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// determine other points.

	std::vector<int> reminepoints;
	for (int i = 0; i < count; ++ i)
	{
		reminepoints.push_back(i);
	}

	reminepoints.erase(reminepoints.begin() + firstpoint);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf( "not while proceed %f seconds\n", duration );
	printf("reminepoints.size() = %d\n",reminepoints.size());
	start = clock();
	while (reminepoints.size() > 1)
	{
		int maxposition = 0;

		double maxdist = distance_2_polygon(contour[reminepoints[0]], polygon);

		for (int i = 1; i < reminepoints.size(); ++ i)
		{
			double dist = distance_2_polygon(contour[reminepoints[i]], polygon);

			if (dist > maxdist)
			{
				maxdist = dist;
				maxposition = i;
			}
		}

		if (maxdist > m_cosThreshold)
		{
			temppolygon.insert(std::make_pair(reminepoints[maxposition], contour[reminepoints[maxposition]]));
			reminepoints.erase(reminepoints.begin() + maxposition);

			//////////////////////////////////////////////////////////////////////////
			polygon.clear();
			std::map<int, CPoint>::iterator it = temppolygon.begin();
			for (; it != temppolygon.end(); ++ it)
			{
				polygon.push_back(it->second);
			}
			//////////////////////////////////////////////////////////////////////////
			//printf("%d\n", polygon.size());
		}
		else
		{
			break;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf( "while proceed %f seconds\n", duration );
	return true;
}

void PolygonGenerator::GetOrderedBound(const CPoint &sPoint,
							  int *flag, std::vector<CPoint> &points, int num)
{
	CPoint point;
	int x0,y0;//first point
	int x1(-2),y1(-2);//second point
	int xc(-1),yc(-1);//current border point 
	int xp(-1),yp(-1);//previous border point
	int dir(7);
	int count(0);
	int cc(0);
	int mm = num;
	int xtemp,ytemp;
	RGBQUAD r={255,0,0,0};

	//find the start point
	x0=sPoint.x;
	y0=sPoint.y;
	xc=sPoint.x;
	yc=sPoint.y;
	do
	{
		--mm;
		count++;
		point.x=xc;
		point.y=yc;
		if (flag[yc*m_width+xc])
		{
			flag[yc*m_width+xc]=0;
			cc++;
			points.push_back(point);
		}
		if (dir & 1)
		{
			dir=(dir+6) & 7;
		}
		else
			dir=(dir+7) & 7;
		for (int p=0;p<8;++p)
		{
			if (GetNeighbourValue(xc,yc,dir,xtemp,ytemp))
			{
				xp=xc;
				yp=yc;
				xc=xtemp;
				yc=ytemp;
				if (count==1)
				{
					x1=xc;
					y1=yc;
				}
				break;
			}
			else
			{
				dir++;
				dir &=7;
			}
		}
	} while (mm);

}
int PolygonGenerator::GetNeighbourValue(int x, int y, int dir, 
							   int &xtemp, int &ytemp)
{
	static int direction[8][2]={{1,0},{1,1},{0,1},{-1,1},
	{-1,0},{-1,-1},{0,-1},{1,-1}};
	//static int direction[8][2] = {{-1,1}, {0,1}, {1,1},{1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}};

	if (!(x>=0 && x<m_width && y>=0 && y<m_height))
		return 0;

	xtemp=x+direction[dir][0];
	ytemp=y+direction[dir][1];
	if (xtemp>=0&&xtemp<m_width&&ytemp>=0&&ytemp<m_height)
	{
		if (m_alpha[ytemp*m_width + xtemp]==1)
			return 1;
		else
			return 0;
	}
	else
		return 0;

}
void PolygonGenerator::GetBound(int *flag, std::vector<CPoint> &points)
{
	int i,j,k;
	int x,y;
	int index1,index2;
	CPoint point;
	int direct[4][2]={{1,0},{0,1},{-1,0},{0,-1}};

	for(i=1;i<m_height-1;++i)
	{
		for(j=1;j<m_width-1;++j)
		{
			index1=i*m_width+j;
			if (m_alpha[index1])
			{			
				for(k=0;k<4;++k)
				{	
					y=i+direct[k][0];
					x=j+direct[k][1];
					index2=y*m_width+x;
					//   if(x>=0 && x<m_width && y>=0 && y<m_height)
					{
						if(m_alpha[index1]!=m_alpha[index2])
						{
							point.x=j;
							point.y=i;
							points.push_back(point);
							flag[index1]=1;
							break;
						}
					}
				}
			}
		}
	}

	for (int i=0; i<m_width; ++i)
	{
		index1 = 0*m_width+i;
		index2 = (m_height-1)*m_width+i;
		if (m_alpha[index1])
		{
			point.x = i;
			point.y = 0;
			points.push_back(point);
			flag[index1] =1;
		}
		if (m_alpha[index2])
		{
			point.x = i;
			point.y = m_height-1;
			points.push_back(point);
			flag[index2] = 1;
		}
	}

	for (int j=0; j<m_height; ++j)
	{
		index1 = j*m_width+0;
		index2 = j*m_width+m_width-1;
		if (m_alpha[index1])
		{
			point.x = 0;
			point.y = j;
			points.push_back(point);
			flag[index1] = 1;
		}
		if (m_alpha[index2])
		{
			point.x = m_width-1; 
			point.y = j;
			points.push_back(point);
			flag[index2] = 1;
		}
	}

	CxImage tempim;
	tempim.Create(m_width, m_height,24);
	for (int i=0; i<points.size(); ++i)
	{
		tempim.SetPixelColor(points[i].x, points[i].y, RGB(255,0,0));
	}
	tempim.Save("H:/te_ge.bmp", CXIMAGE_FORMAT_BMP);
}
//void Display( const std::vector<CPoint>& point_set, CxImage srcImage, COLORREF rgb )
//{
//	int static cnt = 0;
//	CxImage result;
//	result.Create( srcImage.GetWidth(), srcImage.GetHeight(), 24, CXIMAGE_FORMAT_PNG );
//	for ( int j = 0; j < srcImage.GetHeight(); ++j )
//	{
//		for ( int i = 0; i < srcImage.GetWidth(); ++i )
//		{
//			result.SetPixelColor( i, j, srcImage.GetPixelColor( i, j ) );
//		}
//	}
//	for ( int i = 0; i < point_set.size(); ++i )
//	{
//		result.SetPixelColor( point_set[i].x, point_set[i].y, rgb );	
//	}
//	std::string name;
//	name = "output.png";
//	result.Save( name.c_str(), CXIMAGE_FORMAT_PNG );
//	cnt++;
//}
void PolygonGenerator::Slove( CxImage& srcImage, double cosThreshold, double disThreshold, CPolygonSet& polygon_set )
{
	m_cosThreshold = cosThreshold;
	m_disThreshold = disThreshold;
	printf("m_cosThreshold = %lf, m_disThreshold = %lf\n", m_cosThreshold, m_disThreshold );
	Init( srcImage );
	int* flag;
	std::vector<CPoint> boundPoints;
	std::vector<CPoint> orderPoints;
	std::vector<CPoint> polygon;
	std::vector<CPoint>::iterator ibp;
	flag = new int[m_width*m_height];
	memset( flag, 0, sizeof(int)*m_width*m_height );
	clock_t start, finish;
	double   duration;
	start = clock();
	GetBound( flag, boundPoints );
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf( "get bound %f seconds\n", duration );
	//Simplify( boundPoints );
	//Display( boundPoints, m_image, RGB( 255, 0, 0 ) );
	start = clock();
	int index;
	for ( ibp = boundPoints.begin(); ibp != boundPoints.end(); ++ibp )
	{
		index = (*ibp).y*m_width + (*ibp).x;
		if ( flag[index] == 1 )
		{
			orderPoints.clear();
			start = clock();
			GetOrderedBound( *ibp, flag, orderPoints, boundPoints.size() );
			finish = clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			//printf( "GetOrderedBound %f seconds\n", duration );
			polygon.clear();
			start = clock();
			bool flag;
			do 
			{
				flag = generate_polygon_from_contour_simple( orderPoints, polygon );
				orderPoints = polygon;
			} while ( !flag );
			//generate_polygon_from_contour( orderPoints, polygon );
			finish = clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			//printf( "generate_polygon_from_contour %f seconds\n", duration );
			if ( polygon.size() >= 3 )
			{
				CPolygon add_polygon( polygon );
				for ( int i = 0; i < add_polygon.m_vPointSet.size(); ++i )
				{
					add_polygon.m_vEdgeSet.push_back( pair<CPoint, CPoint>( add_polygon.m_vPointSet[i], add_polygon.m_vPointSet[ (i + 1) % add_polygon.m_vPointSet.size() ] ) );
				}
				polygon_set.m_polygon.push_back( add_polygon );
				//Display( polygon, m_image,	 RGB( 255, 0, 0 ) );
			}
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf( "get polygon based on bound %f seconds\n", duration );
	delete [] flag;
}

double PolygonGenerator::DisSum( const CPoint& A, const CPoint& B, const CPoint& C )
{
	return Distance( A, B ) + Distance( B, C );
}

double PolygonGenerator::Distance( const CPoint& A, const CPoint& B )
{
	return sqrt( (double)( (A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y) ) );
}
