#include <stdio.h>
#include <stdlib.h>
/*
* ʹ���Ѿ������x����ǰ����x����getx()���ã�
* float a[][]    ϵ������
* float x[]        �������
* int    i        ������
* int    n        �����С
* return        ��ʽ����Ҫ�ĺ�
*/
double getm(double ** a, double * x, int i, int n)
{
	double m = 0;
	int r;
	for(r=i+1; r<n; r++)
	{
		m += a[i][r] * x[r];
	}
	return m;
}

/*
* �ⷽ���飬����x
* float a[][]    ϵ������
* float b[]        �Ҷ���    
* float x[]        �������
* int    i        ������
* int    n        �����С
* return        ������ĵ�i����
*/
double getx(double ** a, double * b, double * x, int i, int n)
{
	double result;
	if(i==n-1) //�������һ��x��ֵ
		result = double(b[n-1]/a[n-1][n-1]);
	else //��������xֵ�����ڹ�ʽ�е���Ͳ��֣���Ҫ����getm()������
		result = double((b[i]-getm(a,x,i,n))/a[i][i]);

	return result;
}


void solve_Gauss(double ** a, double * b, double *x, int n)
{
	int i,j,k;

	/*���и�˹��ȥ*/

	for(i=0; i<n-1; i++)
	{
		for(j=i+1; j<n; j++)
		{
			a[i][j]=(float)(a[i][j]/a[i][i]);
		}
		b[i]=(float)(b[i]/a[i][i]);
		a[i][i]=1;
		for(k=i+1;k<n;k++)
		{
			for(j=i+1;j<n;j++)
			{
				a[k][j]=(float)(a[k][j]-a[k][i]*a[i][j]);
			}
			b[k]=(float)(b[k]-a[k][i]*b[i]);
		}
		for(j=i+1;j<n;j++)
		{
			a[j][i]=0;
		}
	}

	

	/*�ش���ʽ�ⷽ����*/
	for(i=n-1; i>=0; i--)
	{
		x[i] = getx(a,b,x,i,n);
	}

	
}
