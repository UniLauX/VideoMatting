#pragma once

//#include "simpleimage.h"

#include <vector>
#include "simpleimage.h"

struct PeakPos
{
  PeakPos(float x = 0, float y = 0, float peak_value = 0.0f)
    :fx(x), fy(y), peak_value(peak_value){}
  float fx, fy;
  float peak_value;
};

struct CvMat;

inline bool PeakPosGreator(PeakPos& a, PeakPos& b)
{
  return a.peak_value > b.peak_value;
}

class HarrisCornerDetector
{
public:
  HarrisCornerDetector(void);
public:
  ~HarrisCornerDetector(void);

  void detect_limited(CSimpleImagef &, std::vector<PeakPos> &, int max_peak_count, float threshold = 0.0f);

protected:
 
  void detect(CSimpleImagef &, std::vector<PeakPos> &, float threshold = 0.0f);

  void local_maximum_thresholded(CvMat&, std::vector<PeakPos>&, float threshold);
};
