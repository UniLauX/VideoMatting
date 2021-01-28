#include "stdafx.h"
#include "UserScribble.h"
UserScribble::UserScribble(void)
{
}

UserScribble::~UserScribble(void)
{
}

int UserScribble::forgptsize()
{
	return (int)forground_points.size();
}

int UserScribble::backgptsize()
{
	return (int)background_points.size();
}

POINT UserScribble::ForgPt(int index)
{
	return forground_points[index];
}
POINT UserScribble::BackgPt(int index)
{
	return background_points[index];
}
void UserScribble::clear()
{
	forground_points.clear();
	background_points.clear();
}

void UserScribble::addBackgPt(int x, int y)
{
	POINT p;
	p.x = x;
	p.y = y;
	background_points.push_back(p);
}

void UserScribble::addForgPt(int x, int y)
{
	POINT p;
	p.x = x;
	p.y = y;
	forground_points.push_back(p);
}