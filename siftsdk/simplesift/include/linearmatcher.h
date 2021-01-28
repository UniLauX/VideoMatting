#pragma once

#include <vector>

namespace SIMPLESIFT
{
class LinearMatcher
{
public:
	LinearMatcher(void);
	~LinearMatcher(void);

	void init(std::vector<std::vector<float> > &kpv);
	bool find_match(std::vector<float> &kp, int nbCount, int* nbIndices, float* nbDists);

protected:
	std::vector<std::vector<float> > _keypoint;
};
}
