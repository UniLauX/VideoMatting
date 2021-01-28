#pragma once
#include "windows.h"
#include <vector>
class UserScribble
{
	//typedef struct  
	//{
	//	int x;
	//	int y;
	//}POINT;
public:
	UserScribble(void);
	~UserScribble(void);

	int forgptsize();
	int backgptsize();
	void addForgPt(int x, int y);
	void addBackgPt(int x, int y);
	POINT ForgPt(int index);
	POINT BackgPt(int index);
	void clear();

private:
	std::vector<POINT> forground_points;
	std::vector<POINT> background_points;

};
