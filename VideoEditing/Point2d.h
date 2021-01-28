#pragma once

#include <boost/functional/hash.hpp>
class point2
{
public:
	int x;
	int y;
	double value;

public:
	point2() : x(0), y(0), value(0) {}
	point2(int _x, int _y) : x(_x), y(_y){}
	point2(int _x, int _y, double _value) : x(x), y(y), value(value) {}

	bool operator==(point2 const& other) const
	{
		return x == other.x && y == other.y;
	}

	friend std::size_t hash_value(point2 const& p)
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, p.x);
		boost::hash_combine(seed, p.y);
		return seed;
	}
};
