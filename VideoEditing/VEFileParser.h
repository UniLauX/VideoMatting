#pragma once
#include "FileParser.h"
#include "VEProject.h"
class CVEFileParser:public CFileParser
{
public:
	CVEFileParser(CVEProject* veprj);
	~CVEFileParser(void);

	bool LoadProject(const char* prjFile, bool bLoadLayerDescripter=true);
	bool SaveProject(const char* prjFile);

	void LocateContext(std::string str,int & start,int& end);
	void LocateContext(std::string str,int rangeBegin,int rangeEnd,int& start,int& end);

	void ParseSequence();
	void ParseLayers();
	void ParseProjectInfo();
	void ParsePolygons();

private:
	CVEProject* m_veprj;
	


};
