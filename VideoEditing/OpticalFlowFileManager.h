#pragma once

#include <string>

typedef struct node
{
	float* u;
	float* v;
}Flowdata;
class OpticalFlowFileManager
{
public:
	OpticalFlowFileManager();
	OpticalFlowFileManager(std::string _path, int _width, int _height);
	void Init(std::string _path, int _width, int _height);
	bool LoadFlow(int i);//load optical flow of frame i
	bool LoadFlowFromZGF(int i);
	Flowdata* Forwardflow(int i);//return optical flow of i to i+1
	Flowdata* Backwardflow(int i);//return optical flow of i to i-1
	~OpticalFlowFileManager(void);
	void Clear();
private:
	Flowdata  forward;
	Flowdata backward;
	std::string path;
	int width;
	int height;
	int currentFrame;
};
