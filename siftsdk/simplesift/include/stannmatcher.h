#pragma once

#include <dpoint.hpp>
#include <sfcnn.hpp>

#include <vector>

namespace SIMPLESIFT
{
class STANNMatcher
{
public:
	STANNMatcher(void);
	~STANNMatcher(void);

	void init(std::vector<std::vector<float> > &kpv);
	bool find_match(const std::vector<float> &kp, int nbCount, int* nbIndices, double* nbDists);

protected:
	std::vector<std::vector<float> > _keypoint;
  
	typedef reviver::dpoint<float, 64> Point;

    sfcnn<Point, 64, float> *_SFC;
	std::vector<Point> _points;
};
}
