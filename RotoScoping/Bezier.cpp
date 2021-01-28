#include "../RotoScoping/stdafx.h"
#include "../RotoScoping/Bezier.h"

Bezier * Bezier::GetInstance()
{
	static Bezier BezierSingleton;
	return &BezierSingleton;
}

void Bezier::GenerateCurve(CvPoint2D32f p0, CvPoint2D32f p1, CvPoint2D32f p2, CvPoint2D32f p3,int NumOfIntermediatePoint,CvPoint2D32f * Output){
	//  Calculate  line1( determined by x0 and x1 )  and  line2 (determined by x2 and x3)
	/*	����һ��cubic����������
		��ʽΪ��
		(1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3
		*/


	double step = 1.0/(NumOfIntermediatePoint+1);
	double t=step;	//  ֻ���м�ĵ�
	for (int i=0;i<NumOfIntermediatePoint;i++,t+=step)
	{

		double coeff0 = (1-t)*(1-t)*(1-t);
		double coeff1 = 3*(1-t)*(1-t)*t;
		double coeff2 = 3*(1-t)*t*t;
		double coeff3 = t*t*t;

		Output[i].x = coeff0 * p0.x + coeff1 * p1.x + coeff2*p2.x + coeff3*p3.x;
		Output[i].y = coeff0 * p0.y + coeff1 * p1.y + coeff2*p2.y + coeff3*p3.y;
	}


}



/*

@	CurvePoints�����������ϵĵ�
@	normals�����������ϵĵ��Ӧ�ĵ�λ������

	����һ�������������ϵ����е�������Լ���������������ͷ�ĵ�����һ���㡣
	��ͷ�ĵ��indexΪ0�����һ��������ΪNumOfIntermediatePoint + 1
	��������м����N������ôCurvePoints��Normals�ĳ��ȶ�Ӧ��ΪN+2����

	��cubic����������

	��ʽΪ��
	(1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3

	dx/dt = p1*(t - 1)^2 - p0*(t - 1)^2 - p2*t^2 + p3*t^2 + p1*t*(2*t - 2) - 2*p2*t*(t - 1)
	dy/dt = p1*(t - 1)^2 - p0*(t - 1)^2 - p2*t^2 + p3*t^2 + p1*t*(2*t - 2) - 2*p2*t*(t - 1)
*/
void Bezier::GenerateCurveAndNormal(CvPoint2D32f p0, CvPoint2D32f p1, CvPoint2D32f p2, CvPoint2D32f p3,int NumOfIntermediatePoint,CvPoint2D32f * PointsOnCurve,Normal * normals){

	double step = 1.0/(NumOfIntermediatePoint+1);
	//��һ���������Ϊ(t=0)
	PointsOnCurve[0] = p0;
	double dxdt = p1.x - p0.x;
	double dydt = p1.y - p0.y;
	double norm = sqrt(dxdt * dxdt + dydt * dydt);
	//����Ϊ
	normals[0].NormalX = -dydt / norm;
	normals[0].NormalY = dxdt / norm;
	//printf("dxdt=%f,dydt=%f,normals[0].NormalX = %f, normals[0].NormalY =%f, norm=%f\n",dxdt,dydt,normals[0].NormalX ,normals[0].NormalY, norm);


	double t=step;	//  �м�ĵ�
	for (int i=1;i<=NumOfIntermediatePoint;i++,t+=step)
	{
		double tcubic = t*t*t;
		double tsquare = t*t;
		double t_minus_one_square = ( t - 1 ) * ( t - 1 );
		double coeff0 = t_minus_one_square*(1-t);
		double coeff1 = 3*t_minus_one_square*t;
		double coeff2 = 3*(1-t)*tsquare;


		PointsOnCurve[i].x = coeff0 * p0.x + coeff1 * p1.x + coeff2*p2.x + tcubic*p3.x;
		PointsOnCurve[i].y = coeff0 * p0.y + coeff1 * p1.y + coeff2*p2.y + tcubic*p3.y;
		
		
		dxdt = p1.x * t_minus_one_square - p0.x * t_minus_one_square- p2.x * tsquare + p3.x * tsquare + 2 * p1.x * t*(t - 1) - 2 * p2.x * t * (t - 1);
		dydt = p1.y * t_minus_one_square - p0.y * t_minus_one_square- p2.y * tsquare + p3.y * tsquare + 2 * p1.y * t*(t - 1) - 2 * p2.y * t * (t - 1);
		norm = sqrt(dxdt * dxdt + dydt * dydt);

		normals[i].NormalX = -dydt/norm;
		normals[i].NormalY = dxdt/norm;
		//printf("normals[i].NormalX = %f, normals[i].NormalY =%f, norm=%f\n",normals[i].NormalX ,normals[i].NormalY, norm);
	}


	//���һ���������Ϊ(t=1)
	PointsOnCurve[NumOfIntermediatePoint+1] = p3;
	dxdt = p3.x - p2.x;
	dydt = p3.y - p2.y;
	norm = sqrt(dxdt * dxdt + dydt * dydt);
	
	//����Ϊ
	normals[NumOfIntermediatePoint+1].NormalX = -dydt / norm;
	normals[NumOfIntermediatePoint+1].NormalY = dxdt / norm;

	//printf("normals[i].NormalX = %f, normals[i].NormalY =%f, norm=%f\n",normals[NumOfIntermediatePoint+1].NormalX ,normals[NumOfIntermediatePoint+1].NormalY, norm);
}