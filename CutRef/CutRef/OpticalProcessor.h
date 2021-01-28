#ifndef MOVING_OBJS_DETECTOR_H
#define MOVING_OBJS_DETECTOR_H

#include "Video.h"
#include "Global.h"
#include <vector>
#include <string>

using std::string;
using std::vector;

class OpticalProcessor
{
    
    SEQ         video;
    string      optdir;
    string      depthdir;
    string      labeldir;
	int w, h;

private:

    bool  LoadOpticalflow(int from, int to, float *u, float *v);
    bool  CheckOptError(int x, int y, const float *u1, const float *v1, const float *u2, const float *v2);

private:

    struct FrameInfo
    {
        int     num;
        float * u;
        float * v;
        float * r;
        int *   label;
        CxImage image;

        FrameInfo():num(-1),u(NULL),v(NULL),r(NULL),label(NULL){};

        void Destroy()
        {
            delete [] u;
            delete [] v;
            delete [] r;
            delete [] label;
        }
    };

    void LoadFrameInfo(int num, vector<FrameInfo> &info, float *conf, int win=2);
	void LoadFrameInfo_jin(vector<OptData> & label, int num, float *conf, int win = 2);

public:

    OpticalProcessor();
	OpticalProcessor(int _w, int _h)
		:w(_w)
		,h(_h){};

    ~OpticalProcessor();

    void SetVideo(SEQ &seq);
    void SetDir(string opt, string depth, string cutdir="F:/data/move/");

    void LoadOptLabel(int num, int win, vector<OptData> &depth);
	void LoadOptLabel_jin(int num, int win, vector<OptData> &depth);
};


#endif
