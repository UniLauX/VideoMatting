#ifndef VIDEO_H
#define VIDEO_H

#include <string>
#include <vector>
#include "../include/CxImage/ximage.h"

using std::string;
using std::vector;

typedef struct Seq_t
{

	string  name;
	int     start;
	int     end;
	int optStart;
	int optEnd;
	int     width,height;
	string fileTitle;// 

	Seq_t(){};
	Seq_t(string name, int start, int end, int _optStart, int _optEnd);
	~Seq_t(){};

	void SetInfo(string name, int start, int end, int _optStart, int _optEnd);
	bool LoadImage(int num, CxImage &image);
	void GetFileTitle(string _name);

}SEQ;
#endif