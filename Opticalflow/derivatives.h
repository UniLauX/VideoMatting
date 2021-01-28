extern void laplacian(float* image,float *lap,int nx,int ny);
extern void laplacian2(float* image,float *lap,int nx,int ny);
extern void horn_derives(float* image1,float* image2,float* dx,float* dy,float* dt,int nx,int ny);
extern void horn_dy(float* image1,float* image2,float* dy,int nx,int ny);
extern void horn_dx(float* image1,float* image2,float* dx,int nx,int ny);
extern void horn_dt(float* image1,float* image2,float* dt,int nx,int ny);
