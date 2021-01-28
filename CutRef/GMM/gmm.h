#ifndef GMM_H
#define GMM_H


typedef struct Cluster_ywz
{

    int       clusterNum;
    double    (*mean)[3];
    double ** cov;
    double *  det;
    double *  weight;

    Cluster_ywz(int num=3)
    {
        clusterNum=num;
        mean=new double[num][3];
        det=new double[num];
        weight=new double[num];
        cov=new double *[num];
        for(int i=0;i<num;++i)
            cov[i]=new double[9];
    }

    ~Cluster_ywz()
    {
        delete [] mean;
        delete [] det;
        delete [] weight;
        for(int i=0;i<clusterNum;++i)
            delete [] cov[i];
        delete [] cov;
    }

    float Prob_ywz(double data[3]);
    void  Print();

}CLUSTER_ywz;

class GMM_ywz
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

    static void Train(double data[], int n, Cluster_ywz &gmm);
    // Test
    static void TestGMM();
    static void Print(int clusterNum, double mean[][3], double pi[], 
                      double det[], double *segma[9]);

};

double Prob_ywz(double *data, double *mean, double det, double *segma);

#endif