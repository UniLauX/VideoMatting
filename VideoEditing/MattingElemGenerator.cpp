#include "MattingElemGenerator.h"
#include "../grab/BiImageProcess.h"
#include <algorithm>
#undef min
#undef  max
MattingElemGenerator::MattingElemGenerator(void)
{
}

MattingElemGenerator::~MattingElemGenerator(void)
{
}

void MattingElemGenerator::GenerateMattingElem(std::vector<MattingElem*>& vec_matElem, CxImage* trimap, CxImage* colImage)
{
	VVPOINTS centers;
	int width = trimap->GetWidth();
	int height = trimap->GetHeight();

	int startx=width-1, starty=height-1, endx=0, endy=0;
	for (int j=0; j<height; ++j)
		for (int i=0; i<width; ++i)
		{
			if (trimap->GetPixelGray(i,j)==128)
			{
				startx = std::min(startx, i);
				starty = std::min(starty, j);
				endx = std::max(endx, i);
				endy = std::max(endy, j);
			}
		}

		startx = (startx-100>0)?startx-100:0;
		starty = (starty-100>0)?starty-100:0;
		endx = (endx+100<width)?endx+100:width-1;
		endy = (endy+100<height)?endy+100:height-1;


		int tempwidth = endx -startx+1;
		int tempheight = endy-starty+1;

		int TRIMAPWIDTH=400;
		if (tempwidth>tempheight)
		{
			int heightmat = tempheight/2;
			if (tempwidth%TRIMAPWIDTH>TRIMAPWIDTH/2)
			{
				TRIMAPWIDTH+=100;
			}
			int count = tempwidth/TRIMAPWIDTH;
			for (int i=0; i<count; ++i)
			{
				for (int j=0; j<2; ++j)
				{
					int mattelemSx = startx+i*TRIMAPWIDTH;
					int mattelemSy = starty+j*heightmat;
					int mattelemHeight = heightmat;
					int mattelemWidth;

					if (mattelemSx+TRIMAPWIDTH<endx&&i!=count-1)
					{
						mattelemWidth = TRIMAPWIDTH;
					}
					else
						mattelemWidth = (endx-mattelemSx+1);

					MattingElem* newMattElem = new MattingElem(mattelemSx, mattelemSy, mattelemWidth, mattelemHeight);
					for (int itemp=0; itemp<mattelemWidth; ++itemp)
					{
						for (int jtemp=0; jtemp<mattelemHeight; ++jtemp)
						{
							int xindex = itemp+mattelemSx;
							int yindex = jtemp+mattelemSy;
							newMattElem->colorImage->SetPixelColor(itemp, jtemp, colImage->GetPixelColor(xindex, yindex));
							newMattElem->trimapImage->SetPixelColor(itemp, jtemp, trimap->GetPixelColor(xindex, yindex));
						}
					}
					vec_matElem.push_back(newMattElem);
				}
			}
		}
		else
		{
			int widthmat = tempwidth/2;
			if (tempheight%TRIMAPWIDTH>TRIMAPWIDTH/2)
				TRIMAPWIDTH+=100;
			int count = tempheight/TRIMAPWIDTH;
			for (int i=0; i<count; ++i)
			{
				for (int j=0; j<2; ++j)
				{
					int mattelemSx  = startx+j*widthmat;
					int mattelemSy = starty+i*TRIMAPWIDTH;
					int mattelemWidth = widthmat;
					int mattelemHeight;
					if (mattelemSy+TRIMAPWIDTH<endy&&i!=count-1)
							mattelemHeight = TRIMAPWIDTH;
					else
						mattelemHeight = endy - mattelemSy +1;

					MattingElem* newMattElem = new MattingElem(mattelemSx, mattelemSy, mattelemWidth, mattelemHeight);
					for (int itemp=0; itemp<mattelemWidth; ++itemp)
					{
						for (int jtemp=0; jtemp<mattelemHeight; ++jtemp)
						{
							int xindex = itemp+mattelemSx;
							int yindex = jtemp+mattelemSy;
							newMattElem->colorImage->SetPixelColor(itemp, jtemp, colImage->GetPixelColor(xindex, yindex));
							newMattElem->trimapImage->SetPixelColor(itemp, jtemp, trimap->GetPixelColor(xindex, yindex));
						}
					}
					vec_matElem.push_back(newMattElem);
				}
			}
		}
}
