//#include "RotoScoping.h"
//#include "cximage.h"
//#include "nurbs.h"
//#include "levmar.h"
//#include "ImageSequenceManager.h"
//#include "ImageBuffer.h"
//#include "NurbsCurveDataManager.h"
//#include <iostream>
//
//#include "highgui.h"
//using std::cout;
//#define  GAP 5
//
//ImageSequenceManager RotoScoping::imageSequnce;
////vector<Nurbs> RotoScoping::vecNurbs;
//Nurbs * RotoScoping::vecNurbs = NULL;
//int RotoScoping::kdown = 0;
//int RotoScoping::kup = 0;
//int RotoScoping::pnum = 0;
//int RotoScoping::xnum = 0;
//int RotoScoping::imageheight = 0;
//int RotoScoping::imagewidth = 0;
//double RotoScoping::K = 0;
//double * RotoScoping::M = 0;
//int RotoScoping::taIndex = 0;
//int RotoScoping::tbIndex = 0;
//int RotoScoping::poiNumOfPerFrame = 0;
//ON_2dPoint * RotoScoping::Direct = 0;
//double * RotoScoping::position = 0;
//ON_2dPoint * RotoScoping::posTa = 0;
//ON_2dPoint * RotoScoping::posTb = 0;
//NurbsCurveDataManager * RotoScoping::curveDataManager = NULL;
//
//IplImage ** RotoScoping::img = NULL;
//
//
//int times = 0;
//
//RotoScoping::RotoScoping()
//{
//
//	
//	
//}
//
//RotoScoping::~RotoScoping()
//{
//	printf("~Rotoscoping! \n");
//}
//
//void RotoScoping::initialize()
//{
//	K = 1081 ;
//	taIndex = -1;
//	tbIndex = -1;
//	kup = 5;
//	kdown = -5;
//	imageSequnce.loadImage(0, 1);
//	//printf("%s", imageSequnce.pathname);
//	imageheight = imageSequnce.getImageBuffer(0)->GetImage()->GetHeight();
//	imagewidth  = imageSequnce.getImageBuffer(0)->GetImage()->GetWidth();
//	//printf("%s", imageSequnce.pathname);
//
//}
//
//void RotoScoping::setNurbsNum(int num)
//{
//	curveDataManager = new NurbsCurveDataManager;
//	curveDataManager->initilaizeCurveData(num);
//	//Direct   = new ON_2dPoint[num];
//	//position = new double[num];
//}
//
//void RotoScoping::initPointLocation( int ta, int tb )
//{
//	int number = curveDataManager->getCurveData(ta).getNumOfData();
//	
//	//cout << number << endl;
//	int * num = new int[number];
//	for (int i = 0; i < number; ++i)
//	{
//		//cout << "i:" << i << endl;
//		int nextindex = (i + 1) %number;
//		ON_2dPoint p = curveDataManager->getCurveData(ta).getPoi(i);
//		ON_2dPoint q = curveDataManager->getCurveData(ta).getPoi(nextindex);
//		num[i] = (int)(sqrt(distance2(p, q)) / GAP);
//	}
//
//	curveDataManager->getCurveData(ta).generateSamplePoint(num);
//	curveDataManager->getCurveData(tb).generateSamplePoint(num);
//	poiNumOfPerFrame = curveDataManager->getCurveData(ta).index[curveDataManager->getCurveData(ta).index.size()-1];
//	
//	IplImage * image = cvCreateImage(cvSize(imagewidth, imageheight),IPL_DEPTH_8U, 3);
//	for (int i = 0; i < image->height; ++i)
//	{
//		for(int j = 0; j < image->width; ++j)
//		{
//			CV_IMAGE_ELEM(image, uchar, i, j * image->nChannels) = 255;
//			CV_IMAGE_ELEM(image, uchar, i, j * image->nChannels+1) = 255;
//			CV_IMAGE_ELEM(image, uchar, i, j * image->nChannels+2) = 255;
//		}
//	}
//	for (int k = 10; k < poiNumOfPerFrame; ++k)
//	{
//		int j = curveDataManager->getCurveData(tb).getSamplePoint(k).x;
//		int i = curveDataManager->getCurveData(tb).getSamplePoint(k).y;
//
//		CV_IMAGE_ELEM(image, uchar, i, j * image->nChannels) = 0;
//		CV_IMAGE_ELEM(image, uchar, i, j * image->nChannels+1) = 0;
//		CV_IMAGE_ELEM(image, uchar, i, j * image->nChannels+2) = 0;
//	}
//	//cvNamedWindow("a");
//	//cvShowImage("a",image);
//
//	int d = tb - ta;
//
//	for (int i = ta + 1; i < tb; ++i)
//	{
//		//cout << i << endl;
//		curveDataManager->getCurveData(i).setIndexAccordParaNum(num, number);
//		curveDataManager->getCurveData(i).clearSamplePoint();
//		curveDataManager->getCurveData(i).clearSampleDirect();
//		//cout << "1#" << endl;
//		/*double ia = abs((double)(ta - i)) / d;
//		double ib = abs((double)(tb - i)) / d;*/
//		double ia = 1;
//		double ib = 0;
//		for (int j = 0; j < poiNumOfPerFrame; ++j)
//		{
//			//cout << "j: " << j << endl;
//			ON_2dPoint cp;
//			cp.x = curveDataManager->getCurveData(ta).getSamplePoint(j).x * ia + curveDataManager->getCurveData(tb).getSamplePoint(j).x * ib;
//			cp.y = curveDataManager->getCurveData(ta).getSamplePoint(j).y * ia + curveDataManager->getCurveData(tb).getSamplePoint(j).y * ib;
//			ON_2dPoint dp;
//			dp.x = curveDataManager->getCurveData(ta).getSampleDirect(j).x * ia +  curveDataManager->getCurveData(tb).getSampleDirect(j).x * ib;
//			dp.y = curveDataManager->getCurveData(ta).getSampleDirect(j).y * ia +  curveDataManager->getCurveData(tb).getSampleDirect(j).y * ib;
//			
//			
//			curveDataManager->getCurveData(i).push_backSamplePoint(j, cp);
//			curveDataManager->getCurveData(i).push_backSampleDirect(j, dp);
//		}
//	}
//}
//
//double RotoScoping::eLsigleTerm(ON_2dPoint ti, ON_2dPoint ti1, ON_2dPoint t1i, ON_2dPoint t1i1)
//{
//
//	double temp = distance2(ti1, ti) - distance2(t1i1, t1i);
//	return temp;
//}
//
//double RotoScoping::eCsigleTerm(ON_2dPoint ti, ON_2dPoint ti1, ON_2dPoint ti2, ON_2dPoint t1i, ON_2dPoint t1i1, ON_2dPoint t1i2)
//{
//	
//		ON_2dPoint p1, p2;
//		p1.x = ti.x - 2 * ti1.x + ti2.x;
//		p1.y = ti.y - 2 * ti1.y + ti2.y;
//		p2.x = t1i.x - 2 * t1i1.x + t1i2.x;
//		p2.y = t1i.y - 2 * t1i1.y + t1i2.y;
//		return sqrt(distance2(p1, p2));
//}
//
//double RotoScoping::eVsigleTerm( ON_2dPoint ti, ON_2dPoint t1i )
//{
//	return sqrt(distance2(ti, t1i));
//}
//
//double RotoScoping::eIsigleTerm( int t, int i, int k, ON_2dPoint ti, ON_2dPoint t1i )
//{
//	/*if (i < poiNumOfPerFrame)
//	{*/
//		double value = 0;
//		CPoint nt, ntplus;
//		ON_2dPoint p1, p2;
//		ON_2dPoint d;
//		double dis;
//
//		d  = curveDataManager->getCurveData(t).getSampleDirect(i);
//		nt.x = - d.y;
//		nt.y = d.x;
//
//		int tplus = t + 1;
//		d  = curveDataManager->getCurveData(tplus).getSampleDirect(i);
//		ntplus.x = - d.y;
//		ntplus.y = d.x;
//
//
//		double stepinc1 = min(abs(1 / (nt.x + 1e-6)), abs(1 / (nt.y + 1e-6)));
//		double stepinc2 = min(abs(1 / (ntplus.x + 1e-6)), abs(1 / (ntplus.y + 1e-6)));
//
//		int ei, ej;
//		int epi, epj;
//		int r, g, b;
//		int rt, gt, bt;
//		/*for (int k = kdown; k <= kup; ++k)
//		{*/
//			double temp = 0;
//			double incj = nt.x * stepinc1 * k;
//			double inci = nt.y * stepinc1 * k;
//			ej = int(ti.x + incj + 0.5);
//			ei = int(ti.y + inci + 0.5);
//			if ( !isInImage(ei, ej))
//			{
//				return 0;
//			}
//			incj = ntplus.x * stepinc2 * k;
//			inci = ntplus.y * stepinc2 * k;
//			epj = int(t1i.x + incj + 0.5);
//			epi = int(t1i.y + inci + 0.5);
//			if ( !isInImage(epi, epj))
//			{
//				return 0;
//			}
//			int tei = imageheight - ei - 1;
//			int tepi = imageheight - epi - 1;
//			double r1 = imageSequnce.getImageBuffer(t)->GetImage()->GetPixelColor(ej, tei).rgbRed;
//			double g1 = imageSequnce.getImageBuffer(t)->GetImage()->GetPixelColor(ej, tei).rgbGreen;
//			double b1 = imageSequnce.getImageBuffer(t)->GetImage()->GetPixelColor(ej, tei).rgbBlue;
//			double r2 = imageSequnce.getImageBuffer(t+1)->GetImage()->GetPixelColor(epj, tepi).rgbRed;
//			double g2 = imageSequnce.getImageBuffer(t+1)->GetImage()->GetPixelColor(epj, tepi).rgbGreen;
//			double b2 = imageSequnce.getImageBuffer(t+1)->GetImage()->GetPixelColor(epj, tepi).rgbBlue;
//			
//			/*int index = t - taIndex;
//			double cvb1 = CV_IMAGE_ELEM(img[index], uchar, ei, ej * img[index]->nChannels);
//			double cvg1 = CV_IMAGE_ELEM(img[index], uchar, ei, ej * img[index]->nChannels+1);
//			double cvr1 = CV_IMAGE_ELEM(img[index], uchar, ei, ej * img[index]->nChannels+2);
//
//			if (abs(r1 - cvr1 ) > 1 || abs(cvg1 - g1) > 1 || abs(cvb1 - b1) > 1)
//			{
//				cout << "Cxiamge:" << r1 << " " << g1 << " " << b1 << endl;
//				cout << "OpenCv:" << cvr1 << " " << cvg1 << " " << cvb1 << endl;
//			}*/
//
//
//
//			temp = (r1 - r2) * (r1 - r2) + 
//				   (g1 - g2) * (g1 - g2) +
//				   (b1 - b2) * (b1 - b2);
//			value += temp;
//			//cout << value << endl;
//		/*}*/
//		return sqrt(value);
//	/*}*/
//	
//}
//
//double RotoScoping::eGsigleTerm( int t, ON_2dPoint ti, double m )
//{
//	double g = gradient(imageSequnce.getImageBuffer(t)->GetImage(), ti.y, ti.x);
//	double tg = K - g;
//	double temp = tg / m;
//	return temp;
//}
//
//double RotoScoping::distance2( ON_2dPoint& p1, ON_2dPoint& p2 )
//{
//	return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
//}
//void RotoScoping::pointFitToImage( int& i, int& j )
//{
//	i = max(min(imageheight - 1, i), 0);
//	j = max(min(imagewidth - 1, j), 0);
//}
//
//bool RotoScoping::isInImage( int i, int j )
//{
//	if (i > 0 && i < imageheight && j > 0 && j < imagewidth)
//	{
//		return true;
//	}
//	return false;
//}
//
//double RotoScoping::gradient( CxImage * image, int i, int j )
//{
//	//选择单一方向的
//	//int ti, tj;
//	//if (i > imageheight - 1)
//	//{
//	//	ti = i - 1;
//	//}
//	//else
//	//{
//	//	ti = i + 1;
//
//	//}
//	//if (j > imagewidth - 1)
//	//{
//	//	tj = j - 1;
//	//}
//	//else
//	//{
//	//	tj = j + 1;
//	//}
//	//i = imageheight - 1 - i;
//	//ti = imageheight - 1 - ti;
//	//double r = image->GetPixelColor(j, i).rgbRed;
//	//double g = image->GetPixelColor(j, i).rgbGreen;
//	//double b = image->GetPixelColor(j, i).rgbBlue;
//
//	//double tempr = image->GetPixelColor(j, ti).rgbRed;
//	//double tempg = image->GetPixelColor(j, ti).rgbGreen;
//	//double tempb = image->GetPixelColor(j, ti).rgbBlue;
//
//	////double gi = abs(r - tempr) + abs(g - tempg) + abs(b - tempb);
//
//	//double tempr1 = image->GetPixelColor(tj, i).rgbRed;
//	//double tempg1 = image->GetPixelColor(tj, i).rgbGreen;
//	//double tempb1 = image->GetPixelColor(tj, i).rgbBlue;
//
//	////double gj = abs(r - tempr) + abs(g - tempg) + abs(b - tempb);
//
//	//return sqrt((r - tempr) * (r - tempr) + (r - tempr1) * (r - tempr1)) + 
//	//	   sqrt((g - tempg) * (g - tempg) + (g - tempg1) * (g - tempg1)) + 
//	//	   sqrt((b - tempb) * (b - tempb) + (b - tempb1) * (b - tempb1));
//
//	//选取最大梯度方向
//	double maxg = 0;
//	int i1 = max(i-1, 0);
//	int i2 = min(i+1, imageheight-1);
//	int j1 = max(j-1, 0);
//	int j2 = min(j+1, imagewidth-1);
//
//	int ti = imageheight - 1 - i;
//	int ti1 = imageheight - 1 - i1;
//	int ti2 = imageheight - 1 - i2;
//	//double r = image->GetPixelColor(j, i).rgbRed;
//	//double g = image->GetPixelColor(j, i).rgbGreen;
//	//double b = image->GetPixelColor(j, i).rgbBlue;
//
//	double xlr = image->GetPixelColor(j1, ti).rgbRed;
//	double xlg = image->GetPixelColor(j1, ti).rgbGreen;
//	double xlb = image->GetPixelColor(j1, ti).rgbBlue;
//
//	double xrr = image->GetPixelColor(j2, ti).rgbRed;
//	double xrg = image->GetPixelColor(j2, ti).rgbGreen;
//	double xrb = image->GetPixelColor(j2, ti).rgbBlue;
//
//	double yur = image->GetPixelColor(j, ti1).rgbRed;
//	double yug = image->GetPixelColor(j, ti1).rgbGreen;
//	double yub = image->GetPixelColor(j, ti1).rgbBlue;
//
//	double ydr = image->GetPixelColor(j, ti2).rgbRed;
//	double ydg = image->GetPixelColor(j, ti2).rgbGreen;
//	double ydb = image->GetPixelColor(j, ti2).rgbBlue;
//
//	int deli = i2 - i1;
//	int delj = j2 - j1;
//
//	return sqrt((xrr - xlr) * (xrr - xlr) / (delj * delj)+ (yur - ydr) * (yur - ydr) / (deli * deli)) + 
//		   sqrt((xrg - xlg) * (xrg - xlg) / (delj * delj)+ (yug - ydg) * (yug - ydg) / (deli * deli)) + 
//		   sqrt((xrb - xlb) * (xrb - xlb) / (delj * delj)+ (yub - ydb) * (yub - ydb) / (deli * deli));
//			
//
//
//
//}
//
//double RotoScoping::computSingleM( int ta, int tb, int i )
//{
//	double ga = gradient(imageSequnce.getImageBuffer(ta)->GetImage(), curveDataManager->getCurveData(ta).getSamplePoint(i).y, curveDataManager->getCurveData(ta).getSamplePoint(i).x);
//	double gb = gradient(imageSequnce.getImageBuffer(tb)->GetImage(), curveDataManager->getCurveData(tb).getSamplePoint(i).y, curveDataManager->getCurveData(tb).getSamplePoint(i).x);
//	double a = K - ga;
//	double b = K - gb;
//	
//	return min(a, b);
//}
//
//void RotoScoping::computM( int ta, int tb )
//{
//	computeK(ta);
//	
//	for (int i = 0; i < poiNumOfPerFrame; ++i)
//	{
//		M[i] = computSingleM(ta, tb, i);
//		/*if (M[i]==K)
//		{
//			while (true)
//			{
//				cout << "___" << endl;
//			}
//		}*/
//	}
//}
//
//void RotoScoping::func( double * p, double *x, int m, int n, void *data )
//{
//	int xindex = 0;
//
//	double wl = sqrt(0.001);
//	double wc = sqrt(0.001);
//	double wv = sqrt(10.001);
//	double wi = sqrt(0.001);
//	double wg = sqrt(1000.001);
//
//	//cout << "el" << endl;
//	//eL
//	//cout << "1" << endl;
//	for (int i = 0; i < poiNumOfPerFrame - 1; ++i)
//	{
//		x[xindex] = wl * eLsigleTerm(curveDataManager->getCurveData(taIndex).getSamplePoint(i), curveDataManager->getCurveData(taIndex).getSamplePoint(i+1), 
//			                    ON_2dPoint(p[2*i], p[2*i+1]), ON_2dPoint(p[2*(i+1)], p[2*(i+1)+1]));
//		xindex++;
//	}
//	//cout << "2" << endl;
//	for (int i = 0; i < tbIndex - taIndex - 2; ++i)
//	{
//		for (int j = 0; j < poiNumOfPerFrame - 1; ++j)
//		{
//			int indexti = 2 * i * poiNumOfPerFrame + 2 * j;
//			int indextpi = 2 * (i + 1) * poiNumOfPerFrame + 2 * j;
//			x[xindex] = wl * eLsigleTerm(ON_2dPoint(p[indexti], p[indexti+1]), ON_2dPoint(p[indexti+2], p[indexti+3]),
//				                    ON_2dPoint(p[indextpi], p[indextpi+1]), ON_2dPoint(p[indextpi+2],p[indextpi+3]));
//			xindex++;
//			//cout << j << endl;
//		}
//	}
//	//cout << "3" << endl;
//	for (int i = 0; i < poiNumOfPerFrame - 1; ++i)
//	{
//		int indexti = 2 * (tbIndex - taIndex - 2) * poiNumOfPerFrame + 2 * i;
//		//cout << indexti << endl;
//		//cout << p[indexti+3] << endl;
//		x[xindex] = wl * eLsigleTerm(ON_2dPoint(p[indexti], p[indexti+1]), ON_2dPoint(p[indexti+2],p[indexti+3]),
//			                    curveDataManager->getCurveData(tbIndex).getSamplePoint(i), curveDataManager->getCurveData(tbIndex).getSamplePoint(i+1));
//		xindex++;
//	}
//	
//	//cout << "ec" << endl;
//	//eC
//	for (int i = 0; i < poiNumOfPerFrame - 2; ++i)
//	{
//		int indexti = 2 * i;
//		x[xindex] = wc * eCsigleTerm(curveDataManager->getCurveData(taIndex).getSamplePoint(i), curveDataManager->getCurveData(taIndex).getSamplePoint(i+1), curveDataManager->getCurveData(taIndex).getSamplePoint(i+2), 
//			                    ON_2dPoint(p[indexti], p[indexti+1]), ON_2dPoint(p[indexti+2], p[indexti+3]), ON_2dPoint(p[indexti+4], p[indexti+5]));
//		xindex++;
//	}
//	for (int i = 0; i < tbIndex - taIndex - 2; ++i)
//	{
//		for (int j = 0; j < poiNumOfPerFrame - 2; ++j)
//		{
//			int indexti = 2 * i * poiNumOfPerFrame + 2 * j;
//			int indextpi = 2 * (i + 1) * poiNumOfPerFrame + 2 * j;
//			x[xindex] = wc * eCsigleTerm(ON_2dPoint(p[indexti], p[indexti+1]), ON_2dPoint(p[indexti+2], p[indexti+3]), ON_2dPoint(p[indexti+4], p[indexti+5]),
//				                    ON_2dPoint(p[indextpi], p[indextpi+1]), ON_2dPoint(p[indextpi+2],p[indextpi+3]), ON_2dPoint(p[indextpi+4], p[indextpi+5]));
//			xindex++;
//		}
//	}
//
//	for (int i = 0; i < poiNumOfPerFrame - 2; ++i)
//	{
//		int indexti = 2 * (tbIndex - taIndex - 2) * poiNumOfPerFrame + 2 * i;
//		x[xindex] = wc * eCsigleTerm(ON_2dPoint(p[indexti], p[indexti+1]), ON_2dPoint(p[indexti+2], p[indexti+3]), ON_2dPoint(p[indexti+4], p[indexti+5]),
//			                    curveDataManager->getCurveData(tbIndex).getSamplePoint(i), curveDataManager->getCurveData(tbIndex).getSamplePoint(i+1), curveDataManager->getCurveData(tbIndex).getSamplePoint(i+2));
//		xindex++;
//	}
//
//	//cout << "ev" << endl;
//	//eV
//	int iv = xindex;
//	for (int i = 0; i < poiNumOfPerFrame; ++i)
//	{
//		x[xindex] = wv * eVsigleTerm(curveDataManager->getCurveData(taIndex).getSamplePoint(i), 
//			                    ON_2dPoint(p[2*i], p[2*i+1]));
//		xindex++;
//	}
//	for (int i = 0; i < tbIndex - taIndex - 2; ++i)
//	{
//		for (int j = 0; j < poiNumOfPerFrame; ++j)
//		{
//			int indexti = 2 * i * poiNumOfPerFrame + 2 * j;
//			int indextpi = 2 * (i + 1) * poiNumOfPerFrame + 2 * j;
//			x[xindex] = wv * eVsigleTerm(ON_2dPoint(p[indexti], p[indexti+1]), 
//				                    ON_2dPoint(p[indextpi], p[indextpi+1]));
//			xindex++;
//		}
//	}
//
//	for (int i = 0; i < poiNumOfPerFrame; ++i)
//	{
//		int indexti = 2 * (tbIndex - taIndex - 2) * poiNumOfPerFrame + 2 * i;
//		x[xindex] = wv * eVsigleTerm(ON_2dPoint(p[indexti], p[indexti+1]),
//			                    curveDataManager->getCurveData(tbIndex).getSamplePoint(i));
//		xindex++;
//	}
//
//	/*for (int i = iv; i < xindex; ++i)
//	{
//		cout << x[i] << " ";
//	}*/
//
//
//	//cout << "ei" << endl;
//	//eI
//	//int ii = xindex;
//	for (int i = 0; i < poiNumOfPerFrame; ++i)
//	{
//		for (int k = kdown; k <= kup; ++k)
//		{
//			x[xindex] = wi * eIsigleTerm(taIndex, i, k, curveDataManager->getCurveData(taIndex).getSamplePoint(i), ON_2dPoint(p[2*i], p[2*i+1]));
//			xindex++;
//		}
//	}
//	for (int i = 0; i < tbIndex - taIndex - 2; ++i)
//	{
//		for (int j = 0; j < poiNumOfPerFrame; ++j)
//		{
//			int indexti = 2 * i * poiNumOfPerFrame + 2 * j;
//			int indextpi = 2 * (i + 1) * poiNumOfPerFrame + 2 * j;
//			for (int k = kdown; k <= kup; ++k)
//			{
//				x[xindex] = wi * eIsigleTerm(taIndex + 1 + i, j, k, ON_2dPoint(p[indexti], p[indexti+1]), ON_2dPoint(p[indextpi], p[indextpi+1]));
//				xindex++;
//			}
//		}
//	}
//
//	for (int i = 0; i < poiNumOfPerFrame; ++i)
//	{
//		int indexti = 2 * (tbIndex - taIndex - 2) * poiNumOfPerFrame + 2 * i;
//		for (int k = kdown; k <= kup; ++k)
//		{
//			x[xindex] = wi * eIsigleTerm(tbIndex - 1, i, k, ON_2dPoint(p[indexti], p[indexti+1]), curveDataManager->getCurveData(tbIndex).getSamplePoint(i));
//			xindex++;
//		}
//	}
//	/*for (int i = ii; i < xindex; ++i)
//	{
//		cout << x[i] << " ";
//	}*/
//
//	//cout << "eg" << endl;
//	//eG
//	int ig = xindex;
//	for (int i = 0; i < poiNumOfPerFrame; ++i)
//	{
//		//computSingleM(taIndex, tbIndex, i);
//		x[xindex] = wg * eGsigleTerm(taIndex, curveDataManager->getCurveData(taIndex).getSamplePoint(i), M[i]);
//		//cout << x[xindex] << " ";
//		xindex++;
//	}
//	for (int i = 0; i < tbIndex - taIndex - 1; ++i)
//	{
//		for (int j = 0; j < poiNumOfPerFrame; ++j)
//		{
//			//computSingleM(taIndex, tbIndex, j);
//			int indexti = 2 * i * poiNumOfPerFrame + 2 * j;
//			x[xindex] = wg * eGsigleTerm(taIndex + i + 1, ON_2dPoint(p[indexti], p[indexti+1]), M[j]);
//			//cout << x[xindex] << " ";
//			xindex++;
//		}
//	}
//	for (int i = 0; i < poiNumOfPerFrame; ++i)
//	{
//		//computSingleM(taIndex, tbIndex, i);
//		x[xindex] = wg * eGsigleTerm(tbIndex, curveDataManager->getCurveData(tbIndex).getSamplePoint(i), M[i]);
//		//cout << x[xindex] << " ";
//		xindex++;
//	}
//	//cout  << xindex << endl;
//	/*for (int i = ig; i < xindex; ++i)
//	{
//		cout << x[i] << " ";
//	}*/
//
//	//cout << "func over!" << endl;
//	//cout << ++times << endl;
//}
//
//
//void RotoScoping::initilaizePostion()
//{
//	int index = 0;
//	for(int i = taIndex + 1; i < tbIndex; ++i)
//	{
//		for(int j = 0; j < poiNumOfPerFrame; ++j)
//		{
//			//cout << poiNumOfPerFrame;
//			position[index++] = curveDataManager->getCurveData(i).getSamplePoint(j).x;
//			position[index++] = curveDataManager->getCurveData(i).getSamplePoint(j).y;
//			cout << curveDataManager->getCurveData(i).getSamplePoint(j).x << " " << curveDataManager->getCurveData(i).getSamplePoint(j).y << endl;
//			/*if (position[index - 2] == 0 || position[index - 1] == 0)
//			{
//				cout << "asdasd " << index << endl;
//			}*/
//		}
//		//cout <<  " pindex"<< index << endl;
//	}
//
//	/*for (int i = taIndex; i < tbIndex; ++i)
//	{
//		for (int j = 0; j < poiNumOfPerFrame;++j)
//		{
//			int ii = curveDataManager->getCurveData(i).getSamplePoint(j).y;
//			int ji = curveDataManager->getCurveData(i).getSamplePoint(j).x;
//			CV_IMAGE_ELEM(img[i - taIndex], uchar, ii, ji*img[i - taIndex]->nChannels) = 0;
//			CV_IMAGE_ELEM(img[i - taIndex], uchar, ii, ji*img[i - taIndex]->nChannels+1) = 0;
//			CV_IMAGE_ELEM(img[i - taIndex], uchar, ii, ji*img[i - taIndex]->nChannels+2) = 0;
//		}
//		cvNamedWindow("aa");
//		cvShowImage("aa", img[i - taIndex]);
//		cvWaitKey(0);
//	}*/
//}
//
//void RotoScoping::rotoScope( int ta, int tb )
//{
//	taIndex = ta;
//	tbIndex = tb;
//	img = new IplImage *[tb-ta+1];
//	imageSequnce.loadImage(ta, tb - ta + 1);
//	for (int i = ta; i <= tb; ++i)
//	{
//		showImage(*(imageSequnce.getImageBuffer(i)->GetImage()), i-ta);
//
//	}
//	initPointLocation(ta, tb);
//	//poiNumOfPerFrame = curveDataManager->getCurveData(ta).index[curveDataManager->getCurveData(ta).index.size()-1];
//	//因为使用闭合曲线， 所以xum值可以进行增加，此处暂不修改
//	xnum = (tb - ta) * (poiNumOfPerFrame - 1)
//		        + (tb - ta) * (poiNumOfPerFrame - 2)
//				+ (tb - ta) * poiNumOfPerFrame
//				+ (tb - ta) * poiNumOfPerFrame * (kup - kdown  + 1)
//				+ (tb - ta + 1) * poiNumOfPerFrame;
//	double * x = new double[xnum];
//
//	pnum = (tb - ta - 1) * poiNumOfPerFrame * 2;
//	position = new double[pnum];
//
//	M = new double[poiNumOfPerFrame];
//	computM(ta, tb);
//	////Direct   = new ON_2dPoint[(tb - ta - 1) * poiNumOfPerFrame];
//
//
//	cout << "p:" << pnum << endl;
//	cout << "x:" << xnum << endl;
//
//
//	double opts[LM_OPTS_SZ], info[LM_INFO_SZ];
//	opts[0]=LM_INIT_MU; opts[1]=1E-15; opts[2]=1E-15; opts[3]=1E-20;
//	opts[4]= LM_DIFF_DELTA;
//
//	initilizeX(x, xnum);
//	
//	initilaizePostion();
//	cout << "lm begin!" << endl;
//	dlevmar_dif(func, position, x, pnum, xnum, 1000, opts, info, NULL, NULL, NULL);
//	//printf("Results for %s:\n", probname[problem]);
//	   /*info[0]= ||e||_2 at initial p.
//		* info[1-4]=[ ||e||_2, ||J^T e||_inf,  ||Dp||_2, \mu/max[J^T J]_ii ], all computed at estimated p.
//		* info[5]= # iterations,
//		* info[6]=reason for terminating: 1 - stopped by small gradient J^T e
//		*                                 2 - stopped by small Dp
//		*                                 3 - stopped by itmax
//		*                                 4 - singular matrix. Restart from current p with increased \mu
//		*                                 5 - no further error reduction is possible. Restart with increased mu
//		*                                 6 - stopped by small ||e||_2
//		*                                 7 - stopped by invalid (i.e. NaN or Inf) "func" values; a user error
//		* info[7]= # function evaluations
//		* info[8]= # Jacobian evaluations
//		* info[9]= # linear systems solved, i.e. # attempts for reducing error
//		*/
//	printf("Levenberg-Marquardt returned in %g iter, reason %g, sumsq %g [%g]\n", info[5], info[6], info[1], info[0]);
//	/*printf("\n\nMinimization info:\n");
//	for(int i=0; i<LM_INFO_SZ; ++i)
//		printf("%g ", info[i]);
//	printf("\n");
//	cout << "p:" << pnum << endl;
//	cout << "x:" << xnum << endl;*/
//	getResult();
//}
//
//void RotoScoping::initilizeX(double *x,  int xnum )
//{
//	for (int i = 0; i < xnum; ++i)
//	{
//		x[i] = 0;
//	}
//}
//
//void RotoScoping::getResult()
//{
//	int pindex = 0; // 每帧的position数据偏移
//
//	curveDataManager->getCurveData(taIndex).updateDirect();
//	curveDataManager->getCurveData(tbIndex).updateDirect();
//
//	for (int i = taIndex + 1; i < tbIndex; ++i)
//	{
//		double a = (double)(i - taIndex) / (tbIndex - taIndex);
//		double b = (double)(tbIndex - i) / (tbIndex - taIndex);
//
//		//cout << "ab" << a << b << endl;
//		
//
//		int temp = 0;
//
//		for (int j = 0; j < curveDataManager->getCurveData(taIndex).getNumOfData(); ++j)
//		{
//			
//			if ( j != 0)
//			{
//				temp = 2 * curveDataManager->getCurveData(taIndex).index[j-1];
//			}
//			//pindex = curveDataManager->getCurveData(taIndex).index[j];
//			ON_2dPoint p, tp;
//			
//			p.x = position[pindex + temp];
//			p.y = position[pindex + temp + 1];
//			
//			/*if (p.x != curveDataManager->getCurveData(i).getPoi(j).x || p.y != curveDataManager->getCurveData(i).getPoi(j).y)
//			{
//				cout << "poi: " << i << " " << j << endl;
//			}*/
//			curveDataManager->getCurveData(i).updatePoi(j, p);
//			
//			//因为是闭合曲线，所以最后一个点和第一个点为同一个点
//			//求解当前点的切线斜率
//			int index1 = pindex + ((temp + 2) % (2 * poiNumOfPerFrame));
//			/*if (temp + 2 > 2 * poiNumOfPerFrame)
//			{
//				index1 = pindex;
//			}
//			else
//			{
//				index1 = pindex + temp + 2;
//			}*/
//
//
//			//后一个Sample点
//			
//			p.x = position[index1];
//			p.y = position[index1+1];
//			//前一个Sample点
//			int index2 = pindex + ((temp - 2 + 2 * poiNumOfPerFrame) % (2 * poiNumOfPerFrame));
//			/*if (temp - 2 < 0)
//			{
//				index2 = pindex + 2 * poiNumOfPerFrame - 2;
//			}
//			else
//			{
//				index2 = pindex + temp - 2;
//			}*/
//			
//			tp.x = position[index2];
//			tp.y = position[index2+1];
//			
//			ON_2dPoint direc = curveDataManager->getCurveData(i).computD(p, tp);
//
//			//计算切线终点位置,确定在图像内部
//			p = curveDataManager->getCurveData(i).getPoi(j);
//			double length = sqrt(distance2(curveDataManager->getCurveData(taIndex).getDp(j), curveDataManager->getCurveData(taIndex).getPoi(j))) * a
//				          + sqrt(distance2(curveDataManager->getCurveData(tbIndex).getDp(j), curveDataManager->getCurveData(tbIndex).getPoi(j))) * b;
//			
//			double x = p.x + length * direc.x;
//			double y = p.y + length * direc.y;
//			
//			if (!isInImage(y, x))
//			{
//				double k = direc.y / (direc.x + 1e-6) + 1e-6;
//				if(x > imagewidth - 1)
//				{
//					x = imagewidth - 1;
//					y = k * (x - p.x) + p.y;
//				}
//				else if(x < 0)
//				{
//					x = 0;
//					y = k * (0 - p.x) + p.y;
//				}
//				if (y > imageheight - 1)
//				{
//					y = imageheight;
//					x = (y - p.y) / k + p.x;
//				}
//				else if (y < 0)
//				{
//					y = 0;
//					x = (y - p.y) / k + p.x;
//				}
//			}
//			curveDataManager->getCurveData(i).updateDp(j, ON_2dPoint(x,y));
//		}
//		pindex += 2 * poiNumOfPerFrame;
//	}
//}
//
//void RotoScoping::simpleNurbsPropagate(int st, int en, int key)
//{
//	if (en > st)
//	{
//		for (int i = st; i <= en; ++i)
//		{
//			if ( i == key)
//			{
//				continue;
//			}
//			curveDataManager->getCurveData(i).copy(curveDataManager->getCurveData(key));
//		}
//		
//		
//	}
//	
//}
//
//void RotoScoping::jacfunc( double *p, double *jac, int m, int n, void *data )
//{
//
//	memset(jac, 0, sizeof(double) * xnum * pnum);
//
//	int jacindex;
//
//
//	//eL
//	for (int i = 0; i < poiNumOfPerFrame; ++i)
//	{
//		
//	}
//	
//
//}
//
//void RotoScoping::showImage(CxImage& c, int index)
//{
//	//c.Save("a.jpg",CXIMAGE_FORMAT_JPG);
//	img[index] = cvCreateImage(cvSize(c.GetWidth(), c.GetHeight()), IPL_DEPTH_8U, 3);
//	int w = c.GetWidth();
//	int h = c.GetHeight();
//	for (int i = 0; i < img[index]->height; ++i)
//	{
//		for (int j = 0; j < img[index]->width; ++j)
//		{
//			int ti = img[index]->height-1-i;
//			//cout  << "aaa" << endl;
//			CV_IMAGE_ELEM(img[index], uchar, i, j * img[index]->nChannels) = c.GetPixelColor(j, ti).rgbBlue;
//			CV_IMAGE_ELEM(img[index], uchar, i, j * img[index]->nChannels + 1) = c.GetPixelColor(j, ti).rgbGreen;
//			CV_IMAGE_ELEM(img[index], uchar, i, j * img[index]->nChannels + 2) = c.GetPixelColor(j, ti).rgbRed;
//			//cout << "__" << j << endl;
//		}
//		//cout << i << " " << endl;
//	}
//	/*cvNamedWindow("aa");
//	cvShowImage("aa", img[index]);
//	cvWaitKey(0);*/
//}
//
//void RotoScoping::computeK( int ta )
//{
//	int maxg = 0;
//	for (int i = 0; i < imageheight; ++i)
//	{
//		for (int j = 0; j < imagewidth; ++j)
//		{
//			int temp = gradient(imageSequnce.getImageBuffer(ta)->GetImage(), i, j);
//			if (temp > maxg)
//			{
//				maxg = temp;
//			}
//		}
//	}
//
//	K = maxg + 1;
//}
//
//
