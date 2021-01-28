#include "Grabcut.h"
#include <vector>

//ofstream out("E:\\pro.txt");

void Grabcut::LocalCalculate(vector<POINT> pixels, CxImage &scrible)
{

	NWeight **weight=new NWeight *[originalImage.GetWidth()];
	Graph::node_id **localGraphNode=new Graph::node_id *[originalImage.GetWidth()];
	for (int i=0;i<originalImage.GetWidth();++i)
	{
		weight[i]=new NWeight [originalImage.GetHeight()];
		localGraphNode[i]=new Graph::node_id[originalImage.GetHeight()];
		memset(localGraphNode[i],0,sizeof(Graph::node_id)*originalImage.GetHeight());
	}

	for (int i=0;i<originalImage.GetWidth();++i)
	{
		for (int j=0;j<originalImage.GetHeight();++j)
		{
			weight[i][j].up=0;
			weight[i][j].down=0;
			weight[i][j].left=0;
			weight[i][j].right=0;
		}
	}
	//init Local GMM
	int size=(int)pixels.size();



	initLocalGMM(pixels);


	//s=clock();
	//calLocalNWeight(pixels,weight);
	//e=clock();
	//d=(e-s)/CLK_TCK;
	//cout<<"calLoaclNTime: "<<d<<endl;

	constructLocalGraph(pixels,scrible,weight,localGraphNode);

	if (graph)
	{
		graph->maxflow();
	}

	POINT ptemp;
	int x,y;
	RGBQUAD rgbs;
	int index;
	for (int i=0;i<size;++i)
	{
		ptemp=pixels[i];
		x=ptemp.x;//-cropRect.left;
		y=ptemp.y;//-cropRect.bottom;
		rgbs=scrible.GetPixelColor(ptemp.x,ptemp.y);

		index=ptemp.y*originalImage.GetWidth()+ptemp.x;

		if (graph->what_segment(localGraphNode[x][y])==Graph::SOURCE)
			label[index]=1;

		else
			label[index]=0;

	}

	checkConnectivity(label,originalImage.GetWidth(),originalImage.GetHeight());

	for (int i=0;i<originalImage.GetWidth();++i)
	{
		delete[] weight[i];
		weight[i]=NULL;

		delete[] localGraphNode[i];
		localGraphNode[i]=NULL;
	}
	delete[] weight;
	weight=NULL;

	delete[] localGraphNode;
	localGraphNode=NULL;
}

void Grabcut::initLocalGMM(vector<POINT>& pixels)
{
	int size=(int)pixels.size();
	vector<colorNode> fortempvec;
	vector<colorNode> backtempvec;

	POINT ptemp;
	int x,y;
	int index;
	for (int i=0;i<size;++i)
	{
		ptemp=pixels[i];
		x=ptemp.x;
		y=ptemp.y;

		index=ptemp.y*originalImage.GetWidth()+ptemp.x;

		colorNode temp;
		RGBQUAD rgb=originalImage.GetPixelColor(x,y);
		temp.indexX=x;
		temp.indexY=y;
		temp.colorinfo.resize(3,1);
		temp.colorinfo(0,0)=(double)rgb.rgbRed;
		temp.colorinfo(1,0)=(double)rgb.rgbGreen;
		temp.colorinfo(2,0)=(double)rgb.rgbBlue;
		if (label[index])
		{
			fortempvec.push_back(temp);
		}
		else if (!label[index])
		{
			backtempvec.push_back(temp);
		}
	}
	vector< vector<colorNode> > forvec;
	vector< vector<colorNode> > backvec;

	forvec.push_back(fortempvec);
	backvec.push_back(backtempvec);

	cluster(forvec);
	cluster(backvec);

	initGMM(forvec,fgmm);
	initGMM(backvec,bgmm);

}

void Grabcut::constructLocalGraph(std::vector<POINT> &pixels, CxImage &scribble,NWeight **weight,Graph::node_id **localGNode)
{

	if(graph)
		delete graph;
	graph=new Graph();

	int w,h;
	w=originalImage.GetWidth();
	h=originalImage.GetHeight();
	int size=(int)pixels.size();
	POINT ptemp;
	int x,y;
	for (int i=0;i<size;++i)
	{
		x=pixels[i].x;
		y=pixels[i].y;
		if (localGNode[x][y]==NULL)
		{
			localGNode[x][y]=graph->add_node();
		}

		if (y+1<h)
		{
			if (localGNode[x][y+1]==NULL)
				localGNode[x][y+1]=graph->add_node();
		}
		if (y-1>=0)
		{
			if(localGNode[x][y-1]==NULL)
				localGNode[x][y-1]=graph->add_node();
		}
		if (x+1<w)
		{
			if(localGNode[x+1][y]==NULL)
				localGNode[x+1][y]=graph->add_node();
		}
		if(x-1>=0)
		{
			if(localGNode[x-1][y]==NULL)
				localGNode[x-1][y]=graph->add_node();
		}




	}


	RGBQUAD rgb1,rgb2;
	double backposs,forposs;
	for (int i=0;i<size;++i)
	{
		ptemp=pixels[i];
		x=ptemp.x;
		y=ptemp.y;

		rgb1=scribble.GetPixelColor(ptemp.x,ptemp.y);
		if (rgb1.rgbRed==255)
			graph->add_tweights(localGNode[x][y],4501,0);
		else if (rgb1.rgbRed==0)
			graph->add_tweights(localGNode[x][y],0,4501);
		else
		{
			rgb2=originalImage.GetPixelColor(x,y);
			backposs=0;
			forposs=0;

			for (int itemp=0;itemp<5;++itemp)
			{
				if(bgmm[itemp].count)
				{
					backposs+=bgmm[itemp].pi*calposs(0,itemp,rgb2);
				}
			}

			for (int itemp=0;itemp<5;++itemp)
			{
				if (fgmm[itemp].count)
					forposs+=fgmm[itemp].pi*calposs(1,itemp,rgb2);
			}


			//if(backposs==0)
			//	backposs=1e-30;
			//if(forposs==0)
			//	forposs=1e-30;
			backposs=log(backposs);
			forposs=log(forposs);

			//out<<"forposs: "<<forposs<<endl;
			//out<<"backposs: "<<backposs<<endl;
			graph->add_tweights(localGNode[x][y],forposs,backposs);


		}

	}


	double distanceE,discol;
	double weightD;
	for (int i=0;i<size;++i)
	{
		ptemp=pixels[i];
		x=ptemp.x;
		y=ptemp.y;

		if(y+1<h)
		{
			distanceE=distance(x,y,x,y+1);
			discol=coldistance(originalImage.GetPixelColor(x,y),originalImage.GetPixelColor(x,y+1));
			weightD=calN(distanceE,discol);
		graph->add_edge(localGNode[x][y],localGNode[x][y+1],weightD*0.3,weightD*0.3);
		}
		if(y-1>=0)
		{
			distanceE=distance(x,y,x,y-1);
			discol=coldistance(originalImage.GetPixelColor(x,y),originalImage.GetPixelColor(x,y-1));
			weightD=calN(distanceE,discol);
			graph->add_edge(localGNode[x][y],localGNode[x][y-1],weightD*0.3,weightD*0.3);
		}
		if(x-1>=0)
		{
			distanceE=distance(x,y,x-1,y);
			discol=coldistance(originalImage.GetPixelColor(x,y),originalImage.GetPixelColor(x-1,y));
			weightD=calN(distanceE,discol);
			graph->add_edge(localGNode[x][y],localGNode[x-1][y],weightD*0.3,weightD*0.3);
		}
		if(x+1<w)
		{
			distanceE=distance(x,y,x+1,y);
			discol=coldistance(originalImage.GetPixelColor(x,y),originalImage.GetPixelColor(x+1,y));
			weightD=calN(distanceE,discol);
			graph->add_edge(localGNode[x][y],localGNode[x+1][y],weightD*0.3,weightD*0.3);
		}

	}

}

void Grabcut::calLocalNWeight(vector<POINT> &pixels,Nweight ** weight)
{
	POINT ptemp;
	double distanceE,discol;
	int imax,jmax;
	imax=originalImage.GetWidth();
	jmax=originalImage.GetHeight();

	RGBQUAD r1;
	for (int x=0;x<originalImage.GetWidth();++x)
	{
		for (int y=0;y<originalImage.GetHeight();++y)
		{
			r1=originalImage.GetPixelColor(x,y);
			if (y+1<jmax)
			{
				distanceE=distance(x,y,x,y+1);
				discol=coldistance(r1,originalImage.GetPixelColor(x,y+1));
				weight[x][y].up=calN(distanceE,discol);
			}
			if (y-1>=0)
			{
				distanceE=distance(x,y,x,y-1);
				discol=coldistance(r1,originalImage.GetPixelColor(x,y-1));
				weight[x][y].down=calN(distanceE,discol);
			}
			if (x-1>=0)
			{
				distanceE=distance(x,y,x-1,y);
				discol=coldistance(r1,originalImage.GetPixelColor(x-1,y));
				weight[x][y].left=calN(distanceE,discol);
			}

			if (x+1<imax)
			{
				distanceE=distance(x,y,x+1,y);
				discol=coldistance(r1,originalImage.GetPixelColor(x+1,y));
				weight[x][y].right=calN(distanceE,discol);
			}
		}
	}
}
