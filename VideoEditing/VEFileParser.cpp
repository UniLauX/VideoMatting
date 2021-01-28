#include "StdAfx.h"
#include "VEFileParser.h"
CVEFileParser::CVEFileParser(CVEProject* veprj)
{
	this->m_veprj=veprj;
}

CVEFileParser::~CVEFileParser(void)
{
}

bool CVEFileParser::LoadProject(const char* prjFile, bool bLoadLayerDescripter/* =true */)
{

	if(!ReadFile(prjFile))
		return FALSE;

	//m_veprj->m_strProject=prjFile;

	//SetCurrentDirectory(GetFileDir(prjFile));

	m_IndexBuffer = 0;
	CString strImagePath;
	//CMainFrame*	pMainFrame = (CMainFrame*)AfxGetMainWnd();

	ReadLine();
	if(strncmp(m_pBufferLine,"#video edit project file",strlen("#video edit project file"))){
		AfxMessageBox("This is not video editing Project File!");
		return 0;
	}

	ParseSequence();
	ParseProjectInfo();
	ParseLayers();
	ParsePolygons();
	return TRUE;

}

void CVEFileParser::LocateContext(string str,int& start,int& end)
{

	CString keyString1,keyString2;
	keyString1 = CString("<") + CString(str.c_str()) + CString(">");
	keyString2 = CString("</") + CString(str.c_str()) + CString(">");

	if(OffsetToString(keyString1)){
		start = m_IndexBuffer + keyString1.GetLength();
	}
	if(OffsetToString(keyString2)){
		end = m_IndexBuffer - 1 ;
	}

}

void CVEFileParser::LocateContext(std::string str,int rangeBegin,int rangeEnd,int& start,int& end)
{
	CString keyString1,keyString2;
	keyString1 = CString("<") + CString(str.c_str()) + CString(">");
	keyString2 = CString("</") + CString(str.c_str()) + CString(">");

	if(OffsetToString(keyString1,rangeBegin,rangeEnd)){
		start = m_IndexBuffer + keyString1.GetLength();
	}
	if(OffsetToString(keyString2,rangeBegin,rangeEnd)){
		end = m_IndexBuffer - 1 ;
	}
}

void CVEFileParser::ParseSequence()//得到图片序列的路径 起始帧和结束帧
{
	int start,end;
	int index;

	LocateContext("Image Sequence",start,end);
	if (OffsetToString("Sequence",start,end))
	{
		index=m_IndexBuffer+strlen("Sequence:");
		strcpy(m_pBufferWord,m_pBufferLine+strlen("Sequence:"));
		CString tmpString=m_pBufferWord;
		tmpString.TrimLeft();
		tmpString.TrimRight();
		TCHAR FullPath[MAX_PATH];
		TCHAR PrjDir[MAX_PATH];
		GetFullPathName(tmpString,MAX_PATH,FullPath,(TCHAR**)&PrjDir);

		CString str;
		str.Format("%s",FullPath);
		m_veprj->m_sequencePath=str;
	}

	if (OffsetToString("start:",start,end))
	{
		index=m_IndexBuffer+strlen("start:");
		strcpy(m_pBufferWord,m_pBufferLine+strlen("start:"));
		m_veprj->SetStartFrame(atoi(m_pBufferWord));
	}

	if (OffsetToString("end:",start,end))
	{
		index=m_IndexBuffer+strlen("end:");
		strcpy(m_pBufferWord,m_pBufferLine+strlen("end:"));
		m_veprj->SetEndFrame(atoi(m_pBufferWord));
	}


}

void CVEFileParser::ParseProjectInfo()
{
	int start=0,end=0;
	int index;

	LocateContext("Project Info",start,end);
	if (OffsetToString("LayerNum:",start,end))
	{
		index=m_IndexBuffer+strlen("LayerNum:");
		strcpy(m_pBufferWord,m_pBufferLine+strlen("LayerNum:"));
		m_veprj->layerNum=atoi(m_pBufferWord);
	}
	if(OffsetToString("DspMin:", start, end))
	{
		index = m_IndexBuffer + strlen("DspMin:");
		strcpy(m_pBufferWord, m_pBufferLine + strlen("DspMin:"));
		m_veprj->m_fDspMin = atof(m_pBufferWord);
	}
	if(OffsetToString("DspMax:", start, end))
	{
		index = m_IndexBuffer + strlen("DspMax:");
		strcpy(m_pBufferWord, m_pBufferLine + strlen("DspMax:"));
		m_veprj->m_fDspMax = atof(m_pBufferWord);
	}
}

void CVEFileParser::ParseLayers()
{
	int start,end;
	int index;
	m_IndexBuffer=0;
	for (int i=0;i<m_veprj->layerNum;++i)
	{
		LayerObject layer;
		LocateContext("Layer",start,end);

		m_IndexBuffer=start;
		if (OffsetToString("Name:"),start,end)//get layer name
		{
			index=m_IndexBuffer+strlen("Name:");
			strcpy(m_pBufferWord,m_pBufferLine+strlen("Name:"));

			CString str;
			str.Format("%s",m_pBufferWord);
			str.Delete(str.GetLength()-1);
			layer.SetName(str);
			printf("%s %d\n",str,str.GetLength());

		}
		if (OffsetToString("Type:",start,end))
		{
			index = m_IndexBuffer+strlen("Type:");
			strcpy(m_pBufferWord, m_pBufferLine+strlen("Type:"));
			layer.SetType(atoi(m_pBufferWord));
		}

		if (OffsetToString("StartIndex:",start,end))//get layer start index
		{
			index=m_IndexBuffer+strlen("StartIndex:");
			strcpy(m_pBufferWord,m_pBufferLine+strlen("StartIndex:"));

			layer.SetStartIndex(atoi(m_pBufferWord));
			layer.SetPos(atoi(m_pBufferWord));
		}

		int keyframenum;
		if (OffsetToString("KeyFrameNum:",start,end))
		{
			index=m_IndexBuffer+strlen("KeyFrameNum:");
			strcpy(m_pBufferWord,m_pBufferLine+strlen("KeyFrameNum:"));
			keyframenum=atoi(m_pBufferWord);
		}

		LocateContext("KeyFrame",start,end);
		m_IndexBuffer=start;
		for (int i=0;i<keyframenum;++i)
		{
			ReadWord();
			int frame=atoi(m_pBufferWord);
			layer.AddKeyFrame(frame);
		}

		LocateContext("PlanePoints",start,end);
		m_IndexBuffer=start;

		for (int i=0;i<keyframenum;++i)
		{
			int x1,x2,x3;
			int y1,y2,y3;
			int z1,z2,z3;

			ReadWord();
			x1=atoi(m_pBufferWord);
			ReadWord();
			y1=atoi(m_pBufferWord);
			ReadWord();
			z1=atoi(m_pBufferWord);

			ReadWord();
			x2=atoi(m_pBufferWord);
			ReadWord();
			y2=atoi(m_pBufferWord);
			ReadWord();
			z2=atoi(m_pBufferWord);

			ReadWord();
			x3=atoi(m_pBufferWord);
			ReadWord();
			y3=atoi(m_pBufferWord);
			z3=atoi(m_pBufferWord);

			Point_3D p1;
			p1.x=x1;
			p1.y=y1;
			p1.z=z1;

			Point_3D p2;
			p2.x=x2;
			p2.y=y2;
			p2.z=z2;

			Point_3D p3;
			p3.x=x3;
			p3.y=y3;
			p3.z=z3;

			layer.AddPlanePoint(p1,p2,p3);

		}

		m_veprj->vec_layer.push_back(layer);

	}
}


bool CVEFileParser::SaveProject(const char* prjFile)
{
	FILE* fp;
	int i,j,k;

	printf("Begin Save Project File\n");

	if (m_veprj->GetFrameCount()<=0)
	{
		AfxMessageBox("数据为空，无法保存");
		return false;
	}

	if ((fp=fopen(prjFile,"w"))==NULL)
	{
		AfxMessageBox("无法保存工程文件！");
		return false;
	}

	fprintf(fp,"#video edit project file\n");

	fprintf(fp,"<Image Sequence>\n");

	char relativePath[MAX_PATH]="";
	if (!PathRelativePathTo(relativePath,prjFile,FILE_ATTRIBUTE_NORMAL,m_veprj->m_sequencePath.GetBuffer(),FILE_ATTRIBUTE_NORMAL))
		strcpy(relativePath,m_veprj->m_sequencePath.GetBuffer());

	fprintf(fp,"Sequence:%s\n",relativePath);
	fprintf(fp,"start:%d\n",m_veprj->GetStartFrame());
	fprintf(fp,"end:%d\n",m_veprj->GetEndFrame());
	fprintf(fp,"</Image Sequence>\n\n");

	fprintf(fp,"<Project Info>\n");
	fprintf(fp,"LayerNum:%d\n",m_veprj->GetLayerNum());
	fprintf(fp,"DspMin:%f\n",m_veprj->m_fDspMin);
	fprintf(fp,"DspMax:%f\n",m_veprj->m_fDspMax);
	fprintf(fp,"</Project Info>\n\n");

	for (int i=0;i<m_veprj->GetLayerNum();++i)
	{
		fprintf(fp,"<Layer>\n");
		fprintf(fp,"Name:%s\n",m_veprj->vec_layer[i].GetName());
		fprintf(fp, "Type:%d\n", m_veprj->vec_layer[i].GetType());
		fprintf(fp,"StartIndex:%d\n",m_veprj->vec_layer[i].GetStartIndex());
		fprintf(fp,"KeyFrameNum:%d\n",m_veprj->vec_layer[i].keyFrame.size());

		fprintf(fp,"<KeyFrame>\n");
		for (int j=0;j<m_veprj->vec_layer[i].keyFrame.size();++j)
		{
			fprintf(fp,"%d ",m_veprj->vec_layer[i].keyFrame[j]);
		}
		fprintf(fp,"\n");
		fprintf(fp,"</KeyFrame>\n");

		fprintf(fp,"<PlanePoints>\n");

		for (int j=0;j<m_veprj->vec_layer[i].plane.size();++j)
		{
			fprintf(fp,"%d %d %d ",(m_veprj->vec_layer[i].plane[j]).x,(m_veprj->vec_layer[i].plane[j]).y,(m_veprj->vec_layer[i].plane[j]).z);
		}
		fprintf(fp,"\n");
		fprintf(fp,"</PlanePoints>\n");
		fprintf(fp,"</Layer>\n\n");

	}
	fclose(fp);

}

void CVEFileParser::ParsePolygons()
{
	printf("begin load polygon...\n");
	int point_sum = 0;
	FILE* fp;
	if ( ( fp = fopen( "polygon.txt","r" ) ) != NULL )
	{
		int vec_layer_size;
		fscanf( fp, "%d", &vec_layer_size );
		//printf("*there are %d layers\n",vec_layer_size);
		if ( vec_layer_size == m_veprj->vec_layer.size() )
		{
			for ( int i = 0; i < vec_layer_size; ++i )
			{//enumerate layer
				CPolygonManager polygonManager;
				int frame_num;
				fscanf( fp, "%d", &frame_num );
				//printf("**there are %d frames having polygonSet\n",frame_num);
				for ( int j = 0; j< frame_num; ++j )
				{//enumerate polygon of each frame
					int frame_index;
					CPolygonSet polygon_set;
					int polygon_num;
					fscanf( fp, "%d %d", &frame_index, &polygon_num );
					//printf("***there are %d polygons in frame %d\n",polygon_num, frame_index);
					for ( int l = 0; l < polygon_num; ++l )
					{
						int point_num;
						fscanf( fp, "%d", &point_num );
						//printf("****there are %d points in polygon %d\n",point_num, l);
						if ( point_num == 0 )
						{
							continue;
						}
						CPolygon polygon;
						int x, y;
						for ( int k = 0; k < point_num; ++k )
						{//enumerate point of each polygon
							fscanf( fp, "%d %d", &x, &y );
							point_sum++;
							polygon.m_vPointSet.push_back( CPoint( x, y ) );
						}
						for ( int i = 0; i < polygon.m_vPointSet.size(); ++i )
						{
							polygon.m_vEdgeSet.push_back( pair<CPoint ,CPoint>( polygon.m_vPointSet[i], polygon.m_vPointSet[( i + 1 ) % polygon.m_vPointSet.size() ] ) );
						}
						polygon_set.m_polygon.push_back( polygon );
					}
					polygonManager.AddPolygon( frame_index, polygon_set );
				}
				m_veprj->vec_layer[i].SetPolygonManager( polygonManager );
			}
		}
		else
		{
			printf("mismatch of layer number, no polygon loaded!\n");
		}	
		fclose( fp );
	}
	else printf("no polygon.txt file found\n");
	printf("there are %d points loaded\n",point_sum);
	printf("end load polygon...\n");

}