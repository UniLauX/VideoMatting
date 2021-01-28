#ifndef GMM_H
#define GMM_H


class GMM
{

private:

    static void NewGMM(double data[], int dataNum, int clusterNum, 
                       double mean[][3], double pi[], double det[], double *segma[9],
                       int label[]);

public:

    static int times;

public:

    static void InitGMM(double data[], int dataNum, int clusterNum, 
                        double mean[][3], double pi[], double det[], double *segma[9]);

    static void GetGMM(double data[], int dataNum, int clusterNum, 
                       double mean[][3], double pi[], double det[], double *segma[9]);

    static void GetLabel(double data[], int dataNum, int clusterNum, 
                         double mean[][3], double pi[], double det[], double *segma[9],
                         int label[]);

    // Test
    static void TestGMM();
    static void Print(int clusterNum, double mean[][3], double pi[], 
                      double det[], double *segma[9]);

};

double Prob(double *data, double *mean, double det, double *segma);

#endif