#include "../VideoEditing/stdafx.h"
#include "cluster.h"
#include <iostream>
using namespace std;
using techsoft::mslice;
dMatrix calM(const vector<colorNode> &vec)
{
	dMatrix m(3,1,0.0);
	for(size_t i=0;i<vec.size();i++)
	{
		m=m+vec[i].colorinfo;
	}
	return m;
}
int calN(const std::vector<colorNode> &vec)
{
	return (int)vec.size();
}
dMatrix calR(const std::vector<colorNode> &vec)
{
	dMatrix r(3,3,0.0);
	for(size_t i=0;i<vec.size();i++)
	{
		r=r+(vec[i].colorinfo*(~vec[i].colorinfo));
	}
	return r;
}
dMatrix calRC(const dMatrix r, const dMatrix m, const int n)
{
	dMatrix mm=m*(~m);
	mm/=(double)n;
	return r-mm;
}
void cluster(vector<vector<colorNode >> & vec,int CLUSTERTIME)
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

		double temp1;
		double temp2;

		for (size_t t=0;t<vec[maxindex].size();++t)
		{

			temp1=eigenvector(0,0)*vec[maxindex][t].colorinfo(0,0)+eigenvector(1,0)*vec[maxindex][t].colorinfo(1,0)+eigenvector(2,0)*vec[maxindex][t].colorinfo(2,0);
			temp2=eigenvector(0,0)*q(0,0)+eigenvector(1,0)*q(1,0)+eigenvector(2,0)*q(2,0);

			if(temp1<=temp2)
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
