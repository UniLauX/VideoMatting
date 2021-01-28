#ifndef LOCALCLASSIFIER_H
#define LOCALCLASSIFIER_H

#include "../include/Wml/WmlMath.h"
#include "../include/CxImage/ximage.h"
#include "../include/Graphcut/graph.h"
#include "../GMM/gmm.h"
#include <vector>

typedef std::vector<POINT> VPOINTS;
typedef std::vector<VPOINTS> VVPOINTS;

typedef struct ClassifierParam_t
{

    int    boundDis;
    double fcutoff;
    double eMin;
    double foreThres;
    double backThres;

    ClassifierParam_t()
    {
        boundDis=6;
        fcutoff=0.85;
        eMin=2;
        foreThres=0.75;
        backThres=0.25;
    }

}CLASSIFIERPARAM;

class LocalClassifier_ywz
{
     int       cx,cy;
     int       x1,y1;
     int       x2,y2;
     int       winSize;
     int       ptsNum;
     
     // Get them from the video cut class
     int *     label;
     CxImage * image;

     // Color model
     CLUSTER_ywz *	bgCluster;
     CLUSTER_ywz *	fgCluster;

     // Color Probability
     double *  Pc;

     // All probability
     double *  Pf;
     double *  Pb;

     // Test
     bool      valid;

public:

     static CLASSIFIERPARAM param;

private:

     void Init();
     void Clear();
     void InitClassifier();
     void GetColorProb();
     void GetModelProb();

public:

     LocalClassifier_ywz();
     ~LocalClassifier_ywz();

     /************************************************************************/
     /* Can be called only once!                                             */
     /************************************************************************/
     void InitParam(int x, int y, int winSize, int *label, CxImage *pic);
     /************************************************************************/
     /* Set new parameter for a local window.                                */
     /************************************************************************/
     void SetParam(int x, int y, int *label, CxImage *frame);
     /************************************************************************/
     /* Initialize GMMs and other information.                               */
     /************************************************************************/
     void InitModel();
     void GetModelParam();
     void AddProbValues(double *F, double *B, double *P);

     inline int  GetWidth() { return x2-x1; }
     inline int  GetHeight() { return y2-y1; }
     inline void GetCenter(int &x, int &y) { x=cx, y=cy; }
     inline void GetLeftCorner(int &x, int &y) { x=x1, y=y1; }
     inline void GetRightCorner(int &x, int &y) { x=x2, y=y2; }
     
     // Test
     void TestDis(CxImage *image);
     void Test(CxImage *image);
     void TestPos(CxImage *image);
};


#endif