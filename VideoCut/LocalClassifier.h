#ifndef LOCALCLASSIFIER_H
#define LOCALCLASSIFIER_H
#include <boost/thread/thread.hpp>


#include <boost/unordered/unordered_map.hpp>
#include "../GrabCut/Global.h"
#include "../ClosedFormMatting/Matting.h"
#include <vector>
#include <map>
#include "../VideoEditing/Point2d.h"
#include <hash_map>
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
        boundDis=6;// for large 30 
		//boundDis = 30;
        fcutoff=0.85;
        eMin=2;//for large 8
		//eMin = 8;
        foreThres=0.75;
        backThres=0.25;
    }

}CLASSIFIERPARAM;

class LocalClassifier
{
     int       cx,cy;
     int       x1,y1;
     int       x2,y2;
     int       winSize;
     int       ptsNum;
     
     // Get them from the video cut class
     //int *     label;
     unsigned char* label;
     CxImage * image;

     // Color model
     CLUSTER *	bgCluster;
     CLUSTER *	fgCluster;
  
     // Boundary
     VPOINTS   boundPts;
     int *     dis;

     // Color Probability
     double *  Pc;
     double    Fc;

     // Shape Params
     double    es;
     double *  Fs;

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
     int  GetMinDistance(int x, int y);
     void InitClassifier(int mode=3, int type=0);
     void GetColorProb();
     // The parameter a in the equation
     void GetShapeParam(double a);
     void GetModelProb(bool shape=true);

public:

     LocalClassifier();
     ~LocalClassifier();

     /************************************************************************/
     /* Can be called only once!                                             */
     /************************************************************************/
     void InitParam(int x, int y, int winSize, unsigned char *label, CxImage *pic);
     /************************************************************************/
     /* Set new parameter for a local window.                                */
     /************************************************************************/
     void SetParam(int x, int y, unsigned char *label, CxImage *frame);
     void Move(unsigned char *label, CxImage *frame, VVPOINTS &winCenters);
     /************************************************************************/
     /* Initialize GMMs and other information.                               */
     /************************************************************************/
     void InitModel(int mode=3, bool shape=true, int type=0);
     void GetModelParam(bool shape=true);
     void AddProbValues(double *F, double *B, double *P);
     void AddProbValues(std::map<std::pair<int, int>, double>& F, std::map<std::pair<int ,int>, double>& B, std::map<std::pair<int, int>, double>& P);
	 void AddProbValues(stdext::hash_map<int, double>& F, stdext::hash_map<int, double>& B, stdext::hash_map<int, double>& P);

     inline int  GetWidth();
     inline int  GetHeight();
     inline void GetCenter(int &x, int &y) { x=cx, y=cy; }
     inline void GetLeftCorner(int &x, int &y) { x=x1, y=y1; }
     inline void GetRightCorner(int &x, int &y) { x=x2, y=y2; }
     void DoMatting(Matting &matting, double *alpha, int *count);
     
     // Test
     void TestDis(CxImage *image);
     void TestFs(CxImage *image);
     void Test(CxImage *image);
     void TestPos(CxImage *image);
};


#endif