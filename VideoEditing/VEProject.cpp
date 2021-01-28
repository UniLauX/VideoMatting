#include "StdAfx.h"
#include "VEProject.h"

CVEProject::CVEProject(void)
:startFrame(0)
,endFrame(0)
,frameCount(0)
{
}

CVEProject::~CVEProject(void)
{
}

void CVEProject::SetStartFrame(int sIndex)
{
	startFrame=sIndex;
}

void CVEProject::SetEndFrame(int eIndex)
{
	endFrame=eIndex;
}

void CVEProject::SetFrameRange(int sIndex,int eIndex)
{
	SetStartFrame(sIndex);
	SetEndFrame(eIndex);
}

void CVEProject::SetFrameCount(int count)
{
	frameCount=count;
}

int CVEProject::GetStartFrame()
{
	return startFrame;
}

int CVEProject::GetEndFrame()
{
	return endFrame;
}

int CVEProject::GetFrameCount()
{
	return frameCount;
}

int CVEProject::GetLayerNum()
{
	return vec_layer.size();
}
void CVEProject::SavePolygon()
{
	printf("begin save polygon...\n");
	int point_sum = 0;
	FILE* fp;
	if ( ( fp = fopen( "polygon.txt","w" ) ) != NULL )
	{
		int vec_layer_size = vec_layer.size();
		fprintf( fp, "%d\n", vec_layer_size );
		//printf("*there are %d layers\n",vec_layer_size);
		for ( int i = 0; i < vec_layer_size; ++i )
		{//enumerate layer
			int frame_num;
			CPolygonManager polygonManager = vec_layer[i].GetPolygonManager();
			frame_num = polygonManager.m_PolygonMap.size();
			fprintf( fp, "%d\n", frame_num );
			//printf("**there are %d frames having polygonSet\n",frame_num);

			map<int, CPolygonSet>::iterator it;
			for ( it = polygonManager.m_PolygonMap.begin(); it != polygonManager.m_PolygonMap.end(); it++ )
			{//enumerate polygon of each frame
				int frame_index = ( *it ).first;
				CPolygonSet polygon_set = ( *it ).second;
				int polygon_num = polygon_set.m_polygon.size();

				fprintf( fp, "%d %d\n", frame_index, polygon_num );
				//printf("***there are %d polygons in frame %d\n",polygon_num, frame_index);
				for ( int k = 0; k < polygon_num; ++k )
				{
					int point_num = polygon_set.m_polygon[k].m_vPointSet.size();
					fprintf( fp, "%d\n", point_num );
					//printf("****there are %d points in polygon %d\n",point_num, k);
					int x, y;
					for ( int l = 0; l < point_num; ++l )
					{//enumerate point of each polygon
						x = polygon_set.m_polygon[k].m_vPointSet[l].x;
						y = polygon_set.m_polygon[k].m_vPointSet[l].y;
						fprintf( fp, "%d %d\n", x, y );
						point_sum++;
					}
				}
			}


		}
		fclose( fp );
	}
	else printf("can not open polygon.txt to write in\n");
	printf("there are %d points saved\n",point_sum);
	printf("end save polygon...\n");

}