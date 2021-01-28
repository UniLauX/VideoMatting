
#include "TemporalRefiner.h"
extern "C"
#include "../Opticalflow/calflow.h"

#define  NDATA_COST

CTemporalRefiner::CTemporalRefiner(void)
{
	graph=new Graph;
	index=NULL;
	gnode=NULL;
	w=0;h=0;
}

CTemporalRefiner::~CTemporalRefiner(void)
{
	if (gnode)
	{
		delete[] gnode;
	}
	delete graph;

	if (index)
	{
		for(int i=0;i<10;++i)
		{
			delete[] index[i];
			index[i]=NULL;
		}
		delete[] index;
		index=NULL;
	}	
}

void CTemporalRefiner::SampleColor(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange,int trimapWidth)
{
	int pos=alphaBuffer->GetFramePos();

	CxImage* label;
	CxImage trimap;
	CxImage* colorImage;

	vector<POINT> bound;
	RGBQUAD rgb;
	rgb.rgbBlue=rgb.rgbGreen=rgb.rgbRed=128;
	RGBQUAD rgb2;
	RGBQUAD rgb3;
	
	label=alphaBuffer->GetImage();
	colorImage=imageBuffer->GetImage();

	this->beta=this->GetBeta(colorImage);

	w=label->GetWidth();
	h=label->GetHeight();

	//
	gnode=new Graph::node_id[5*w*h];
	for (int i=0;i<5*w*h;++i)
	{
		gnode[i]=graph->add_node();
	}

	int count=refineRange;

	cout<<pos<<"in sample"<<endl;

	int forcount=0;
	int backcount=0;

       do 
	{
		label=alphaBuffer->GetImage();//label
		colorImage=imageBuffer->GetImage();//source image
		trimap.Copy(*label);//trimap image

		BiImageProcess::getBound(bound,label);
		int size=(int )bound.size();
		int x,y;
		for (int i=0;i<size;++i)
		{
			x=bound[i].x;
			y=bound[i].y;

			for (int k=y-trimapWidth;k<=y+trimapWidth;++k)
			{
				for(int p=x-trimapWidth;p<=x+trimapWidth;++p)
				{
					if (k>=0&&k<h&&p>=0&&p<w)
					{
						if (BiImageProcess::distance(p,k,x,y)<=trimapWidth)
						{
							rgb2=label->GetPixelColor(p,k);
							rgb3=colorImage->GetPixelColor(p,k);

							colorNode co;
							co.indexX=p;
							co.indexY=k;
							co.colorinfo.resize(3,1);
							co.colorinfo(0,0)=(double)rgb3.rgbRed;
							co.colorinfo(1,0)=(double)rgb3.rgbGreen;
							co.colorinfo(2,0)=(double)rgb3.rgbBlue;

							if (rgb2.rgbBlue==255&&BiImageProcess::distance(p,k,x,y)>5&&BiImageProcess::distance(p,k,x,y)<=7)
							{
								fortempvec.push_back(co);
								forcount++;
							}
							else if(rgb2.rgbBlue==0&&BiImageProcess::distance(p,k,x,y)>5&&BiImageProcess::distance(p,k,x,y)<=7)
							{
								backtempvec.push_back(co);
								backcount++;
							}

							trimap.SetPixelColor(p,k,rgb);

						}
					}
				}
			}

		}
		trimap.Save(trimapBuffer->GetFrameName(),CXIMAGE_FORMAT_BMP);
		count--;
	} while (alphaBuffer->Forward()&&trimapBuffer->Forward()&&imageBuffer->Forward()&&count);//color samples and trimap generation

	//reset imagebuffer
	alphaBuffer->Goto(pos);
	trimapBuffer->Goto(pos);
	imageBuffer->Goto(pos);

	vector< vector < colorNode > > forvec;
	vector< vector < colorNode > > backvec;
	forvec.push_back(fortempvec);
	backvec.push_back(backtempvec);
	cluster(forvec);
	cluster(backvec);

	cout<<"total:"<<forcount<<" "<<backcount<<endl;

	initGMM(forvec,fgmm);
	initGMM(backvec,bgmm);

}

void CTemporalRefiner::UpdateGmmParam()
{
	int fsize=fortempvec.size();

	vector<RGBQUAD> *vec_f=new vector<RGBQUAD>[COMNUM];
	RGBQUAD frgb;
	for (int i=0;i<fsize;++i)
	{
		frgb.rgbRed=fortempvec[i].colorinfo(0,0);
		frgb.rgbGreen=fortempvec[i].colorinfo(1,0);
		frgb.rgbGreen=fortempvec[i].colorinfo(2,0);

		float poss=-1;
		int maxindex=-1;
		for (int j=0;j<COMNUM;++j)
		{
			float temposs=CalForPoss(j,frgb);
			if (temposs>poss)
			{
				poss=temposs;
				maxindex=j;
			}
		}
		vec_f[maxindex].push_back(frgb);

	}

	for (int n=0;n<COMNUM;++n)
	{
		CalGmmParam(fgmm[n],vec_f[n],fsize);
	}

	int bsize=backtempvec.size();

	vector<RGBQUAD> *vec_b=new vector<RGBQUAD>[COMNUM];
	RGBQUAD brgb;
	for (int i=0;i<bsize;++i)
	{
		brgb.rgbRed=backtempvec[i].colorinfo(0,0);
		brgb.rgbGreen=backtempvec[i].colorinfo(1,0);
		brgb.rgbBlue=backtempvec[i].colorinfo(2,0);

		float poss=-1;
		float maxindex=-1;

		for (int j=0;j<COMNUM;++j)
		{
			float temposs=CalBackPoss(j,brgb);
			if (temposs>poss)
			{
				poss=temposs;
				maxindex=j;
			}
		}

		vec_b->push_back(brgb);
	}
	for(int n=0;n<COMNUM;++n)
	{
		CalGmmParam(bgmm[n],vec_b[n],bsize);
	}

	delete[] vec_b;
	delete[] vec_f;
}

void CTemporalRefiner::UpdateGmmParam(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange)
{
	int pos=alphaBuffer->GetFramePos();


	CxImage label;
	CxImage trimap;
	CxImage colImage;

	vector<RGBQUAD> *vec_b=new vector<RGBQUAD>[COMNUM];
	vector<RGBQUAD> *vec_f=new vector<RGBQUAD>[COMNUM];

	RGBQUAD trgb,crgb,lrgb;
	int fcount=0;
	int bcount=0;
	for (int i=0;i<refineRange&&alphaBuffer->Forward();++i)
	{
		alphaBuffer->Back();
		label.Copy(*(alphaBuffer->GetImage()));
		alphaBuffer->Forward();
		trimap.Copy(*(trimapBuffer->GetImage()));
		trimapBuffer->Forward();
		colImage.Copy(*(imageBuffer->GetImage()));
		imageBuffer->Forward();


		for (int p=0;p<w;++p)
		{
			for(int k=0;k<h;++k)
			{
				trgb=trimap.GetPixelColor(p,k);
				lrgb=label.GetPixelColor(p,k);

				if (trgb.rgbBlue==128&&lrgb.rgbBlue==255)//for
				{
					fcount++;
					crgb=colImage.GetPixelColor(p,k);
					float poss=-1;
					int maxindex=-1;
					for (int j=0;j<COMNUM;++j)
					{
						float temposs=CalForPoss(j,crgb);
						if (temposs>poss)
						{
							poss=temposs;
							maxindex=j;
						}
					}

					vec_f[maxindex].push_back(crgb);

				}

				if (trgb.rgbBlue==128&&lrgb.rgbBlue==0)//back
				{
					bcount++;
					crgb=colImage.GetPixelColor(p,k);

					float poss=-1;
					int maxindex=-1;

					for (int j=0;j<COMNUM;++j)
					{
						float temposs=CalBackPoss(j,crgb);
						if (temposs>poss)
						{
							poss=temposs;
							maxindex=j;
						}
					}
					vec_b[maxindex].push_back(crgb);

				}
			}
		}
	}

	alphaBuffer->Goto(pos);
	trimapBuffer->Goto(pos);
	imageBuffer->Goto(pos);

	for(int n=0;n<COMNUM;++n)
	{
		CalGmmParam(bgmm[n],vec_b[n],bcount);
		CalGmmParam(fgmm[n],vec_f[n],fcount);
	}
}
void CTemporalRefiner::CalGmmParam(GMMNode &gmm,vector<RGBQUAD> vec,int total)
{
	double sum[3];
	sum[0]=0;
	sum[1]=0;
	sum[2]=0;

	gmm.count=vec.size();
	if (vec.size()==0)
	{
		gmm.pi=0;
	}
	else
	{
		RGBQUAD rgb;
		double temp[3];

		for (int i=0;i<vec.size();++i)
		{

			rgb=vec[i];
			sum[0]+=(double)rgb.rgbRed;
			sum[1]+=(double)rgb.rgbGreen;
			sum[2]+=(double)rgb.rgbBlue;

		}


		double inv=1.0/(double)vec.size();
		gmm.u(0,0)=sum[0]*inv;
		gmm.u(1,0)=sum[1]*inv;
		gmm.u(2,0)=sum[2]*inv;


		double inv2=1.0/total;
		gmm.pi=vec.size()*inv2;
		dMatrix conviance(3,3,0.0);
		for (int i=0;i<vec.size();i++)
		{
			rgb=vec[i];

			temp[0]=(double)rgb.rgbRed;
			temp[1]=(double)rgb.rgbGreen;
			temp[2]=(double)rgb.rgbBlue;

			temp[0]=temp[0]-gmm.u(0,0);
			temp[1]=temp[1]-gmm.u(1,0);
			temp[2]=temp[2]-gmm.u(2,0);

			//conviance+=(temp-gmm.u)*(~(temp-gmm.u));

			conviance(0,0)+=temp[0]*temp[0];
			conviance(0,1)+=temp[0]*temp[1];
			conviance(0,2)+=temp[0]*temp[2];

			conviance(1,0)+=temp[1]*temp[0];
			conviance(1,1)+=temp[1]*temp[1];
			conviance(1,2)+=temp[1]*temp[2];

			conviance(2,0)+=temp[2]*temp[0];
			conviance(2,1)+=temp[2]*temp[1];
			conviance(2,2)+=temp[2]*temp[2];

		}
		conviance*=inv;
		gmm.det=conviance.det();
		gmm.inverseConviance=!conviance;

	}

}
float CTemporalRefiner::CalForPoss(int gmmindex,RGBQUAD rgb)
{

	int z[3];
	//RGBQUAD rgb=colimage.GetPixelColor(x,y,true);
	z[0]=rgb.rgbRed;
	z[1]=rgb.rgbGreen;
	z[2]=rgb.rgbBlue;

	double temp[3];

	double temp2[3];

	temp[0]=z[0]-fgmm[gmmindex].u(0,0);
	temp[1]=z[1]-fgmm[gmmindex].u(1,0);
	temp[2]=z[2]-fgmm[gmmindex].u(2,0);



	temp2[0]=fgmm[gmmindex].inverseConviance(0,0)*temp[0]+fgmm[gmmindex].inverseConviance(0,1)*temp[1]+fgmm[gmmindex].inverseConviance(0,2)*temp[2];
	temp2[1]=fgmm[gmmindex].inverseConviance(1,0)*temp[0]+fgmm[gmmindex].inverseConviance(1,1)*temp[1]+fgmm[gmmindex].inverseConviance(1,2)*temp[2];
	temp2[2]=fgmm[gmmindex].inverseConviance(2,0)*temp[0]+fgmm[gmmindex].inverseConviance(2,1)*temp[1]+fgmm[gmmindex].inverseConviance(2,2)*temp[2];


	//temp2[0]=fgmm[gmmindex].inverseConviance(0,0)*temp[0]+fgmm[gmmindex].inverseConviance(1,0)*temp[1]+fgmm[gmmindex].inverseConviance(2,0)*temp[2];
	//temp2[1]=fgmm[gmmindex].inverseConviance(0,1)*temp[0]+fgmm[gmmindex].inverseConviance(2,1)*temp[1]+fgmm[gmmindex].inverseConviance(3,1)*temp[2];
	//temp2[2]=fgmm[gmmindex].inverseConviance(0,2)*temp[0]+fgmm[gmmindex].inverseConviance(1,2)*temp[1]+fgmm[gmmindex].inverseConviance(2,2)*temp[2];


	if (fgmm[gmmindex].pi>0)
	{
		if (fgmm[gmmindex].det>0)
		{
			double mi=0.5*(temp[0]*temp2[0]+temp[1]*temp2[1]+temp[2]*temp2[2]);

			double leftr=sqrt(abs(fgmm[gmmindex].det))/fgmm[gmmindex].pi;
			return (1.0/(sqrt(abs(fgmm[gmmindex].det)))*Got_Exp(-0.5*mi));

			//return Got_Exp(-0.5*mi/(sqrt(abs(fgmm[gmmindex].det))));
		}
		return 0;

	}
	return 0;
}


float CTemporalRefiner::CalFMi(int gmmindex,RGBQUAD rgb)
{

	int z[3];
	//RGBQUAD rgb=colimage.GetPixelColor(x,y,true);
	z[0]=rgb.rgbRed;
	z[1]=rgb.rgbGreen;
	z[2]=rgb.rgbBlue;

	double temp[3];

	double temp2[3];

	temp[0]=z[0]-fgmm[gmmindex].u(0,0);
	temp[1]=z[1]-fgmm[gmmindex].u(1,0);
	temp[2]=z[2]-fgmm[gmmindex].u(2,0);



	temp2[0]=fgmm[gmmindex].inverseConviance(0,0)*temp[0]+fgmm[gmmindex].inverseConviance(0,1)*temp[1]+fgmm[gmmindex].inverseConviance(0,2)*temp[2];
	temp2[1]=fgmm[gmmindex].inverseConviance(1,0)*temp[0]+fgmm[gmmindex].inverseConviance(1,1)*temp[1]+fgmm[gmmindex].inverseConviance(1,2)*temp[2];
	temp2[2]=fgmm[gmmindex].inverseConviance(2,0)*temp[0]+fgmm[gmmindex].inverseConviance(2,1)*temp[1]+fgmm[gmmindex].inverseConviance(2,2)*temp[2];


	////temp2[0]=fgmm[gmmindex].inverseConviance(0,0)*temp[0]+fgmm[gmmindex].inverseConviance(1,0)*temp[1]+fgmm[gmmindex].inverseConviance(2,0)*temp[2];
	////temp2[1]=fgmm[gmmindex].inverseConviance(0,1)*temp[0]+fgmm[gmmindex].inverseConviance(2,1)*temp[1]+fgmm[gmmindex].inverseConviance(3,1)*temp[2];
	////temp2[2]=fgmm[gmmindex].inverseConviance(0,2)*temp[0]+fgmm[gmmindex].inverseConviance(1,2)*temp[1]+fgmm[gmmindex].inverseConviance(2,2)*temp[2];



	double mi=0.5*(temp[0]*temp2[0]+temp[1]*temp2[1]+temp[2]*temp2[2]);

	return mi;

}


float CTemporalRefiner::CalBackPoss(int gmmindex,RGBQUAD rgb)
{

	int z[3];
	//RGBQUAD rgb=colimage.GetPixelColor(x,y,true);
	z[0]=rgb.rgbRed;
	z[1]=rgb.rgbGreen;
	z[2]=rgb.rgbBlue;

	double temp[3];

	double temp2[3];

	temp[0]=z[0]-bgmm[gmmindex].u(0,0);
	temp[1]=z[1]-bgmm[gmmindex].u(1,0);
	temp[2]=z[2]-bgmm[gmmindex].u(2,0);



	temp2[0]=bgmm[gmmindex].inverseConviance(0,0)*temp[0]+bgmm[gmmindex].inverseConviance(0,1)*temp[1]+bgmm[gmmindex].inverseConviance(0,2)*temp[2];
	temp2[1]=bgmm[gmmindex].inverseConviance(1,0)*temp[0]+bgmm[gmmindex].inverseConviance(1,1)*temp[1]+bgmm[gmmindex].inverseConviance(1,2)*temp[2];
	temp2[2]=bgmm[gmmindex].inverseConviance(2,0)*temp[0]+bgmm[gmmindex].inverseConviance(2,1)*temp[1]+bgmm[gmmindex].inverseConviance(2,2)*temp[2];

	//temp2[0]=bgmm[gmmindex].inverseConviance(0,0)*temp[0]+bgmm[gmmindex].inverseConviance(1,0)*temp[1]+bgmm[gmmindex].inverseConviance(2,0)*temp[2];
	//temp2[1]=bgmm[gmmindex].inverseConviance(0,1)*temp[0]+bgmm[gmmindex].inverseConviance(1,1)*temp[1]+bgmm[gmmindex].inverseConviance(2,1)*temp[2];
	//temp2[2]=bgmm[gmmindex].inverseConviance(0,2)*temp[0]+bgmm[gmmindex].inverseConviance(1,2)*temp[1]+bgmm[gmmindex].inverseConviance(2,2)*temp[2];

	if (bgmm[gmmindex].pi>0)
	{
		if (bgmm[gmmindex].det>0)
		{
			double mi=0.5*(temp[0]*temp2[0]+temp[1]*temp2[1]+temp[2]*temp2[2]);

			double leftr=sqrt(abs(bgmm[gmmindex].det))/bgmm[gmmindex].pi;
			return (1.0/(sqrt(abs(bgmm[gmmindex].det)))*Got_Exp(-0.5*mi));
			//return Got_Exp(-0.5*mi/(sqrt(abs(bgmm[gmmindex].det))));
		}
		return 0;
	}
	return 0;
}

float CTemporalRefiner::CalBMi(int gmmindex,RGBQUAD rgb)
{

	int z[3];
	//RGBQUAD rgb=colimage.GetPixelColor(x,y,true);
	z[0]=rgb.rgbRed;
	z[1]=rgb.rgbGreen;
	z[2]=rgb.rgbBlue;

	double temp[3];
	double temp2[3];

	temp[0]=z[0]-bgmm[gmmindex].u(0,0);
	temp[1]=z[1]-bgmm[gmmindex].u(1,0);
	temp[2]=z[2]-bgmm[gmmindex].u(2,0);



	temp2[0]=bgmm[gmmindex].inverseConviance(0,0)*temp[0]+bgmm[gmmindex].inverseConviance(0,1)*temp[1]+bgmm[gmmindex].inverseConviance(0,2)*temp[2];
	temp2[1]=bgmm[gmmindex].inverseConviance(1,0)*temp[0]+bgmm[gmmindex].inverseConviance(1,1)*temp[1]+bgmm[gmmindex].inverseConviance(1,2)*temp[2];
	temp2[2]=bgmm[gmmindex].inverseConviance(2,0)*temp[0]+bgmm[gmmindex].inverseConviance(2,1)*temp[1]+bgmm[gmmindex].inverseConviance(2,2)*temp[2];

	//temp2[0]=bgmm[gmmindex].inverseConviance(0,0)*temp[0]+bgmm[gmmindex].inverseConviance(1,0)*temp[1]+bgmm[gmmindex].inverseConviance(2,0)*temp[2];
	//temp2[1]=bgmm[gmmindex].inverseConviance(0,1)*temp[0]+bgmm[gmmindex].inverseConviance(1,1)*temp[1]+bgmm[gmmindex].inverseConviance(2,1)*temp[2];
	//temp2[2]=bgmm[gmmindex].inverseConviance(0,2)*temp[0]+bgmm[gmmindex].inverseConviance(1,2)*temp[1]+bgmm[gmmindex].inverseConviance(2,2)*temp[2];


	double mi=0.5*(temp[0]*temp2[0]+temp[1]*temp2[1]+temp[2]*temp2[2]);

	return mi;




}

double CTemporalRefiner::GetBeta(CxImage* colimage)
{
	int width,height;
	width=colimage->GetWidth();
	height=colimage->GetHeight();

	double total=0.0;
	int num=0;
	for (int i=0;i<width;++i)
	{
		for (int j=0;j<height;++j)
		{
			if (i>=0&&j+1<height)
			{
				total+=coldistance(colimage->GetPixelColor(i,j,true),colimage->GetPixelColor(i,j+1));
				num++;
			}
			if (i>=0&&j-1>=0)
			{
				total+=coldistance(colimage->GetPixelColor(i,j,true),colimage->GetPixelColor(i,j-1));
				num++;
			}
			if (i-1>=0&&j<height)
			{
				total+=coldistance(colimage->GetPixelColor(i,j,true),colimage->GetPixelColor(i-1,j));
				num++;
			}
			if (i+1<width&&j<height)
			{
				total+=coldistance(colimage->GetPixelColor(i,j,true),colimage->GetPixelColor(i+1,j));
				num++;
			}
		}
	}
	return (1.0)/(2*(total/num));
}

double CTemporalRefiner::coldistance(RGBQUAD c1,RGBQUAD c2)
{
	return (c1.rgbRed-c2.rgbRed)*(c1.rgbRed-c2.rgbRed)+(c1.rgbGreen-c2.rgbGreen)*(c1.rgbGreen-c2.rgbGreen)+(c1.rgbBlue-c2.rgbBlue)*(c1.rgbBlue-c2.rgbBlue);
}


double CTemporalRefiner::calN(double distance, double coldis)
{
	return 50*Got_Exp(-beta*coldis)/distance;
}

void CTemporalRefiner::constructGraph(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange)
{

	CxImage label;
	//label.Copy(*(alphaBuffer->GetImage()));
	
	int pos=alphaBuffer->GetFramePos();

	cout<<fgmm[0].count<<endl;
	cout<<bgmm[0].count<<endl;

	if (index==NULL)
	{
		index=new int *[refineRange];
		for (int i=0;i<refineRange;++i)
		{
			index[i]=new int [w*h];
			for (int j=0;j<w*h;++j)
				index[i][j]=-1;			
		}
	}


	RGBQUAD rgb1;
	RGBQUAD rgb2;
	RGBQUAD colRgb;
	RGBQUAD lrgb;
	int count=0;

	float *u;
	float *v;
	v=(float*)malloc(sizeof(float)*w*h);
	u=(float*)malloc(sizeof(float)*w*h);

	float* u2;
	float* v2;
	v2=(float*)malloc(sizeof(float)*w*h);
	u2=(float*)malloc(sizeof(float)*w*h);

	for (int i=0;i<refineRange;++i)
	{
		
		/*trimapBuffer->Back();*/
		CxImage frame1;
		frame1.Copy(*(imageBuffer->GetImage()));
		imageBuffer->Forward();

		CxImage frame2;
		frame2.Copy(*(imageBuffer->GetImage()));

		CxImage trimap1;
		trimap1.Copy(*(trimapBuffer->GetImage()));
		trimapBuffer->Forward();
		CxImage trimap2;
		trimap2.Copy(*(trimapBuffer->GetImage()));

		CxImage label1;
		label1.Copy(*(alphaBuffer->GetImage()));
		bool b_nend=alphaBuffer->Forward();
		CxImage label2;
		label2.Copy(*(alphaBuffer->GetImage()));

		//cal optical flow

		if (i!=refineRange-1&&b_nend)
		{
			memset(u,0,w*h*sizeof(float));
			memset(v,0,w*h*sizeof(float));

			memset(u2,0,w*h*sizeof(float));
			memset(v2,0,w*h*sizeof(float));

			this->CalOpticalFlow(frame1,frame2,u,v);

			this->CalOpticalFlow(frame2,frame1,u2,v2);

			//CxImage ttt;
			//ttt.Copy(frame2);
			//ttt.Clear();
			//for (int it=0;it<w;++it)
			//{
			//	for (int jt=0;jt<h;++jt)
			//	{
			//		RGBQUAD rr=label1.GetPixelColor(it,jt);

			//		int itt,jtt;
			//		itt=it+u[jt*w+it];
			//		jtt=jt+v[jt*w+it];

			//		if (rr.rgbBlue==255)
			//		{
			//			RGBQUAD rrr=frame2.GetPixelColor(itt,jtt);
			//			ttt.SetPixelColor(itt,jtt,rrr);
			//		}
			//	}
			//}

			//CString cs="D:\\";
			//cs.AppendFormat("%d",i+1);
			//cs+=".jpg";
			//ttt.Save(cs,CXIMAGE_FORMAT_JPG);

		}


		//end of cal optical flow
		this->beta=GetBeta(&frame1);
		for (int k=0;k<h;++k)
		{
			for (int p=0;p<w;++p)
			{
				 rgb1=trimap1.GetPixelColor(p,k);
				 lrgb=label1.GetPixelColor(p,k);
				 //if (rgb1.rgbBlue==128)//unknown area
				 if(/*lrgb.rgbBlue==255||*/rgb1.rgbBlue==128)
				 {
					 if (index[i][k*w+p]==-1)
					 {

#ifdef  NDATA_COST

						 float fposs=0;
						 float bposs=0;
				
						 if (lrgb.rgbBlue==255)
						 {
							 fposs=0.7;
							 bposs=0.3;
						 }
						 else
						 {
							 fposs=0.3;
							 bposs=0.7;
						 }
					
#else
						 colRgb=frame1.GetPixelColor(p,k);
						 float fposs=CalForTWeight(colRgb);
						 float bposs=CalBackTWeight(colRgb);
#endif
						 graph->set_tweights(gnode[count],fposs,bposs);
						 index[i][k*w+p]=count;
						 ++count;
					 }

					 //neighbourhood
					 if (p-1>=0)
					 {
						 rgb1=trimap1.GetPixelColor(p-1,k);
						 lrgb=label1.GetPixelColor(p-1,k);
						 if (rgb1.rgbBlue==128)//||lrgb.rgbBlue==255)
						 {
							 RGBQUAD colRgb2=frame1.GetPixelColor(p-1,k);
							 if (index[i][k*w+p-1]==-1)
							 {
#ifdef NDATA_COST

								 float fposs=0;
								 float bposs=0;

								 if (lrgb.rgbBlue==255)
								 {
									 fposs=0.7;
									 bposs=0.3;
								 }
								 else
								 {
									 fposs=0.3;
									 bposs=0.7;
								 }
#else
								 float fposs=CalForTWeight(colRgb2);
								 float bposs=CalBackTWeight(colRgb2);	
#endif
								 graph->set_tweights(gnode[count],fposs,bposs);
								 index[i][k*w+p-1]=count;
								 ++count;
							 }
							 
							 float nweight=CalNWeight(colRgb,colRgb2);
							 graph->add_edge(gnode[index[i][k*w+p]],gnode[index[i][k*w+p-1]],nweight,nweight);
						 }
					 }//end if (p-1>=0)

					 if (p+1<w)
					 {
						 rgb1=trimap1.GetPixelColor(p+1,k);
						 lrgb=label1.GetPixelColor(p+1,k);
						 if (rgb1.rgbBlue==128)//||lrgb.rgbBlue==255)
						 {
							 RGBQUAD colRgb2=frame1.GetPixelColor(p+1,k);

							 if (index[i][k*w+p+1]==-1)
							 {
#ifdef NDATA_COST
								 float fposs=0;
								 float bposs=0;
						 		 if (lrgb.rgbBlue==255)
								 {
									 fposs=0.7;
									 bposs=0.3;
								 }
								 else
								 {
									 fposs=0.3;
									 bposs=0.7;
								 }


#else
								 float fposs=CalForTWeight(colRgb2);
								 float bposs=CalBackTWeight(colRgb2);
#endif

								 graph->set_tweights(gnode[count],fposs,bposs);
								 index[i][k*w+p+1]=count;
								 ++count;
							 }

							 float nweight=CalNWeight(colRgb,colRgb2);
							 graph->add_edge(gnode[index[i][k*w+p]],gnode[index[i][k*w+p+1]],nweight,nweight);
						 }

					 }//end if (p+1<w)

					 if (k-1>=0)
					 {
						 rgb1=trimap1.GetPixelColor(p,k-1);
						 lrgb=label1.GetPixelColor(p,k-1);
						 if (rgb1.rgbBlue==128)//||lrgb.rgbBlue==255)
						 {
							 RGBQUAD colRgb2=frame1.GetPixelColor(p,k-1);

							 if (index[i][(k-1)*w+p]==-1)
							 {
#ifdef NDATA_COST
					
								 float fposs=0;
								 float bposs=0;
								 if (lrgb.rgbBlue==255)
								 {
									 fposs=0.7;
									 bposs=0.3;
								 }
								 else
								 {
									 fposs=0.3;
									 bposs=0.7;
								 }
#else
								 float fposs=CalForTWeight(colRgb2);
								 float bposs=CalBackTWeight(colRgb2);
#endif			
								 graph->set_tweights(gnode[count],fposs,bposs);
								 index[i][(k-1)*w+p]=count;
								 ++count;
							 }

							 float nweight=CalNWeight(colRgb,colRgb2);
							 graph->add_edge(gnode[index[i][k*w+p]],gnode[index[i][(k-1)*w+p]],nweight,nweight);
						 }

					 }//end  if (k-1>=0)

					 if (k+1<h)
					 {
						 rgb1=trimap1.GetPixelColor(p,k+1);
						 lrgb=label1.GetPixelColor(p,k+1);
						 if (rgb1.rgbBlue==128)//||lrgb.rgbBlue==255)
						 {
							 RGBQUAD colRgb2=frame1.GetPixelColor(p,k+1);
							 if (index[i][(k+1)*w+p]==-1)
							 {
#ifdef NDATA_COST

								 float fposs=0;
								 float bposs=0;
								 if (lrgb.rgbBlue==255)
								 {
									 fposs=0.7;
									 bposs=0.3;
								 }
								 else
								 {
									 fposs=0.3;
									 bposs=0.7;
								 }
#else
								 float fposs=CalForTWeight(colRgb2);
								 float bposs=CalBackTWeight(colRgb2);
#endif
								 graph->set_tweights(gnode[count],fposs,bposs);
								 index[i][(k+1)*w+p]=count;
								 ++count;
							 }

							 float nweight=CalNWeight(colRgb,colRgb2);
							 graph->add_edge(gnode[index[i][k*w+p]],gnode[index[i][(k+1)*w+p]],nweight,nweight);

						 }


					 }//end  if (k+1<h)
				
					 //temporal constraint
				if (i!=refineRange-1)
					 {
						 int np,nk;
						 np=p+u[k*w+p];
						 nk=k+v[k*w+p];

						 if (np>=w||np<0||nk>=h&&nk<0)
							 continue;


						 RGBQUAD trgb=trimap2.GetPixelColor(np,nk);
						 lrgb=label2.GetPixelColor(np,nk);
						 if (trgb.rgbBlue==128)//||lrgb.rgbBlue==255)
						 {
							 RGBQUAD  tempcolRgb=frame2.GetPixelColor(np,nk);

#ifdef NDATA_COST

							 float fposs=0;
							 float bposs=0;

							 if (lrgb.rgbBlue==255)
							 {
								 fposs=0.7;
								 bposs=0.3;
							 }
							 else
							 {
								 fposs=0.3;
								 bposs=0.7;
							 }
#else
							 float fposs=CalForTWeight(tempcolRgb);
							 float bposs=CalBackTWeight(tempcolRgb);
#endif
							 graph->set_tweights(gnode[count],fposs,bposs);

							 index[i+1][nk*w+np]=count;
							 ++count;
							 RGBQUAD tlrgb1,tlrgb2;
							 tlrgb1=label1.GetPixelColor(p,k);
							 tlrgb2=label2.GetPixelColor(np,nk);
							 if (tlrgb1.rgbBlue==tlrgb2.rgbBlue)
							 {
								  graph->add_edge(gnode[index[i][k*w+p]],gnode[index[i+1][nk*w+np]],0,0);
							 }
							 else
							 {
								 int r1=0,g1=0,b1=0;
								 int r2=0,g2=0,b2=0;
								 int pnum=0;
								 for (int m=p-2;m<=p+2;++m)
								 {
									 for (int n=k-2;n<=k+2;++n)
									 {
										 if (m>=0&&m<w&&n>=0&&n<h)
										 {
											 RGBQUAD rq1;
											 rq1=frame1.GetPixelColor(m,n);
											 r1+=rq1.rgbRed;
											 g1+=rq1.rgbGreen;
											 b1+=rq1.rgbBlue;
											 pnum++;
										 }
									 }
								 }
								 int pnum2=0;

								 for (int m=np-2;m<=np+2;++m)
								 {
									 for (int n=nk-2;n<=nk+2;++n)
									 {
										 RGBQUAD rq2;
										 rq2=frame2.GetPixelColor(m,n);
										 r2+=rq2.rgbRed;
										 g2+=rq2.rgbGreen;
										 b2+=rq2.rgbBlue;
										 pnum2++;
									 }
								 }
								 if (pnum)
								 {
									 r1/=pnum;g1/=pnum;b1/=pnum;
									 r2/=pnum2;g2/=pnum2;b2/=pnum2;

									 float temporalweight=CalTemporalWeight(r1,g1,b1,r2,g2,b2);
									 graph->add_edge(gnode[index[i][k*w+p]],gnode[index[i+1][nk*w+np]],100*temporalweight,100*temporalweight);
								 }
							 }


							  //backword							
							/* int np2,nk2;
							 np2=np+u2[nk*w+np];
							 nk2=nk+v2[nk*w+np];

							 if (np2<0||np2>=w||nk2<0||nk2>=h)
								 continue;

							 trgb=trimap1.GetPixelColor(np2,nk2);
							 lrgb=label1.GetPixelColor(np2,nk2);
							 if (trgb.rgbBlue==128)
							 {
								 if (index[i][nk2*w+np2]==-1)
								 {

									 float fposs,bposs;
									 if (lrgb.rgbBlue==255)
									 {
										 fposs=0.7;
										 bposs=0.3;
									 }
									 else
									 {
										 fposs=0.3;
										 bposs=0.7;
									 }
									 graph->set_tweights(gnode[count],fposs,bposs);
									 index[i][nk2*w+np2]=count;
									 count++;
								 }
								 RGBQUAD tlrgb1=label1.GetPixelColor(np2,nk2);

								 RGBQUAD tlrgb2=label2.GetPixelColor(np,nk);

							
								 if (tlrgb2.rgbBlue==tlrgb1.rgbBlue)
								 {
									
									 graph->add_edge(gnode[index[i+1][nk*w+np]],gnode[index[i][nk2*w+np2]],0,0);
								
								 }


								 else{
									
									 int r1=0,g1=0,b1=0;
									 int r2=0,g2=0,b2=0;
									 int pnum=0;
									 for (int m=np-2;m<=np+2;++m)
									 {
										 for (int n=nk-2;n<=nk+2;++n)
										 {
											 if (m>=0&&m<w&&n>=0&&n<h)
											 {
												 RGBQUAD rq1;
												 rq1=frame2.GetPixelColor(m,n);
												 r1+=rq1.rgbRed;
												 g1+=rq1.rgbGreen;
												 b1+=rq1.rgbBlue;
												 pnum++;
											 }
										 }
									 }
									 int pnum2=0;

									 for (int m=np2-2;m<=np2+2;++m)
									 {
										 for (int n=nk2-2;n<=nk2+2;++n)
										 {
											 if (m>=0&&m<w&&n>=0&&n<h)
											 {
												 RGBQUAD rq2;
												 rq2=frame1.GetPixelColor(m,n);
												 r2+=rq2.rgbRed;
												 g2+=rq2.rgbGreen;
												 b2+=rq2.rgbBlue;
												 pnum2++;
											 }

										 }
									 }
									 if (pnum)
									 {
										 r1/=pnum;g1/=pnum;b1/=pnum;
										 r2/=pnum2;g2/=pnum2;b2/=pnum2;

										 float temporalweight=CalTemporalWeight(r1,g1,b1,r2,g2,b2);

										 graph->add_edge(gnode[index[i+1][nk*w+np]],gnode[index[i][nk2*w+np2]],100*temporalweight,100*temporalweight);
									 }
								 }
							 }*/

						 }	
					 }//temporal

				 }//end  if (rgb1.rgbBlue==128)//unknown area
			}
		}
		cout<<"after for"<<endl;
	}
	alphaBuffer->Goto(pos);
	imageBuffer->Goto(pos);
	trimapBuffer->Goto(pos);
}

void CTemporalRefiner::updateGraph(ImageBuffer * alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange)
{
	
	CxImage label;
	//label=alphaBuffer->GetImage();

	int pos=alphaBuffer->GetFramePos();

	RGBQUAD rgb1;
	RGBQUAD rgb2;
	RGBQUAD colRgb;
	int count=0;
	for (int i=0;i<refineRange;++i)
	{
		//trimapBuffer->Back();
		CxImage frame1;
		frame1.Copy(*(imageBuffer->GetImage()));
		imageBuffer->Forward();
		//CxImage* frame2=imageBuffer->GetImage();
	
		CxImage trimap1;
		trimap1.Copy(*(trimapBuffer->GetImage()));
		trimapBuffer->Forward();

		label.Copy(*(alphaBuffer->GetImage()));
		alphaBuffer->Forward();
		//CxImage* trimap2=trimapBuffer->GetImage();
	
		for (int k=0;k<h;++k)
		{
			for (int p=0;p<w;++p)
			{
				rgb1=trimap1.GetPixelColor(p,k);
				if (rgb1.rgbBlue==128)//unknown area
				{
					//calculate T weight
	
					colRgb=frame1.GetPixelColor(p,k);

					float fposs=CalForTWeight(colRgb);
					float bposs=CalBackTWeight(colRgb);
					graph->set_tweights(gnode[index[i][k*w+p]],fposs,bposs);
				}//end  if (rgb1.rgbBlue==128)//unknown area
			}
		}

	}

	alphaBuffer->Goto(pos);
	imageBuffer->Goto(pos);
	trimapBuffer->Goto(pos);

}

float CTemporalRefiner::CalForTWeight(RGBQUAD rgb)
{
	float  forposs=0.0f;
	for (int jtemp=0;jtemp<COMNUM;jtemp++)
	{
		if (fgmm[jtemp].count)
		{
			forposs+=fgmm[jtemp].pi*CalForPoss(jtemp,rgb);			
		}
		else
			continue;	
	}


	return log(forposs);

	//float forposs=100.0f;
	//for (int jtemp=0;jtemp<COMNUM;++jtemp)
	//{
	//	float temp=0.0;
	//	if (fgmm[jtemp].count)
	//	{
	//		temp=-log(fgmm[jtemp].pi)+0.5*log(fgmm[jtemp].det)+CalFMi(jtemp,rgb);
	//		if (forposs<temp)
	//		{
	//			forposs=temp;
	//		}
	//	}
	//}

	//return forposs;
}

float CTemporalRefiner::CalBackTWeight(RGBQUAD rgb)
{
	float backposs=0.0f;

	for(int jtemp=0;jtemp<COMNUM;++jtemp)
	{
		if (bgmm[jtemp].count)
		{
			backposs+=bgmm[jtemp].pi*CalBackPoss(jtemp,rgb);
		}
		else
			continue;
	}

	return log(backposs);


	//float backposs=100.0f;
	//for (int jtemp=0;jtemp<COMNUM;++jtemp)
	//{
	//	float temp=0.0f;
	//	if (bgmm[jtemp].count)
	//	{
	//		temp=-log(bgmm[jtemp].pi)+0.5*log(bgmm[jtemp].det)+CalBMi(jtemp,rgb);
	//		if (backposs<temp)
	//		{
	//			backposs=temp;
	//		}
	//	}
	//}

	//return backposs;
}

float CTemporalRefiner::CalNWeight(RGBQUAD rgb1,RGBQUAD rgb2)
{
	double distanceE=1;
	double discol=coldistance(rgb1,rgb2);

	return calN(distanceE,discol)*0.2;
}

double CTemporalRefiner::Distance(int x1,int y1,int x2,int y2)
{
		double dis=(x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);
		return sqrt(dis);
}

double CTemporalRefiner::CalTemporalWeight(int r1,int g1,int b1,int r2,int g2,int b2)
{
	double i=(r1-r2)*(r1-r2)+(g1-g2)*(g1-g2)+(b1-b2)*(b1-b2);
	return Got_Exp(-i/(2*9));
}

void CTemporalRefiner::Refine(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange,int trimapWidth)
{

	int pos=alphaBuffer->GetFramePos();
	this->SampleColor(alphaBuffer,imageBuffer,trimapBuffer,refineRange,trimapWidth);
	cout<<"after sampleColor"<<endl;

	this->constructGraph(alphaBuffer,imageBuffer,trimapBuffer,refineRange);
	cout<<"after construct graph"<<endl;

	int changed=21;
	int cc=5;

		changed=this->DoSeg(alphaBuffer,imageBuffer,trimapBuffer,refineRange);
		/*this->UpdateGmmParam(alphaBuffer,imageBuffer,trimapBuffer,refineRange);
		this->updateGraph(alphaBuffer,imageBuffer,trimapBuffer,refineRange);*/
		cout<<"changed: "<<changed<<endl;

	
	alphaBuffer->Goto(pos);
	imageBuffer->Goto(pos);
	trimapBuffer->Goto(pos);

}

int CTemporalRefiner::DoSeg(ImageBuffer* alphaBuffer,ImageBuffer* imageBuffer,ImageBuffer* trimapBuffer,int refineRange)
{
	graph->maxflow();
	int pos=alphaBuffer->GetFramePos();
	CxImage* label;
	CxImage* trimap;

	int count=0;
	RGBQUAD fg,bg;
	fg.rgbBlue=fg.rgbGreen=fg.rgbRed=255;
	bg.rgbBlue=bg.rgbGreen=bg.rgbRed=0;
	//alphaBuffer->Forward();
	RGBQUAD trgb;
	int changed=0;
	do 
	{
		label=alphaBuffer->GetImage();
		trimap=trimapBuffer->GetImage();
		for (int j=0;j<h;++j)
		{
			for (int i=0;i<w;++i)
			{
				trgb=trimap->GetPixelColor(i,j);
				if (/*trgb.rgbBlue==128&&*/index[count][j*w+i]!=-1)
				{
					if (graph->what_segment(gnode[index[count][j*w+i]])==Graph::SOURCE)
					{
						if (label->GetPixelColor(i,j).rgbBlue==0)
						{
							changed++;
						}
						label->SetPixelColor(i,j,fg);
					}

					else
					{
						if (label->GetPixelColor(i,j).rgbBlue==255)
						{
							changed++;
						}
						label->SetPixelColor(i,j,bg);
					}
				}
			}
		}
		label->Save(alphaBuffer->GetFrameName(),CXIMAGE_FORMAT_BMP);
	++count;
	} while (count<refineRange&&alphaBuffer->Forward()&&trimapBuffer->Forward());
	alphaBuffer->Goto(pos);
	imageBuffer->Goto(pos);
	trimapBuffer->Goto(pos);
	return changed;
}

void CTemporalRefiner::CalOpticalFlow(const CxImage &curr,const CxImage &next,float* u,float* v)
{
	CxImage image1,image2;
	image1.Copy(curr);
	image2.Copy(next);

	float* bf1;
	float* bf2;
	int w,h;
	w=image1.GetWidth();
	h=image1.GetHeight();

	bf1=(float*)malloc(sizeof(float)*w*h);
	bf2=(float*)malloc(sizeof(float)*w*h);

	int count=0;
	for (int j=0;j<h;++j)
	{
		for (int i=0;i<w;++i)
		{
			BYTE temp1=image1.GetPixelGray(i,j);
			BYTE temp2=image2.GetPixelGray(i,j);

			bf1[count]=temp1;
			bf2[count]=temp2;
			count++;

		}
	}
	calFlow(bf1,bf2,u,v,0,1,4,1,w,h);

	free(bf1);
	free(bf2);

}