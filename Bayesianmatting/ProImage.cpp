#include "../VideoEditing/stdafx.h"
#include "ProImage.h"
#include <limits>
#include <cmath>
#include <exception>
#include <fstream>
#include <WinDef.h>
ProImage::ProImage(void)
{
	alpha=NULL;
	tag=NULL;
}
ProImage::~ProImage(void)
{
	if (alpha)
	{
		for (int i=0;i<simage.GetWidth();++i)
		{
			delete[] alpha[i];
			delete[] tag[i];
			alpha[i]=NULL;
			tag[i]=NULL;
		}

		delete[] alpha;
		delete[] tag;
		alpha=NULL;
		tag=NULL;
	}
}
ProImage::ProImage(string sname, string triname, string  desname)
{
	this->SImageName=sname;
	this->TriImageName=triname;
	this->DesImageName=desname;

}
void ProImage::init(CxImage si,CxImage  ti)
{
	simage.Copy(si,true,true,true);
	timage.Copy(ti,true,true,true);

	int w,h;
	w=si.GetWidth();
	h=si.GetHeight();

	if (alpha==NULL)
	{
		alpha=new double *[w];
		tag=new int *[w];
		for (int i=0;i<w;++i)
		{
			alpha[i]=new double [h];
			tag[i]=new int [h];
		}
	}

}


void ProImage::getImageName()
{
	//std::cout<<"Enter Image names ,one per line:"<<endl;
	//std::cout<<"Source Image name:"<<" ";
	getline(cin,this->SImageName);
	//std::cout<<"Trimap name: ";
	getline(cin,this->TriImageName);
	//std::cout<<"New background name:";
	getline(cin,this->DesImageName);
}
bool ProImage::isValidName()
{
	//load image......
	//simage.Load(SImageName.c_str(),CXIMAGE_FORMAT_JPG);
	//timage.Load(TriImageName.c_str(),CXIMAGE_FORMAT_BMP);
	//dimage.Load(DesImageName.c_str(),CXIMAGE_FORMAT_BMP);
	//dimage.Clear(0);
	backImage.Copy(simage,true,true,true);
	forImage.Copy(simage,true,true,true);
	alphaImage.Copy(simage,true,true,true);
	//if images exist
	if(simage.IsValid()&&timage.IsValid())//&&dimage.IsValid())//存在
	{
		try
		{
			//std::cout<<"All images exists\n";
			this->width=simage.GetWidth();
			this->height=simage.GetHeight();
		}
		catch (exception &e)
		{
			cout<<"In isvalidName()"<<endl;
		}
		return true;
	}
	else// not exist
	{
		//std::cout<<"Image not exists\n";
		return false;
	}
}
//do matting
void ProImage::mattingImage()
{
	long i;
	long j;
	try
	{
		//initialize alpha and tag
		for( i=0;i<width;i++)
		{
			for( j=0;j<height;j++)
			{
				
				unsigned char  current=timage.GetPixelGray(i,j);
				if ((int)current==255)//known foreground area
				{
					//dimage.SetPixelColor(i,j,simage.GetPixelColor(i,j,true));
					alpha[i][j]=1.0;
					tag[i][j]=1;
				}
				else if ((int)current==0)//know background area
				{
					alpha[i][j]=0.0;
					tag[i][j]=1;
				}
				else
					tag[i][j]=0.0;

			}
		}
		for (i=0;i<width;i++)
		{
			for (j=0;j<height;j++)
			{
				if(!tag[i][j])//deal with unkown area
				{
					process(i,j);
				}
			}
		}

	}
	catch (exception & e)
	{
		cout<<"In mattingImage()"<<endl;
	}
}
//add neighbours as GMM sample
void ProImage::addsample(long i,long j,vector<colorNode> &f,vector<colorNode>& b)
{
	//distance(from near to far)
	int dist=1;
	while (f.size()<MINSAMPLE)
	{
		if (i-dist>0)
		{
			for (int z=maxab(0,j-dist);z<minab(j+dist,height-1);z++)
			{

				unsigned char tri=timage.GetPixelGray(i-dist,z);
				if ((int)tri==255||(tag[i-dist][z]&&alpha[i-dist][z]==1))//known forground or estimated pixel
				{
					RGBQUAD sim=simage.GetPixelColor(i-dist,z,true);
					colorNode temp;
					temp.indexX=i-dist;
					temp.indexY=z;
					temp.colorinfo.resize(3,1);
					temp.colorinfo(0,0)=(double)sim.rgbRed;
					temp.colorinfo(1,0)=(double)sim.rgbGreen;
					temp.colorinfo(2,0)=(double)sim.rgbBlue;
					f.push_back(temp);
					if (f.size()==MINSAMPLE)
					{
						goto BG;
					}

				}

			}
		}
		if (i+dist<width)
		{
			for (int z=maxab(0,j-dist);z<minab(j+dist,height);z++)
			{
				unsigned char tri=timage.GetPixelGray(i+dist,z);
				if ((int)tri==255	||(tag[i+dist][z]&&alpha[i+dist][z]==1))
				{
					RGBQUAD sim=simage.GetPixelColor(i+dist,z,true);
					colorNode temp;
					temp.indexX=i+dist;
					temp.indexY=z;
					temp.colorinfo.resize(3,1);
					temp.colorinfo(0,0)=(double)sim.rgbRed;
					temp.colorinfo(1,0)=(double)sim.rgbGreen;
					temp.colorinfo(2,0)=(double)sim.rgbBlue;
					f.push_back(temp);
					if (f.size()==MINSAMPLE)
					{
						goto BG;
					}
				}

			}
		}
		if (j-dist>0)
		{
			for (int z=maxab(0,i-dist);z<minab(i+dist,width);z++)
			{
				unsigned char tri=timage.GetPixelGray(z,j-dist);
				if ((int)tri==255||(tag[z][j-dist]&&alpha[z][j-dist]==1))
				{
					RGBQUAD sim=simage.GetPixelColor(z,j-dist,true);
					colorNode temp;
					temp.colorinfo.resize(3,1);
					temp.indexY=j-dist;
					temp.indexX=z;
					temp.colorinfo(0,0)=(double)sim.rgbRed;
					temp.colorinfo(1,0)=(double)sim.rgbGreen;
					temp.colorinfo(2,0)=(double)sim.rgbBlue;
					f.push_back(temp);
					if (f.size()==MINSAMPLE)
					{
						goto BG;
					}
				}

			}
		}
		if (j+dist<height)
		{
			for (int z=maxab(0,i-dist);z<minab(i+dist,width);z++)
			{
				unsigned char tri=timage.GetPixelGray(z,j+dist);
				if((int)tri==255||(tag[z][j+dist]&&alpha[z][j+dist]==1))
				{
					RGBQUAD fim=simage.GetPixelColor(z,j+dist,true);
					colorNode temp;
					temp.colorinfo.resize(3,1);
					temp.indexX=z;
					temp.indexY=j+dist;
					temp.colorinfo(0,0)=(double)fim.rgbRed;
					temp.colorinfo(1,0)=(double)fim.rgbGreen;
					temp.colorinfo(2,0)=(double)fim.rgbBlue;
					f.push_back(temp);
					if (f.size()==MINSAMPLE)
					{
						goto BG;
					}
				}

			}
		}
		dist++;
	}
BG:
	while (b.size()<MINSAMPLE)
	{
		if (i-dist>0)
		{
			for (int z=maxab(0,j-dist);z<minab(j+dist,height);z++)
			{
				unsigned char tri=timage.GetPixelGray(i-dist,z);
				if ((int)tri==0||(tag[i-dist][z]&&alpha[i-dist][z]==0.0))//known background or estimated pixel
				{
					RGBQUAD sim=simage.GetPixelColor(i-dist,z,true);
					colorNode temp;
					temp.colorinfo.resize(3,1);
					temp.indexX=i-dist;
					temp.indexY=z;
					temp.colorinfo(0,0)=(double)sim.rgbRed;
					temp.colorinfo(1,0)=(double)sim.rgbGreen;
					temp.colorinfo(2,0)=(double)sim.rgbBlue;
					b.push_back(temp);
					if (b.size()==MINSAMPLE)
					{
						goto DONE;
					}
				}

			}
		}
		if (i+dist<width)
		{
			for (int z=maxab(0,j-dist);z<minab(j+dist,height);z++)
			{
				unsigned char tri=timage.GetPixelGray(i+dist,z);
				if ((int)tri==0||(tag[i+dist][z]&&alpha[i+dist][z]==0.0))
				{
					RGBQUAD sim=simage.GetPixelColor(i+dist,z,true);
					colorNode temp;
					temp.colorinfo.resize(3,1);
					temp.indexX=i+dist;
					temp.indexY=z;
					temp.colorinfo(0,0)=(double)sim.rgbRed;
					temp.colorinfo(1,0)=(double)sim.rgbGreen;
					temp.colorinfo(2,0)=(double)sim.rgbBlue;
					b.push_back(temp);
					if (b.size()==MINSAMPLE)
					{
						goto DONE;
					}
				}

			}
		}
		if (j-dist>0)
		{
			for (int z=maxab(0,i-dist);z<minab(width,i+dist);z++)
			{
				unsigned char tri=timage.GetPixelGray(z,j-dist);
				if ((int)tri==0||(tag[z][j-dist]&&alpha[z][j-dist]==0.0))
				{
					RGBQUAD sim=simage.GetPixelColor(z,j-dist);
					colorNode temp;
					temp.colorinfo.resize(3,1);
					temp.indexX=z;
					temp.indexY=j-dist;
					temp.colorinfo(0,0)=(double)sim.rgbRed;
					temp.colorinfo(1,0)=(double)sim.rgbGreen;
					temp.colorinfo(2,0)=(double)sim.rgbBlue;
					b.push_back(temp);
					if (b.size()==MINSAMPLE)
					{
						goto DONE;
					}
				}

			}
		}
		if (j+dist<height)
		{
			for (int z=maxab(0,i-dist);z<minab(width,i+dist);z++)
			{
				unsigned char tri=timage.GetPixelGray(z,j+dist);
				if ((int)tri==0||(tag[z][j+dist]&&alpha[z][j+dist]==0.0))
				{
					RGBQUAD sim=simage.GetPixelColor(z,j+dist,true);
					colorNode temp;
					temp.colorinfo.resize(3,1);
					temp.indexX=z;
					temp.indexY=j+dist;
					temp.colorinfo(0,0)=(double)sim.rgbRed;
					temp.colorinfo(1,0)=(double)sim.rgbGreen;
					temp.colorinfo(2,0)=(double)sim.rgbBlue;
					b.push_back(temp);
					if (b.size()==MINSAMPLE)
					{
						goto DONE;
					}
				}

			}
		}
		dist++;
	}
DONE:
	;
}
//actually process unknown pixel
void ProImage::process(long i,long j)
{
	//cout<<"Process: "<<i<<j<<endl;
	vector < vector <colorNode> > forvec;//store forground neighbor
	vector < vector <colorNode> > backvec;//store background neighbor
	vector<colorNode> fortempvec;
	vector<colorNode> backtempvec;
	RGBQUAD center=simage.GetPixelColor(i,j,true);//current pixel
	dMatrix centercolor(3,1);
	centercolor(0,0)=(double)center.rgbRed;
	centercolor(1,0)=(double)center.rgbGreen;
	centercolor(2,0)=(double)center.rgbBlue;
	addsample(i,j,fortempvec,backtempvec);
	forvec.push_back(fortempvec);
	backvec.push_back(backtempvec);
	double avgAlpha=getAvgAlpha(i,j);
	cluster(forvec);//forvec--all forground neighbours after cluster
	cluster(backvec);//backvec--all background neighbours after cluster 
	vector<FNode> fmvec;//mean and conviance responding to forvec
	vector<BNode> bmvec;//mean and conviance responding to backvec
	forMatrix(forvec,fmvec,i,j);
	backMatrix(backvec,bmvec,i,j);
	double finalAlpha=0;//final alpha value to save
	dMatrix finalfor(3,1,0.0);//final forground color value to save
	dMatrix finalback(3,1,0.0);//final background color value to save
	double possibility=-666660.0f;//max possibility of clusters
	dMatrix leftmat;//A
	dMatrix rightmat;//b
	double centeralpha=0.0;
	dMatrix f(3,1,0.0);
	dMatrix b(3,1,0.0);
	dMatrix result(6,1,0.0);//equation result
	dMatrix temp(3,1,0.0);
	double mul=0.0;
	double lc=0.0;
	dMatrix temp1;
	double lf;
	dMatrix temp2;
	double lb;
	double poss=0.0;
	int X,Y;
	for (int m=0;m<forvec.size();m++)
	{
		for (int n=0;n<backvec.size();n++)
		{
			double avgalpha2=avgAlpha;
			for (int time=0;time<5;time++)
			{
				getlrMat(leftmat,rightmat,fmvec[m],bmvec[n],avgalpha2,centercolor);
				bool re=leftmat.solve(rightmat,result);
				f=result[mslice(0,0,3,1)];
				b=result[mslice(3,0,3,1)];
				centeralpha=getalpha(f,b,centercolor);
				avgalpha2=centeralpha;
			}
			//calculate the possibility 
			temp=centercolor-centeralpha*f-(1-centeralpha)*b;
			mul=temp(0,0)*temp(0,0)+temp(1,0)*temp(1,0)+temp(2,0)*temp(2,0);
			lc=-mul/(SD*SD);
			temp1=(~(f-fmvec[m].avgf))*(!fmvec[m].cmatrixf)*(f-fmvec[m].avgf)*0.5;
			lf=-temp1(0,0);
			temp2=(~(b-bmvec[n].avgb))*(!(bmvec[n].cmatrixb))*(b-bmvec[n].avgb)*0.5;
			lb=-temp2(0,0);
			poss=lc+lf+lb;
			if (possibility<poss)
			{
				finalfor=f;
				finalback=b;
				finalAlpha=centeralpha;
				possibility=poss;
				X=m;
				Y=n;
			}

		}
	}
	try
	{
		for (int time=0;time<5;time++)
		{
			getlrMat(leftmat,rightmat,fmvec[X],bmvec[Y],finalAlpha,centercolor);
			bool re=leftmat.solve(rightmat,result);//得到的F B存放于result中
			if (!re)
			{
				break;
			}
			finalfor=result[mslice(0,0,3,1)];
			finalback=result[mslice(3,0,3,1)];
			finalAlpha=getalpha(finalfor,finalback,centercolor);
		}
	}
	catch (exception &e)
	{
		cout<<e.what()<<endl;
		int i;
		cin>>i;
	}
	if(finalAlpha>0.999)
		finalAlpha=1.0;
	if(finalAlpha<0.0)
		finalAlpha=0.0;
	alpha[i][j]=finalAlpha;
	tag[i][j]=1;
	RGBQUAD fr;
	fr.rgbRed=(BYTE)finalfor(0,0);
	fr.rgbGreen=(BYTE)finalfor(1,0);
	fr.rgbBlue=(BYTE)finalfor(2,0);
	fr.rgbReserved=(BYTE)0;
	forImage.SetPixelColor(i,j,fr);
	RGBQUAD ba;
	ba.rgbRed=(BYTE)finalback(0,0);
	ba.rgbGreen=(BYTE)finalback(1,0);
	ba.rgbBlue=(BYTE)finalback(2,0);
	ba.rgbReserved=(BYTE)0;
	backImage.SetPixelColor(i,j,ba);
}
double ProImage::getalpha(dMatrix f,dMatrix b,dMatrix c)//calculate alpha value using f b and c
{
	dMatrix cb=c-b;
	dMatrix fb=f-b;
	double length=fb(0,0)*fb(0,0)+fb(1,0)*fb(1,0)+fb(2,0)*fb(2,0);
	double rs=cb(0,0)*fb(0,0)+cb(1,0)*fb(1,0)+cb(2,0)*fb(2,0);
	if (fabs(length-0)<0.000001)
	{
		return -2.0;
	}
	return rs/length;
}
//get the equation matrix
void ProImage::getlrMat(dMatrix &lmat,dMatrix &rmat,const FNode &fm,const BNode &bm,double avgalpha,const dMatrix centercol)//得到方程组的系数矩阵lmat和rmat
{
	try
	{
		lmat.resize(6,6);
		rmat.resize(6,1);
		dMatrix I(3,3);
		I.unit();
		lmat[mslice(0,0,3,3)]=(!fm.cmatrixf)+I*((avgalpha*avgalpha)/(SD*SD));
		lmat[mslice(0,3,3,3)]=I*(avgalpha*(1-avgalpha)/(SD*SD));
		lmat[mslice(3,0,3,3)]=I*(avgalpha*(1-avgalpha)/(SD*SD));
		lmat[mslice(3,3,3,3)]=(!bm.cmatrixb)+I*((1-avgalpha)*(1-avgalpha)/(SD*SD));
		rmat[mslice(0,0,3,1)]=(!fm.cmatrixf)*fm.avgf+centercol*(avgalpha/(SD*SD));
		rmat[mslice(3,0,3,1)]=(!bm.cmatrixb)*bm.avgb+centercol*((1-avgalpha)/(SD*SD));
	}

	catch (exception &e)
	{
		cout<<e.what()<<endl;
	}

}
double ProImage::getAvgAlpha(long x,long y)
{
	int win=1;
	double aa=0.0;
	int num=0;
	for (int i=maxab(x-win,0);i<minab(x+win,width);i++)
	{
		for (int j=maxab(0,y-win);j<minab(y+win,height);j++)
		{
			if (tag[i][j])
			{
				aa+=alpha[i][j];
				num++;
			}
		}
	}
	return (num?aa/num:0);
}
double ProImage::getGi(long sx, long sy, long dx, long dy)
{
	double x=(sx-dx)*(sx-dx)+(sy-dy)*(sy-dy);
	return exp((-x)/(128.0));
}
//void ProImage::refine()
//{
//	for (int i=0;i<width;i++)
//	{
//		for (int j=0;j<height;j++)
//		{
//			RGBQUAD srgb=simage.GetPixelColor(i,j,true);
//			RGBQUAD drgb=dimage.GetPixelColor(i,j,true);
//			unsigned char c=timage.GetPixelGray(i,j);
//			double r=srgb.rgbRed*alpha[i][j]+(1-alpha[i][j])*drgb.rgbRed;
//			double g=srgb.rgbGreen*alpha[i][j]+(1-alpha[i][j])*drgb.rgbGreen;
//			double b=srgb.rgbBlue*alpha[i][j]+(1-alpha[i][j])*drgb.rgbBlue;
//			/*		double r=srgb.rgbRed*(c/255.0)+(1-c/255.0)*drgb.rgbRed;
//			double g=srgb.rgbGreen*(c/255.0)+(1-c/255.0)*drgb.rgbGreen;
//			double b=srgb.rgbBlue*(c/255.0)+(1-c/255.0)*drgb.rgbBlue;*/
//			drgb.rgbRed=(BYTE)r;
//			drgb.rgbGreen=(BYTE)g;
//			drgb.rgbBlue=(BYTE)b;
//			drgb.rgbReserved=(BYTE)0;
//			//dimage.SetPixelColor(i,j,drgb,false);
//		}
//	}
//}
//void ProImage::save()
//{
//
//	for (long i=0;i<width;i++)
//	{
//
//		for (long j=0;j<height;j++)
//		{
//			RGBQUAD temp;
//			temp.rgbRed=(BYTE)(alpha[i][j]*255);
//			temp.rgbGreen=(BYTE)(alpha[i][j]*255);
//			temp.rgbBlue=(BYTE)(alpha[i][j]*255);
//			temp.rgbReserved=(BYTE)0;
//			alphaImage.SetPixelColor(i,j,temp,true);
//		}
//	}
//	refine();
//	alphaImage.Save("d:\\alphaOK.png",CXIMAGE_FORMAT_PNG);
//	//dimage.Save("d:\\dimageOK.png",CXIMAGE_FORMAT_PNG);
//	cout<<"Image saved"<<endl;
//
//}
void ProImage::forMatrix(const std::vector<vector<colorNode> > &forvec, std::vector<FNode> &fmvec,long x,long y)
{
	for (size_t i=0;i<forvec.size();i++)
	{
		double W=0.0;//total of weight (square of alpha)
		FNode tempnode;
		dMatrix averagefor(3,1,0.0);
		dMatrix conviancefor(3,3,0.0);
		for (size_t j=0;j<forvec[i].size();j++)
		{
			double w=alpha[forvec[i][j].indexX][forvec[i][j].indexY]*alpha[forvec[i][j].indexX][forvec[i][j].indexY]*getGi(x,y,forvec[i][j].indexX,forvec[i][j].indexY);
			W+=w;
			averagefor+=forvec[i][j].colorinfo*w;

		}
		double inv_W=1.0/W;
		tempnode.avgf=averagefor*inv_W;
		for (size_t j=0;j<forvec[i].size();j++)
		{
			double w=alpha[forvec[i][j].indexX][forvec[i][j].indexY]*alpha[forvec[i][j].indexX][forvec[i][j].indexY]*getGi(x,y,forvec[i][j].indexX,forvec[i][j].indexY);
			conviancefor+=(forvec[i][j].colorinfo-tempnode.avgf)*(~(forvec[i][j].colorinfo-tempnode.avgf))*w;
		}
		tempnode.cmatrixf=conviancefor/W;
		fmvec.push_back(tempnode);
	}
}
void ProImage::backMatrix(const std::vector<vector<colorNode> > &backvec, std::vector<BNode> &bmvec,long x,long y)
{
	for (size_t i=0;i<backvec.size();i++)
	{
		double W=0;
		BNode tempnode;
		dMatrix averageback(3,1,0.0);
		dMatrix convianceback(3,3,0.0);
		for (size_t j=0;j<backvec[i].size();j++)
		{
			double w=(1-alpha[backvec[i][j].indexX][backvec[i][j].indexY])*(1-alpha[backvec[i][j].indexX][backvec[i][j].indexY])*getGi(x,y,backvec[i][j].indexX,backvec[i][j].indexY);
			W+=w;
			averageback+=backvec[i][j].colorinfo*w;
		}
		tempnode.avgb=averageback/W;
		for (size_t j=0;j<backvec[i].size();j++)
		{
			double w=(1-alpha[backvec[i][j].indexX][backvec[i][j].indexY])*(1-alpha[backvec[i][j].indexX][backvec[i][j].indexY])*getGi(x,y,backvec[i][j].indexX,backvec[i][j].indexY);
			convianceback+=(backvec[i][j].colorinfo-tempnode.avgb)*(~(backvec[i][j].colorinfo-tempnode.avgb))*w;
		}
		tempnode.cmatrixb=convianceback/W;
		bmvec.push_back (tempnode);
	}
}
/************************************************************************/
/* member for color cluster                                                           */
/************************************************************************/
dMatrix ProImage::calM(const vector<colorNode> &vec)
{

	dMatrix m(3,1,0.0);
	for(size_t i=0;i<vec.size();i++)
	{
		m=m+vec[i].colorinfo;
	}
	return m;

}
int ProImage::calN(const std::vector<colorNode> &vec)
{
	return (int)vec.size();
}
dMatrix ProImage::calR(const std::vector<colorNode> &vec)
{
	dMatrix r(3,3,0.0);
	for(size_t i=0;i<vec.size();i++)
	{
		r=r+(vec[i].colorinfo*(~vec[i].colorinfo));
	}
	return r;

}
void ProImage::cluster(vector<vector<colorNode >> & vec)
{
	for (int i=0;i<CLUSTERTIME;i++)
	{
		size_t maxindex=0;//the index of node with max eigenvalue
		dMatrix eigenvector(3,1,0.0);//eigenvector responding to max eigenvalue
		double lanbeida=numeric_limits<double>::min();
		dMatrix q(3,1,0.0);
		for (size_t j=0;j<vec.size();j++)
		{
			dMatrix m = calM(vec[j]);
			int n = calN(vec[j]);
			dMatrix r = calR(vec[j]);
			dMatrix rc=calRC(r,m,n);
			dMatrix eigenvec(3,3);
			dVector lanbeidavec(3);
			bool rec;
			rec=rc.eigen(lanbeidavec,eigenvec);
			double maxlan=lanbeidavec.max();
			if (lanbeida<maxlan)
			{
				lanbeida=maxlan;
				size_t inde;
				for (inde=0;inde<lanbeidavec.size();inde++)
				{
					if (lanbeidavec[inde]==maxlan)
					{
						break;
					}
				}
				eigenvector=eigenvec[mslice(0,inde,3,1)];
				maxindex=j;
				q=m/(double)n;
			}
		}
		vector<colorNode> newvec1;
		vector<colorNode> newvec2;
		for (size_t t=0;t<vec[maxindex].size();t++)
		{
			dMatrix temp1=(~eigenvector)*vec[maxindex][t].colorinfo;
			dMatrix temp2=(~eigenvector)*q;
			if(temp1(0,0)<=temp2(0,0))
			{
				newvec1.push_back(vec[maxindex][t]);

			}
			else
			{
				newvec2.push_back(vec[maxindex][t]);
			}
		}
		vec.erase(vec.begin()+maxindex);
		vec.push_back(newvec1);
		vec.push_back(newvec2);
	}
}
dMatrix ProImage::calRC(const dMatrix r, const dMatrix m, const int n)
{
	dMatrix mm=m*(~m);
	mm/=(double)n;
	return r-mm;
}
/************************************************************************/
/*                   end of  member for color cluster                              */
/************************************************************************/
bool ProImage::isvalid(long i,long j)
{
	if ((i<0)||(j<0)||(i>=width)||(j>=height))
	{
		return false;
	}
	return true;
}
