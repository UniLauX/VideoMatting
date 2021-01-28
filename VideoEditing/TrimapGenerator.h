#pragma once
#include <string>
#include <cximage.h>
#include "ImageBuffer.h"
class TrimapGenerator
{
public:
	TrimapGenerator(std::string dirName, std::string layername, std::string exttype, int start, int unknownDis)
		:m_DirName(dirName),
		m_LayerName(layername),
		m_ExtType(exttype),
		m_iStart(start),
		//m_iEnd(end),
		m_iUnknownDis(unknownDis){}
	~TrimapGenerator(void);
	void GeneratorTrimap(ImageBuffer* trimapBuffer, ImageBuffer* alphaBuffer);

private:
	std::string m_DirName;
	std::string m_LayerName;
	std::string m_ExtType;
	int m_iStart;
	//int m_iEnd;
	int m_iUnknownDis;
};
