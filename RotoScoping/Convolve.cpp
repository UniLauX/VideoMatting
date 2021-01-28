#include "../RotoScoping/stdafx.h"
#include "../RotoScoping/Convolve.h"
//#include "WinDef.h"



void GenerateKernel(double sigma,ConvolveKernel & GaussKernel,ConvolveKernel & GaussDeriv){

	const double factor = 0.01;
	
	/*
	* Calculate the crude kernel
	*/
	int Radius = MAX_WINDOW_WIDTH/2;
	for (int i=-Radius;i<=Radius;i++)
	{
		GaussKernel.data[i + Radius] = exp(-i*i / (2.0 * sigma * sigma));
		GaussDeriv.data[i + Radius] = -i * GaussKernel.data[i + Radius];
	}
	GaussKernel.Width = MAX_WINDOW_WIDTH;
	GaussDeriv.Width = MAX_WINDOW_WIDTH;




	/*
	*  Find Max
	*/

	double MaxGaussian = 0.0;
	double MaxGauDeriv = 0.0;

	for (int i=0;i<MAX_WINDOW_WIDTH;i++)
	{
		if (fabs(GaussKernel.data[i]) > MaxGaussian)
		{
			MaxGaussian = fabs(GaussKernel.data[i]);
		}
		if (fabs(GaussDeriv.data[i]) > MaxGauDeriv)
		{
			MaxGauDeriv = fabs(GaussDeriv.data[i]);
		}
	}




	/*
	*  Crop the Kernel
	*/
	for (int i=0;fabs(GaussKernel.data[i]/MaxGaussian) < factor;i++)
	{
		GaussKernel.Width-=2;
	}
	for (int i=0;fabs(GaussDeriv.data[i]/MaxGauDeriv) < factor;i++)
	{
		GaussDeriv.Width-=2;
	}



	
	/*
	* Translate the Kernel
	*/
	for (int i=0;i<GaussKernel.Width;i++)
	{
		GaussKernel.data[i] = GaussKernel.data[i + (MAX_WINDOW_WIDTH - GaussKernel.Width)/2];
	}
	for (int i=0;i<GaussDeriv.Width;i++)
	{
		GaussDeriv.data[i] = GaussDeriv.data[ i + (MAX_WINDOW_WIDTH - GaussDeriv.Width)/2];
	}
	


	/*
	* Normalize the Kernel
	*/
	double sum=0.0;
	for (int i=0;i<GaussKernel.Width;i++)
	{
		sum += GaussKernel.data[i];
	}
	for (int i=0;i<GaussKernel.Width;i++)
	{
		GaussKernel.data[i] /= sum;
	}

	sum = 0.0;
	for (int i=0;i<GaussDeriv.Width;i++)
	{
		sum -= i*GaussDeriv.data[i];
	}
	for (int i=0;i<GaussDeriv.Width;i++)
	{
		GaussDeriv.data[i] /= sum;
	}

	printf("Gauss Kernel Width = %d , Gauss Deriv Kernel Width = %d\n",GaussKernel.Width,GaussDeriv.Width);
}
/*

static void ConvolveHori(double * ImgIn,ConvolveKernel &Kernel,double * Out){
	
	assert(Kernel.Width%2 == 1);

	int Width = ImgIn->width;
	int Height = ImgIn->height;
	
	double * OutPtr = (double *)Out->imageData;
	double * InPtr = (double *)ImgIn->imageData;
	double * InRowPtr;
	double * OutRowPtr;
	int Radiu = Kernel.Width/2;


	for (int y =0; y < Height; y++)
	{
		InRowPtr = InPtr;
		OutRowPtr = OutPtr;
		for (int x = 0; x<Radiu; x++)
		{
			*OutRowPtr++ = 0.0;
		}
		for (int x= Radiu;x<Width - Radiu;x++)
		{
			double sum = 0;
			double * ppp = InRowPtr + x - Radiu;
			for (int k = Kernel.Width-1;k>=0;k--)
			{
				sum += *ppp++ * Kernel.data[k];
			}
			*OutRowPtr++ = sum;
		}
		for (int x = Width - Radiu;x<Width;x++)
		{
			*OutRowPtr++ = 0.0;
		}
		InPtr += Width;
		OutPtr += Width;
	}
}


static void ConvolveVert(double * ImgIn,ConvolveKernel &Kernel,double * Out){
	assert(Kernel.Width%2 == 1);
	assert(ImgIn->width == Out->width);
	assert(ImgIn->height == Out->height);

	double * InPtr = (double *)ImgIn->imageData;
	double * OutPtr = (double *)Out->imageData;
	
	int Width = ImgIn->width;
	int Height = ImgIn->height;
	int Radius = Kernel.Width/2;

	for (int x = 0 ;  x < Width; x++)
	{
		double * InColHeadPtr = InPtr;
		double * OutColPtr = OutPtr;

		for (int y = 0; y < Radius; y++)
		{
			*OutColPtr = 0.0;
			OutColPtr += Width;
		}

		for (int y=Radius; y<Height - Radius; y++)
		{
			double sum=0.0;
			double * InPtr = InColHeadPtr + (y-Radius)* Width ;
			for (int k=Kernel.Width-1;k>=0;k--)
			{
				sum += *InPtr * Kernel.data[k];
				InPtr += Width;
			}
			*OutColPtr = sum;
			OutColPtr += Width;
		}
		
		for (int y=Height-Radius;y<Height;y++)
		{
			*OutColPtr = 0.0;
			OutColPtr += Width;
		}

		InPtr ++;
		OutPtr ++;
	}

}

static void ConvolveSeparate(
	double *ImgIn,
	ConvolveKernel & HoriKernel, 
	ConvolveKernel & VertKernel, 
	double * ImgOut)
{

		IplImage * TmpImage = cvCreateImage(cvGetSize(ImgIn),IPL_DEPTH_64F,1);

		ConvolveHori(ImgIn,HoriKernel,TmpImage);
		ConvolveVert(TmpImage,VertKernel,ImgOut);

		cvReleaseImage(&TmpImage);

}

static double SigmaPre = 0.0;
static ConvolveKernel GaussKernel;
static ConvolveKernel GaussDeriv;



void KLTGradientImage(IplImage * ImgIn, double sigma, IplImage * Gradx, IplImage * Grady){


	if (fabs(sigma - SigmaPre) >=0.2)    // Recalculate only when necessary
	{
		GenerateKernel(sigma,GaussKernel,GaussDeriv);
		SigmaPre = sigma;
	}
	ConvolveSeparate(ImgIn,GaussDeriv,GaussKernel,Gradx);
	ConvolveSeparate(ImgIn,GaussKernel,GaussDeriv,Grady);

	printf("Gauss Kernel Width = %d\n",GaussKernel.Width);
	printf("Gauss Derivative Kernel Width = %d\n",GaussDeriv.Width);
}



*/



double KernelX[49]=    //Derivative of Gaussian Kernel
{
	-0.0002  ,  -0.0013 ,   -0.0030  ,       0.0,   0.0030 ,  0.0013 ,  0.0002,
	-0.0020  ,  -0.0161 ,   -0.0360  ,       0.0,   0.0360 ,  0.0161 ,  0.0020,
	-0.0089  ,  -0.0719 ,   -0.1612  ,       0.0,   0.1612 ,  0.0719 ,  0.0089,
	-0.0146  ,  -0.1186 ,   -0.2658  ,       0.0,   0.2658 ,  0.1186 ,  0.0146,
	-0.0089  ,  -0.0719 ,   -0.1612  ,       0.0,   0.1612 ,  0.0719 ,  0.0089,
	-0.0020  ,  -0.0161 ,   -0.0360  ,       0.0,   0.0360 ,  0.0161 ,  0.0020,
	-0.0002  ,  -0.0013 ,   -0.0030  ,       0.0,   0.0030 ,  0.0013 ,  0.0002
};
double KernelY[49]=
{
	-0.0002 ,   -0.0020  ,  -0.0089  ,  -0.0146   , -0.0089  ,  -0.0020 ,   -0.0002,
	-0.0013 ,   -0.0161  ,  -0.0719  ,  -0.1186   , -0.0719  ,  -0.0161 ,   -0.0013,
	-0.0030 ,   -0.0360  ,  -0.1612  ,  -0.2658   , -0.1612  ,  -0.0360 ,   -0.0030,
	0.0    ,    0.0     ,   0.0     ,   0.0      ,  0.0     ,   0.0    ,    0.0   ,
	0.0030,     0.0360  ,   0.1612  ,   0.2658   ,  0.1612  ,   0.0360 ,    0.0030,
	0.0013,     0.0161  ,   0.0719  ,   0.1186   ,  0.0719  ,   0.0161 ,    0.0013,
	0.0002,     0.0020  ,   0.0089  ,   0.0146   ,  0.0089  ,   0.0020 ,    0.0002
};

double ConvolveX(double * Image,int Cx,int Cy,int Width){
	int Index;
	double Result = 0.0;
	for (int r=0,dr=-3;r<7;r++,dr++)
	{
		for (int c=0,dc=-3;c<7;c++,dc++)
		{
			Index = (Cy + dr) * Width + Cx + dc;
			Result += Image[Index]*KernelX[r*7 + c];
		}
	}	

	//cout << Result << endl;
	//system("pause");
	return Result;

}

double ConvolveY(double * Image,int Cx,int Cy,int Width){
	int Index;
	double Result = 0.0;
	for (int r=0,dr=-3;r<7;r++,dr++)
	{
		for (int c=0,dc=-3;c<7;c++,dc++)
		{
			Index = (Cy + dr) * Width + Cx + dc;
			Result += Image[Index]*KernelY[r*7 + c];
		}
	}
	return Result;

}

double ConvolveX(unsigned char * Image,int Cx,int Cy,int Width, int Height){
	int Index;
	double Result = 0.0;
	for (int r=0,dr=-3;r<7;r++,dr++)
	{
		for (int c=0,dc=-3;c<7;c++,dc++)
		{
			int ct, rt;
			ct = __max(0,__min(Cx + dc, Width - 1));
			rt = __max(0,__min(Cy + dr, Height - 1));
			//Index = (Cy + dr) * Width + Cx + dc;
			Index = rt * Width + ct;
			Result += int(Image[Index])*KernelX[r*7 + c];
		}
	}	

	//cout << Result << endl;
	//system("pause");
	return Result;

}

double ConvolveY(unsigned char * Image,int Cx,int Cy,int Width, int Height){
	int Index;
	double Result = 0.0;
	for (int r=0,dr=-3;r<7;r++,dr++)
	{
		for (int c=0,dc=-3;c<7;c++,dc++)
		{
			int ct, rt;
			ct = __max(0,__min(Cx + dc, Width - 1));
			rt = __max(0,__min(Cy + dr, Height - 1));
			//Index = (Cy + dr) * Width + Cx + dc;
			Index = rt * Width + ct;
			Result += int(Image[Index])*KernelY[r*7 + c];
		}
	}
	return Result;

}





double ConvolveX(double * Image,int Cx,int Cy,const ConvolveKernel& Gauss,const ConvolveKernel & GaussDeriv,int Width){

	int Radius = Gauss.Width/2;

	double VertSum = 0.0;
	for (int yRad = -Radius;yRad <= Radius; yRad++)
	{
		int y = Cy + yRad;
		int yIndex = y* Width;
		double HoriSum =0.0;
		for (int xRad = -Radius; xRad <= Radius; xRad ++)
		{
			int x = Cx + xRad;
			HoriSum += GaussDeriv.data[xRad + Radius] * Image[yIndex + x];
		}

		VertSum += HoriSum * Gauss.data[yRad + Radius];
		
	}

	return VertSum;

}

double ConvolveY(double * Image,int Cx,int Cy,const ConvolveKernel& Gauss,const ConvolveKernel & GaussDeriv,int Width){

	int Radius = Gauss.Width/2;

	double VertSum = 0.0;
	for (int yRad = -Radius;yRad <= Radius; yRad++)
	{
		int y = Cy + yRad;
		int yIndex = y* Width;
		double HoriSum =0.0;
		for (int xRad = -Radius; xRad <= Radius; xRad ++)
		{
			int x = Cx + xRad;
			HoriSum += Gauss.data[xRad + Radius] * Image[yIndex + x];
		}

		VertSum += HoriSum * GaussDeriv.data[yRad + Radius];

	}

	return VertSum;


}

double ConvolveX(unsigned char * Image,int Cx,int Cy,const ConvolveKernel& Gauss,const ConvolveKernel & GaussDeriv,int Width){


	int Radius = Gauss.Width/2;

	double VertSum = 0.0;
	for (int yRad = -Radius;yRad <= Radius; yRad++)
	{
		int y = Cy + yRad;
		int yIndex = y* Width;
		double HoriSum =0.0;
		for (int xRad = -Radius; xRad <= Radius; xRad ++)
		{
			int x = Cx + xRad;
			HoriSum += GaussDeriv.data[xRad + Radius] * int(Image[yIndex + x]);
		}

		VertSum += HoriSum * Gauss.data[yRad + Radius];

	}

	return VertSum;







}

double ConvolveY(unsigned char * Image,int Cx,int Cy,const ConvolveKernel& Gauss,const ConvolveKernel & GaussDeriv,int Width){

	int Radius = Gauss.Width/2;

	double VertSum = 0.0;
	for (int yRad = -Radius;yRad <= Radius; yRad++)
	{
		int y = Cy + yRad;
		int yIndex = y* Width;
		double HoriSum =0.0;
		for (int xRad = -Radius; xRad <= Radius; xRad ++)
		{
			int x = Cx + xRad;
			HoriSum += GaussDeriv.data[xRad + Radius] * int(Image[yIndex + x]);
		}

		VertSum += HoriSum * Gauss.data[yRad + Radius];

	}

	return VertSum;


}