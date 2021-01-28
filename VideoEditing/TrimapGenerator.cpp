#include "stdafx.h"
#include "TrimapGenerator.h"
#include <cximage.h>
#include <io.h>
#include "../grab/BiImageProcess.h"
#include "ImageBuffer.h"
TrimapGenerator::~TrimapGenerator(void)
{
}
void TrimapGenerator::GeneratorTrimap(ImageBuffer* trimapBuffer, ImageBuffer* alphaBuffer)
{	
	for (int i=m_iStart; ; ++i)
	{
		CString alphaname = alphaBuffer->GetFrameName(i);
		printf("%s\n ", alphaname.GetBuffer());
		if (_access(alphaname.GetBuffer(), 0)==-1)
		{
			printf("in break 1\n");
			break;		
		}
		CString trimapname = trimapBuffer->GetFrameName(i);
		if (_access(trimapname.GetBuffer(),0)==-1)//文件不存在
		{
			printf(" %s\n ", trimapname.GetBuffer());
			CxImage alpha;
			alpha.Load(alphaname.GetBuffer());
			CxImage trimap;
			trimap.Copy(alpha);
			BiImageProcess::GetTrimap(&alpha, trimap, m_iUnknownDis);
	    	trimap.Save(trimapBuffer->GetFrameName(i).GetBuffer(), CXIMAGE_FORMAT_PNG);
		}
	}
}