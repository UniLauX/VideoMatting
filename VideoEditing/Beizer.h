#ifndef BEIZER_H
#define BEIZER_H
#include <vector>
#include "stdafx.h"
using namespace std;

class Beizer
{
public:
	Beizer();
	~Beizer();
	Beizer(int numofControlPoint, CPoint * controlPoint);
	void addControlPoint(float x, float y);
	bool removeControlPoint(float x, float y);
	CPoint getPoint(float index);
	void clearControlPoint();

private:
	vector<CPoint> controlPoint;
	int numofControlPoint;
};







#endif