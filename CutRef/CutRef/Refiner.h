#ifndef TRINOVIDEODEPTH_H
#define TRINOVIDEODEPTH_H

#include "Video.h"
#include <set>

class Refiner
{
    SEQ           video;
    string        outDir;
     string moveDir;
     string optDir;
	 string trimapDir;

public:

    int gmmWin;         // Temporal refine: learning GMM.
    int boundWin;       // Temporal refine: boundary size.

private:

    void OptRefineCuttingProb(int frame, float *res, int win=3);

public:

    Refiner();
	Refiner(string _outDir, string _moveDir, string _optDir, string _trimapDir);
    ~Refiner();

    void SetOutputDir(string dir);
    void SetVideo(string name, int start, int end, int optStart, int optEnd);

    void GMMRefineCutting();
	void Trimap_Common(CxImage& trimap, CxImage& newTrimap);
};

#endif