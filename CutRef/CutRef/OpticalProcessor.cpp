#include "OpticalProcessor.h"
#include "Global.h"
#include <iostream>
#include <algorithm>

using namespace std;

OpticalProcessor::OpticalProcessor()
{
    optdir="F:/data/opt/";
    depthdir="F:/data/depth/";
    labeldir="F:/data/move/";
}

OpticalProcessor::~OpticalProcessor()
{

}

void OpticalProcessor::SetVideo(SEQ &seq)
{
    video=seq;
}

void OpticalProcessor::SetDir(string opt, string depth, string cutdir)
{
    optdir=opt;
    depthdir=depth;
    labeldir=cutdir;
}

bool OpticalProcessor::LoadOpticalflow(int from, int to, float *u, float *v)
{
  //  int   w,h;
    float *t1,*t2;
    char  uname[64];
    char  vname[64];
    CxImage A,B;

    sprintf_s(uname, "%su_%d_%d.raw", optdir.c_str(), from, to);
    sprintf_s(vname, "%sv_%d_%d.raw", optdir.c_str(), from, to);
    t1=LoadData(uname, w, h);
    t2=LoadData(vname, w, h);
	printf("%s\n", uname);
	printf("%s\n", vname);
    if(t1==NULL || t2==NULL)
    {
        if(!video.LoadImage(from, A) ||
            !video.LoadImage(to, B))
        {
            delete [] t1;
            delete [] t2;
            return false;
        }

        OpticalFlow(A, B, u, v);

        w=A.GetWidth();
        h=A.GetHeight();
        SaveData(uname, w, h, u);
        SaveData(vname, w, h, v);

        CxImage image;
        sprintf_s(uname, "%s/u_%d_%d.jpg", optdir.c_str(), from, to);
        sprintf_s(vname, "%s/v_%d_%d.jpg", optdir.c_str(), from, to);
        GetResult(video.width, video.height, -1, u, image);
        image.Save(uname, CXIMAGE_FORMAT_JPG);
        GetResult(video.width, video.height, -1, v, image);
        image.Save(vname, CXIMAGE_FORMAT_JPG);
    }
    else
    {
        memcpy(u, t1, sizeof(float)*w*h);
        memcpy(v, t2, sizeof(float)*w*h);
        delete [] t1;
        delete [] t2;
    }
    return true;
}

inline static float _CheckOptError(int x, int y, int w, int h, const float *u1, 
                                   const float *v1, const float *u2, const float *v2)
{
    int    i,a,b;

    if(x<0 || x>=w || y<0 || y>=h)
        return 1.0f;

    i=x+y*w;
    a=Near(x+u1[i]);
    b=Near(y+v1[i]);
    if(a>=0 && a<w && b>=0 && b<h)
    {
        i=a+b*w;
        a=Near(a+u2[i]-x);
        b=Near(b+v2[i]-y);
        return sqrt((float)a*a+b*b);
    }
    else
        return 100.0f;
}

bool OpticalProcessor::CheckOptError(int x, int y, const float *u1, 
                                     const float *v1, const float *u2, const float *v2)
{
    float t=_CheckOptError(x, y, video.width, video.height, u1, v1, u2, v2);

    if(t>3.0f)
        return false;
    else
        return true;
}

inline void VecAdd(int N, const float *a, float *r)
{
    int i;

    #pragma omp parallel for
    for(i=0;i<N;++i)
        r[i]+=a[i];
}

static bool Compare(float a, float b)
{
    return a<b;
}

void OpticalProcessor::LoadFrameInfo(int num, vector<FrameInfo> &info, float *conf, int win)
{
    int   n,i,di;
    int   x,y,k;
    int   f1,f2;
    int   w,h;
    float *u,*v;
    float *u2,*v2;
    float *us,*vs;
    float *us2,*vs2;
    CxImage A,B,img;

    n=video.width*video.height;
    w=video.width;
    h=video.height;
    u=new float[n];
    v=new float[n];
    u2=new float[n];
    v2=new float[n];
    us=new float[n];
    vs=new float[n];
    us2=new float[n];
    vs2=new float[n];

    video.LoadImage(num, A);

    FrameInfo f;
    f.u    =new float[n];
    f.v    =new float[n];
    f.r    =new float[n];
    f.label=new int[n];
    f.num=num;
    memset(f.u, 0, sizeof(float)*n);
    memset(f.v, 0, sizeof(float)*n);
    memset(f.r, 0, sizeof(float)*n);
    //LoadImage(labeldir.c_str(), "bmp", f.num, img);
	LoadImage((labeldir+video.fileTitle).c_str(), "png", img);

    ImageToLabel(img, f.label);
    video.LoadImage(f.num, f.image);
    info.push_back(f);

    for(di=-1;di<2;di+=2)
    {
        memset(us, 0, sizeof(float)*n);
        memset(vs, 0, sizeof(float)*n);
        memset(us2, 0, sizeof(float)*n);
        memset(vs2, 0, sizeof(float)*n);

        for(i=0;i<win;++i)
        {
            f1=num+i*di;
            f2=f1+di;
			if(f1<video.start || f1>=video.end || f2<video.start || f2>=video.end)
				break;

            if(!LoadOpticalflow(f1, f2, u, v) ||
               !LoadOpticalflow(f2, f1, u2, v2))
                break;

            cout << "Adding " << f2 << endl;

            video.LoadImage(f2, B);

            VecAdd(n, u, us);
            VecAdd(n, v, vs);
            VecAdd(n, u2, us2);
            VecAdd(n, v2, vs2);
            
            FrameInfo f;
            f.u    =new float[n];
            f.v    =new float[n];
            f.r    =new float[n];
            f.label=new int[n];
            f.num=f2;
            memcpy(f.u, us, sizeof(float)*n);
            memcpy(f.v, vs, sizeof(float)*n);
            LoadImage(labeldir.c_str(), "png", f.num, img);
            ImageToLabel(img, f.label);
            video.LoadImage(f.num, f.image);
            info.push_back(f);

            for(y=0,k=0;y<video.height;++y)
            {
                for(x=0;x<video.width;++x,++k)
                {
                    int a=Near(x+us[k]);
                    int b=Near(y+vs[k]);
                    if(a>=0 && a<video.width && b>=0 && b<video.height)
                    {
                        float s=a+us2[a+b*video.width];
                        float t=b+vs2[a+b*video.width];
                        f.r[x+y*video.width]=sqrt((s-x)*(s-x)+(t-y)*(t-y));
                    }
                }
            }
        }
    }
    delete [] u;
    delete [] v;
    delete [] u2;
    delete [] v2;
    delete [] us;
    delete [] vs;
    delete [] us2;
    delete [] vs2;
}

void OpticalProcessor::LoadFrameInfo_jin(vector<OptData> & label, int num, float *conf, int win /* = 2 */)
{
	int   n,i,j,di;
	int   x,y,k;
	int   f1,f2;
	int   w,h;
	float *u,*v;
	float *u2,*v2;
	float *us,*vs;
	float *us2,*vs2;
	CxImage A,B,img;

	n=video.width*video.height;
	w=video.width;
	h=video.height;
	u=new float[n];
	v=new float[n];
	u2=new float[n];
	v2=new float[n];
	us=new float[n];
	vs=new float[n];
	us2=new float[n];
	vs2=new float[n];

	video.LoadImage(num, A);

	FrameInfo f;
	f.u    =new float[n];
	f.v    =new float[n];
	f.r    =new float[n];
	f.label=new int[n];
	f.num=num;
	memset(f.u, 0, sizeof(float)*n);
	memset(f.v, 0, sizeof(float)*n);
	memset(f.r, 0, sizeof(float)*n);
	LoadImage((labeldir+video.fileTitle).c_str(), "png", img);
	ImageToLabel(img, f.label);
	video.LoadImage(f.num, f.image);

//add
	OptData t;
	n=video.width*video.height;
	t.Create(n);
	t.num=f.num;
	memcpy(t.r, f.r, sizeof(float)*n);
	int a,b;
	for(i=0,k=0;i<video.height;++i)
	{
		for(j=0;j<video.width;++j,++k)
		{
			a=Near(j+f.u[k]);
			b=Near(i+f.v[k]);
			if(a>=0 && a<video.width && b>=0 && b<video.height)
				t.data[k]=(float)f.label[a+b*video.width];
			else
				t.data[k]=-1;
		}   
	}
	label.push_back(t);
	f.Destroy();
//add
	for(di=-1;di<2;di+=2)
	{
		memset(us, 0, sizeof(float)*n);
		memset(vs, 0, sizeof(float)*n);
		memset(us2, 0, sizeof(float)*n);
		memset(vs2, 0, sizeof(float)*n);

		for(i=0;i<win;++i)
		{
			f1=num+i*di;
			f2=f1+di;
			if(f1<video.optStart || f1>=video.optEnd || f2<video.optStart || f2>=video.optEnd)
				break;

			if(!LoadOpticalflow(f1, f2, u, v) ||
				!LoadOpticalflow(f2, f1, u2, v2))
				break;

			cout << "Adding " << f2 << endl;

			video.LoadImage(f2, B);

			VecAdd(n, u, us);
			VecAdd(n, v, vs);
			VecAdd(n, u2, us2);
			VecAdd(n, v2, vs2);

			FrameInfo f;
			f.u    =new float[n];
			f.v    =new float[n];
			f.r    =new float[n];
			f.label=new int[n];
			f.num=f2;
			memcpy(f.u, us, sizeof(float)*n);
			memcpy(f.v, vs, sizeof(float)*n);
			LoadImage(labeldir.c_str(), "png", f.num, img);
			ImageToLabel(img, f.label);
			video.LoadImage(f.num, f.image);
			for(y=0,k=0;y<video.height;++y)
			{
				for(x=0;x<video.width;++x,++k)
				{
					int a=Near(x+us[k]);
					int b=Near(y+vs[k]);
					if(a>=0 && a<video.width && b>=0 && b<video.height)
					{
						float s=a+us2[a+b*video.width];
						float t=b+vs2[a+b*video.width];
						f.r[x+y*video.width]=sqrt((s-x)*(s-x)+(t-y)*(t-y));
					}
				}
			}
			OptData t;
			n=video.width*video.height;
			t.Create(n);
			t.num=f.num;
			memcpy(t.r, f.r, sizeof(float)*n);
			int i, j;
			int a,b;
			for(i=0,k=0;i<video.height;++i)
			{
				for(j=0;j<video.width;++j,++k)
				{
					a=Near(j+f.u[k]);
					b=Near(i+f.v[k]);
					if(a>=0 && a<video.width && b>=0 && b<video.height)
						t.data[k]=(float)f.label[a+b*video.width];
					else
						t.data[k]=-1;
				}   
			}
			label.push_back(t);
			f.Destroy();
		}
	}
	delete [] u;
	delete [] v;
	delete [] u2;
	delete [] v2;
	delete [] us;
	delete [] vs;
	delete [] us2;
	delete [] vs2;
}

void OpticalProcessor::LoadOptLabel(int num, int win, vector<OptData> &label)
{
    int i,j;
    int k,n;
    int a,b;
    vector<FrameInfo> info;

    for(vector<OptData>::iterator it=label.begin();it!=label.end();++it)
        it->Destroy();
    label.clear();

    LoadFrameInfo(num, info, NULL, win);
    for(vector<FrameInfo>::iterator it=info.begin();it!=info.end();++it)
    {
        OptData t;
        n=video.width*video.height;
        t.Create(n);
        t.num=it->num;
        memcpy(t.r, it->r, sizeof(float)*n);
        for(i=0,k=0;i<video.height;++i)
        {
            for(j=0;j<video.width;++j,++k)
            {
                a=Near(j+it->u[k]);
                b=Near(i+it->v[k]);
                if(a>=0 && a<video.width && b>=0 && b<video.height)
                    t.data[k]=(float)it->label[a+b*video.width];
                else
                    t.data[k]=-1;
            }   
        }
        label.push_back(t);
    }
    for(vector<FrameInfo>::iterator i=info.begin();i!=info.end();++i)
        i->Destroy();
}

void OpticalProcessor::LoadOptLabel_jin(int num, int win, vector<OptData> &label)
{
	for(vector<OptData>::iterator it=label.begin();it!=label.end();++it)
		it->Destroy();
	label.clear();
	LoadFrameInfo_jin(label, num,NULL, win );
}
