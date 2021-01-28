#include "Beizer.h"
#include <iostream>
using namespace std;

Beizer::Beizer()
{
	numofControlPoint = 0;
	controlPoint.clear();
}

Beizer::Beizer( int numofControlPoint, CPoint * controlPoint )
{
	this->numofControlPoint = numofControlPoint;
	for (int i = 0; i < this->numofControlPoint; ++i)
	{
		this->controlPoint.push_back(controlPoint[i]);
	}
}

Beizer::~Beizer()
{
	controlPoint.clear();

}


void Beizer::addControlPoint( float x, float y )
{
	this->controlPoint.push_back(CPoint(x, y));	
	numofControlPoint++;
}

bool Beizer::removeControlPoint( float x, float y )
{
	vector<CPoint>::iterator iter;
	for (iter = controlPoint.begin(); iter != controlPoint.end(); iter++)
	{
		if (iter->x == x && iter->y == y)
		{
			this->controlPoint.erase(iter);
			return true;
		}
	}
	return false;
}

CPoint Beizer::getPoint( float t )
{//假定三次曲线
	
	CPoint tp(-1,-1);
	if (numofControlPoint == 4)
	{
		double coeff0 = (1-t)*(1-t)*(1-t);
		double coeff1 = 3*(1-t)*(1-t)*t;
		double coeff2 = 3*(1-t)*t*t;
		double coeff3 = t*t*t;

		

		tp.x = (LONG)(coeff0 * controlPoint[0].x + coeff1 * controlPoint[1].x + coeff2 * controlPoint[2].x + coeff3 * controlPoint[3].x + 0.5);
		tp.y = (LONG)(coeff0 * controlPoint[0].y + coeff1 * controlPoint[1].y + coeff2 * controlPoint[2].y + coeff3 * controlPoint[3].y + 0.5);

	}
	else
	{
		printf("Oh, no, it is not cubic curve!\n");
	}
	return tp;
}

void Beizer::clearControlPoint()
{
	controlPoint.clear();
	numofControlPoint = 0;
}


