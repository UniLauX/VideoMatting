#include "OpticalFlowFileManager.h"
#include "ZImage.h"
OpticalFlowFileManager::OpticalFlowFileManager()
{

}
OpticalFlowFileManager::OpticalFlowFileManager(std::string _path, int _width, int _height)
{
	Init(_path, _width, _height);
}
void OpticalFlowFileManager::Init(std::string _path, int _width, int _height)
{
	path = _path;
	width = _width;
	height = _height;

	forward.u = 0;
	forward.v = 0;

	backward.u = 0;
	backward.v = 0;
	currentFrame = -1;
}

bool OpticalFlowFileManager::LoadFlow(int i)
{
	if (currentFrame != i)
	{
		bool re1=false,re2=false, re3=false,re4=false;
		std::string fup;
		std::string fvp;
		std::string bup;
		std::string bvp;
		char frame[10]; _itoa(i,frame,10);
		char framenext[10];  _itoa(i+1,framenext, 10);
		char frameprev[10]; _itoa(i-1,frameprev, 10);
		if (path.find('\\')!= std::string::npos)//find '\'
		{
			fup=  path+"\\"+"u_"+std::string(frame) + "_"+std::string(framenext)+".raw";
			fvp=  path +"\\"+"v_"+std::string(frame) + "_"+std::string(framenext)+".raw";
			bup= path+"\\"+"u_"+std::string(frame)+"_"+std::string(frameprev)+".raw";
			bvp= path+"\\"+"v_"+std::string(frame)+"_"+std::string(frameprev)+".raw";
		}
		else 
		{
			fup=  path+"/"+"u_"+std::string(frame) + "_"+std::string(framenext)+".raw";
			fvp=  path +"/"+"v_"+std::string(frame) + "_"+std::string(framenext)+".raw";
			bup= path+"/"+"u_"+std::string(frame)+"_"+std::string(frameprev)+".raw";
			bvp= path+"/"+"v_"+std::string(frame)+"_"+std::string(frameprev)+".raw";
		}

		FILE* pfile = NULL;
		pfile = fopen(fup.c_str(), "rb");
		if (pfile)
		{
			if(forward.u==0)
			{
				forward.u = new float[width*height];
				memset(forward.u, 0, sizeof(float)*width*height);
			}
			fread(forward.u, sizeof(float), width*height, pfile);
			fclose(pfile);
			re1 = true;
		}
		pfile = NULL;
		pfile = fopen(fvp.c_str(), "rb");
		if (pfile)
		{
			if(forward.v ==0)
			{
				forward.v = new float[width*height];
				memset(forward.v, 0, sizeof(float)*width*height);
			}
			fread(forward.v, sizeof(float), width*height, pfile);
			fclose(pfile);
			re2 = true;
		}

		pfile = NULL;
		pfile = fopen(bup.c_str(), "rb");
		if (pfile)
		{
			if (backward.u==0)
			{
				backward.u = new float[width*height];
				memset(backward.u, 0, sizeof(float)*width*height);
			}
			
			fread(backward.u, sizeof(float), width*height, pfile);
			fclose(pfile);
			re3 = true;
		}

		pfile = NULL;
		pfile = fopen(bvp.c_str(), "rb");
		if (pfile)
		{
			if (backward.v==0)
			{
				backward.v = new float[width*height];
				memset(backward.v, 0, sizeof(float)*width*height);
			}
			fread(backward.v, sizeof(float), width*height, pfile);
			fclose(pfile);
			re4 = true;
		}
			currentFrame = i;
		return(re1&&re2)||(re3&&re4);
	}
	else
		return true;
}

bool OpticalFlowFileManager::LoadFlowFromZGF(int i)
{
	if (currentFrame!=i)
	{
		char num[10];
		_itoa(i, num, 10);
		std::string  filepath;
		if (path.find('\\')!= std::string::npos)//find '\'
		{
			filepath=  path+"\\"+"_flow"+std::string(num)+".raw";
		}
		else 
		{
			filepath=  path+"/"+"_flow"+std::string(num) +".raw";
		}
		ZFloatImage flowimage;
		printf("flow file name: %s\n", filepath.c_str());
		flowimage.Create(width, height, 4);
		FILE* fp;
		fp = fopen(filepath.c_str(), "rb");
		if (fp)
		{
			fread(flowimage.GetMap(), sizeof(char), flowimage.GetSize(), fp);
			fclose(fp);

			if (forward.u==0)
			{
				forward.u = new float[width*height];
				memset(forward.u, 0, sizeof(float)*width*height);
				forward.v = new float[width*height];
				memset(forward.v, 0, sizeof(float)*width*height);
				backward.u = new float[width*height];
				memset(backward.u, 0, sizeof(float)*width*height);
				backward.v = new float[width*height];
				memset(backward.v, 0, sizeof(float)*width*height);
			}
			for (int i=0; i<width; ++i)
			{
				for (int j=0; j<height; ++j)
				{
					forward.u[j*width+i] = flowimage.at(i,j,0);
					forward.v[j*width+i] = flowimage.at(i,j,1);
					backward.u[j*width+i] = flowimage.at(i,j,2);
					backward.v[j*width+i] = flowimage.at(i,j,3);
				}
			}
			currentFrame = i;
			return true;
		}		
		else
			return false;
	}
}

Flowdata* OpticalFlowFileManager::Forwardflow(int i)
{
	if (currentFrame!=i)
		if (!LoadFlow(i))
		//if(!LoadFlowFromZGF(i))
		{
			printf("optical flow file does not exists\n");
			delete[] forward.u;
			delete[] forward.v;
			forward.u = 0;
			forward.v = 0;
		}
	return &forward;
}

Flowdata* OpticalFlowFileManager::Backwardflow(int i)
{
	if (currentFrame!=i)
		if (!LoadFlow(i))
		//if(!LoadFlowFromZGF(i))
		{
			printf("optical flow file does not exists\n");
			delete[] backward.u;
			delete[] backward.v;
			backward.u = 0;
			backward.v = 0;
		}
	return &backward;
}
OpticalFlowFileManager::~OpticalFlowFileManager(void)
{
	Clear();
}
void OpticalFlowFileManager::Clear()
{
	if (forward.u!=0)
		delete[] forward.u;
	if (forward.v!=0)
		delete[] forward.v;
	if(backward.u!=0)
		delete[] backward.u;
	if(backward.v!=0)
		delete backward.v;

	forward.u=0;
	forward.v=0;
	backward.u=0;
	backward.v=0;
	currentFrame=-1;
}
