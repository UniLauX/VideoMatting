#pragma once
#include <vector>
#include <fstream>

class CSimpleKeypoint
{
public:
	CSimpleKeypoint(void);
	~CSimpleKeypoint(void);

	float x;
	float y;
	float rotation;
	float scale;
	float dvalue;
	std::vector<float> descriptor;
};


std::ostream& operator<<(std::ostream &os, CSimpleKeypoint const &kp);

std::istream& operator>>(std::istream &is, CSimpleKeypoint &kp);