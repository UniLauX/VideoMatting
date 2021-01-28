#include <stdio.h>
#include <stdlib.h>
/*
* 使用已经求出的x，向前计算x（供getx()调用）
* float a[][]    系数矩阵
* float x[]        方程组解
* int    i        解的序号
* int    n        矩阵大小
* return        公式中需要的和
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
* 解方程组，计算x
* float a[][]    系数矩阵
* float b[]        右端项    
* float x[]        方程组解
* int    i        解的序号
* int    n        矩阵大小
* return        方程组的第i个解
*/
double getx(double ** a, double * b, double * x, int i, int n)
{
	double result;
	if(i==n-1) //计算最后一个x的值
		result = double(b[n-1]/a[n-1][n-1]);
	else //计算其他x值（对于公式中的求和部分，需要调用getm()函数）
		result = double((b[i]-getm(a,x,i,n))/a[i][i]);

	return result;
}


void solve_Gauss(double ** a, double * b, double *x, int n)
{
	int i,j,k;

	/*进行高斯消去*/

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

	

	/*回代方式解方程组*/
	for(i=n-1; i>=0; i--)
	{
		x[i] = getx(a,b,x,i,n);
	}

	
}
