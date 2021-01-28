#pragma once
#include <ximage.h>
#include <vector>
#include "Polygon.h"
class PolygonGenerator
{
public:
	PolygonGenerator(void);
	~PolygonGenerator(void);
	void Init( CxImage& srcImage );
	void Slove( CxImage& srcImage, double cosThreshold, double disThreshold, CPolygonSet& polygon_set );
	void GetContour( CxImage &img, std::vector<CPoint> &contour );
	double Curvature(const CPoint &p0, const CPoint &p1, const CPoint &p2);
	double distance_2_segment(const CPoint &X, const CPoint &P, const CPoint &Q);
	double distance_2_polygon(const CPoint &pt, std::vector<CPoint> &polygon);
	bool generate_polygon_from_contour(const std::vector<CPoint> &contour, std::vector<CPoint> &polygon);
	bool Simplify(std::vector<CPoint> &contour);
	bool generate_polygon_from_contour_simple(const std::vector<CPoint> &contour, std::vector<CPoint> &polygon);
	void GetOrderedBound(const CPoint &sPoint, int *flag, std::vector<CPoint> &points, int num);
	int GetNeighbourValue(int x, int y, int dir, int &xtemp, int &ytemp);
	void GetBound(int *flag, std::vector<CPoint> &points);
	int DistanceSqr( const CPoint &A, const CPoint &B );
	int DotProduct( const CPoint &A, const CPoint &B );
	int DotPerpendicularProduct( const CPoint &A, const CPoint &B );
	double Position_U(const CPoint &X, const CPoint &P, const CPoint &Q);
	double Distance_V(const CPoint &X, const CPoint &P, const CPoint &Q);
	double DisSum( const CPoint& A, const CPoint& B, const CPoint& C );
	double Distance( const CPoint& A, const CPoint& B );
private:
	int m_width;
	int m_height;
	unsigned char* m_alpha;
	CxImage m_image;
	double m_cosThreshold;
	double m_disThreshold;
};
