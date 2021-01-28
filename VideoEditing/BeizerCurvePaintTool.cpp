#include "BeizerCurvePaintTool.h"
#include "Beizer.h"
#include "cv.h"
#include <cmath>

#include "RotoScopeInterface.h"
#include "ImageSequenceManager.h"
#include <iostream>
using namespace  std;

#define GPEN 2
#define BPEN 1
#define TEMPCURVE 10

namespace VideoActiveTool{
	CBeizerCurvePaintTool::CBeizerCurvePaintTool( void )
	{
		m_bIsDrawBeizer = false;
		m_bisDrawing = false;
		m_bIsLeftbuttonDown = false;
		m_bIsInsertPoint = false;
		m_bIsNeedInitialize = true;
		m_bNeedRotoScope = false;
		m_bIsDrawOver = false;
		beizer = new Beizer;
		MaskMat = NULL;
		keyFrameFlag = NULL;
		keyFrameUpdateFlag = NULL;
		imageSequence = NULL;
		rotoScopeInterface = NULL;
		m_bIsMovingPoint = false;
		rotoScopeInterface = NULL;
		CurveManager = NULL;

	}

	CBeizerCurvePaintTool::~CBeizerCurvePaintTool(void)
	{
		printf("~CBeizerCurvePaintTool\n");
	}

	CTool * CBeizerCurvePaintTool::Clone() const
	{
		return new CBeizerCurvePaintTool;
	}

	bool CBeizerCurvePaintTool::Activate()
	{
		setDraw();
		return true;
	}

	void CBeizerCurvePaintTool::Deactivate()
	{
		m_bIsDrawBeizer = false;
	}

	int CBeizerCurvePaintTool::WindowProc( CWnd * pWnd, UINT Message, WPARAM wParam, LPARAM lParam )
	{
		if (m_bIsDrawBeizer)
		{
			if (pWnd->GetRuntimeClass()->IsDerivedFrom(RUNTIME_CLASS(CVideoEditingView)))
			{
				pView=dynamic_cast<CVideoEditingView*>(pWnd);
				imageView = pView->GetImageEditingView();
				imageView->SetLineColor(RGB(255,0,0));
				
				if (pView->b_IsRotoScopingReadData)//为了直接显示
				{
					curruntFrameIndex = -1;
				}
				
				if (m_bIsNeedInitialize)
				{
					m_bIsNeedInitialize = false;
					if (!pView->b_IsRotoScopingReadData)
					{
						int height = pView->imageBuffer.GetImage()->GetHeight();
						int width  = pView->imageBuffer.GetImage()->GetWidth();

						if (pView->rotoScopeInterface != NULL)
						{
							delete pView->rotoScopeInterface;
						}
						pView->rotoScopeInterface = new RotoScopeInterface;
						
						pView->rotoScopeInterface->initializeRotoscopeData(pView->fileRange, height, width,  pView->imageBuffer.GetName(),
							pView->imageBuffer.GetExt(), pView->getStartIndex());
					
					}

					
					m_bIsDrawOver = pView->b_IsClosed;

					rotoScopeInterface = pView->rotoScopeInterface;

					CurveManager  = rotoScopeInterface->getDataManager();
					imageSequence = rotoScopeInterface->getImageSquenceManager();
					MaskMat       = rotoScopeInterface->getMaskMat();
					keyFrameFlag  = rotoScopeInterface->getKeyFrameFlag();
					keyFrameUpdateFlag = rotoScopeInterface->getKeyFrameUpdateFlag();

					/*this->CurveManager->initilaizeCurveData(pView->fileRange);
					interactiveMask = new set<int>[pView->fileRange];

					imageSequence = new ImageSequenceManager();

					imageSequence->pathname = pView->imageBuffer.GetName();
					imageSequence->extName  = pView->imageBuffer.GetExt();
					imageSequence->startVedioIndex = pView->getStartIndex();*/

					//cout << "get image" << endl;
					//CxImage * img = imageSequence->getImageBuffer(0)->GetImage();
					//
					////cout << img->GetNumColors() << endl;
					//IplImage * opencvImage = cvCreateImage(cvSize(img->GetWidth(), img->GetHeight()), IPL_DEPTH_8U, img->GetNumColors() / 8);
					//int channel = img->GetNumColors() / 8;
					//for (int i = 0; i < opencvImage->height; i++)
					//{
					//	unsigned char * imgdata = img->GetBits(i);
					//	for (int j = 0; j < opencvImage->width; j++)
					//	{
					//		CV_IMAGE_ELEM(opencvImage, uchar, i, j * channel) = *(imgdata + j * channel);
					//		CV_IMAGE_ELEM(opencvImage, uchar, i, j * channel + 1) = *(imgdata + j * channel + 1);
					//		CV_IMAGE_ELEM(opencvImage, uchar, i, j * channel + 2) = *(imgdata + j * channel + 2);
					//	}
					//}

					/*cvNamedWindow("a");
					cvShowImage("a", opencvImage);
					cvWaitKey(0);*/




					/*if (MaskMat != NULL)
					{
						cvReleaseMat(&MaskMat);
					}
					MaskMat = cvCreateMat(height, width, CV_32FC1);
					cvZero(MaskMat);

					if (keyFrameFlag != NULL)
					{
						delete[] keyFrameFlag;
					}
					keyFrameFlag = new int[pView->fileRange];
					memset(keyFrameFlag, 0, pView->fileRange * sizeof(int));

					if (keyFrameUpdateFlag != NULL)
					{
						keyFrameUpdateFlag->clear();
						delete keyFrameUpdateFlag;
					}
					keyFrameUpdateFlag = new vector<bool>(pView->fileRange, false);*/
					
					
				}

				pView->b_IsRotoScopingReadData = false;
				
				if (curruntFrameIndex != pView->imageBuffer.GetFramePos() - pView->getStartIndex())
				{
					
					if (m_bNeedRotoScope)
					{
						m_bNeedRotoScope = false;
						rotoScope();
					}
					curruntFrameIndex = pView->imageBuffer.GetFramePos() - pView->getStartIndex();

					cvZero(MaskMat);
					imageView->reLoad();
					imageView->updateLine(CurveManager->getCurveData(curruntFrameIndex));
					if (CurveManager->getCurveData(curruntFrameIndex).getNumofCurve() > 0)
					{
						int n = CurveManager->getCurveData(curruntFrameIndex).getNumofCurve();
						int * num = new int[n];
						CurveManager->getCurveData(curruntFrameIndex).getNumofDisplayPoint(num);
						imageView->updateCurves(CurveManager->getCurveData(curruntFrameIndex), num, MaskMat);
						delete[] num;
					}
				}
				
				
				//cout << "curruntFrame:" << curruntFrame << endl;
			}
			switch(Message){
				case WM_LBUTTONDOWN:
					{
						leftButtonDown(lParam);
					}
					break;
				case WM_MOUSEMOVE:
					{
						mouseMove(lParam);	
					}
					
					break;
				case WM_LBUTTONUP:
					{
						leftButtonUp(lParam);
					}
					break;
				case WM_LBUTTONDBLCLK:
					{
						leftButtonDoubleClick();
					}
				default:
					break;
			}
		}

		return 1;

	}

	void CBeizerCurvePaintTool::setDraw()
	{
		m_bIsDrawBeizer = true;
	}
	CPoint CBeizerCurvePaintTool::MakePoint(LPARAM lparam)
	{
		int x = GET_X_LPARAM( lparam );
		int y = GET_Y_LPARAM( lparam );
		pView->AdjustPos( x, y, imageView->GetRegion() );
		//cout << x << " " << y << endl;
		return CPoint( x, y );
		
	}

	void CBeizerCurvePaintTool::setLeftButton()
	{
		m_bIsLeftbuttonDown = true;
	}

	void CBeizerCurvePaintTool::leftButtonDown(LPARAM lParam)
	{
		if (isMovingPoint(lParam))
		{
			m_bIsMovingPoint = true;
			int index = movePointId / 3 * 3;
			//start_Point、prev_Point、curr_Point依次按顺序得到控制点
			start_Point = CurveManager->getCurveData(curruntFrameIndex).getPoint(index);
			prev_Point  = CurveManager->getCurveData(curruntFrameIndex).getPoint(index + 1);
			curr_Point  = CurveManager->getCurveData(curruntFrameIndex).getPoint(index + 2);
			/*imageView->adjustPoiToSys(start_Point.x, start_Point.y);
			imageView->adjustPoiToSys(prev_Point.x, prev_Point.y);
			imageView->adjustPoiToSys(curr_Point.x, curr_Point.y);*/

			if ((movePointId - 1) % 3 == 0) //3是三次曲线的端点控制点在控制点序列中的位移
			{
				movePointFlag = 1;//表示移动时当前点及其前后两点共同移动。即移动这个切线
			}
			else if (movePointId % 3 == 0)
			{
				movePointFlag = 2;//只移动两个个点，即调整切线的方向,调整的为三个点的前一个点
				distance = sqrt((double)((curr_Point.x - prev_Point.x) * (curr_Point.x - prev_Point.x) + (curr_Point.y - prev_Point.y) * (curr_Point.y - prev_Point.y)));
			}
			else
			{
				movePointFlag = 3; //移动后一个点
				distance = sqrt((double)((start_Point.x - prev_Point.x) * (start_Point.x - prev_Point.x) + (start_Point.y - prev_Point.y) * (start_Point.y - prev_Point.y)));

			}
			imageView->reLoad();
			int n = CurveManager->getCurveData(curruntFrameIndex).getNumofCurve();
			int * num = new int[n];
			CurveManager->getCurveData(curruntFrameIndex).getNumofDisplayPoint(num);
			imageView->updateCurves(CurveManager->getCurveData(curruntFrameIndex), num, MaskMat);
			imageView->updateLine(CurveManager->getCurveData(curruntFrameIndex), movePointId / 3);
			
		}
		else if (seachClosestPoint(MakePoint(lParam),5) != -1)
		{
			start_Point = MakePoint(lParam);
			curr_Point = start_Point;
			m_bIsInsertPoint = true;
		}
		else if (m_bIsDrawBeizer && !m_bIsDrawOver)
		{
			start_Point = MakePoint(lParam);
			curr_Point = start_Point;
			setLeftButton();
			m_bisDrawing = true;
		}

	}

	void CBeizerCurvePaintTool::mouseMove( LPARAM lParam )
	{
		if (m_bIsMovingPoint)
		{
			if (movePointFlag == 1)
			{
				ajudstControlPointTanslation(lParam);
			}
			else
			{
				ajustControlPointoffLine(lParam, distance);
			}
			int index = movePointId / 3 * 3;
			
			/*CPoint p1 = imageView->adjustPoiToImage(start_Point);
			CPoint p2 = imageView->adjustPoiToImage(prev_Point);
			CPoint p3 = imageView->adjustPoiToImage(curr_Point);*/
			CurveManager->getCurveData(curruntFrameIndex).updatePoint(start_Point, index, true);
			CurveManager->getCurveData(curruntFrameIndex).updatePoint(prev_Point, index + 1, true);
			CurveManager->getCurveData(curruntFrameIndex).updatePoint(curr_Point, index + 2, true);

			imageView->reLoad();
			imageView->updateLine(CurveManager->getCurveData(curruntFrameIndex), movePointId / 3);
			if (CurveManager->getCurveData(curruntFrameIndex).getNumofCurve() > 0)
			{
				int n = CurveManager->getCurveData(curruntFrameIndex).getNumofCurve();
				int * num = new int[n];
				CurveManager->getCurveData(curruntFrameIndex).getNumofDisplayPoint(num);
				imageView->updateCurves(CurveManager->getCurveData(curruntFrameIndex), num, MaskMat);
				delete[] num;
			}
		}
		else if (m_bIsInsertPoint)
		{
			CPoint tp;
			tp.x = 2 * start_Point.x - curr_Point.x;
			tp.y = 2 * start_Point.y - curr_Point.y;
			imageView->drawLine(tp, curr_Point, false);
			imageView->drawPoint(curr_Point);
			imageView->drawPoint(start_Point);
			imageView->drawPoint(tp);

			prev_Point = curr_Point;
			curr_Point = MakePoint(lParam);
			tp.x = 2 * start_Point.x - curr_Point.x;
			tp.y = 2 * start_Point.y - curr_Point.y;
			imageView->drawLine(tp, curr_Point, false);
			imageView->drawPoint(curr_Point);
			imageView->drawPoint(start_Point);
			imageView->drawPoint(tp);

			/*if (abs(curr_Point.x - start_Point.x) + abs(curr_Point.y - curr_Point.y) > 2)
			{
				tp.x = 2 * start_Point.x - curr_Point.x;
				tp.y = 2 * start_Point.y - curr_Point.y;
				imageView->drawLine(tp, curr_Point, true);
				imageView->drawPoint(curr_Point, true);
				imageView->drawPoint(start_Point, true);
				imageView->drawPoint(tp, true);

				CPoint p1 = imageView->adjustPoiToImage(tp);
				CPoint p2 = imageView->adjustPoiToImage(start_Point);
				CPoint p3 = imageView->adjustPoiToImage(curr_Point);

				int index = (insertCurveId + 1) * 3;
				CurveManager->getCurveData(curruntFrameIndex).insertPoint(p1, index);
				CurveManager->getCurveData(curruntFrameIndex).insertPoint(p2, index + 1);
				CurveManager->getCurveData(curruntFrameIndex).insertPoint(p3, index + 2);

				imageView->reLoad();
				imageView->updateLine(CurveManager->getCurveData(curruntFrameIndex));
				if (CurveManager->getCurveData(curruntFrameIndex).getNumofCurve() > 0)
				{
					int n = CurveManager->getCurveData(curruntFrameIndex).getNumofCurve();
					int * num = new int[n];
					CurveManager->getCurveData(curruntFrameIndex).getNumofDisplayPoint(num);

					imageView->updateCurves(CurveManager->getCurveData(curruntFrameIndex), num, MaskMat);
				}
			}*/

			

		}
		else if (m_bisDrawing && m_bIsLeftbuttonDown)
		{
			CPoint tp;
			//tp.x = 2 * start_Point.x - curr_Point.x;
			//tp.y = 2 * start_Point.y - curr_Point.y;
			//imageView->drawLine(tp, curr_Point, false);
			//imageView->drawPoint(curr_Point);
			////imageView->drawPoint(start_Point, false);
			//imageView->drawPoint(tp);

			imageView->reLoad();
			imageView->updateLine(CurveManager->getCurveData(curruntFrameIndex));

			prev_Point = curr_Point;
			curr_Point = MakePoint(lParam);
			tp.x = 2 * start_Point.x - curr_Point.x;
			tp.y = 2 * start_Point.y - curr_Point.y;
			imageView->drawLine(tp, curr_Point, false);
			imageView->drawPoint(curr_Point);
			imageView->drawPoint(start_Point);
			imageView->drawPoint(tp);
			//cout << "pos:" << curr_Point.x << " " << curr_Point.y << endl;


			if (CurveManager->getCurveData(curruntFrameIndex).getNumofCurve() > 0)
			{
				int n = CurveManager->getCurveData(curruntFrameIndex).getNumofCurve();
				int * num = new int[n];
				CurveManager->getCurveData(curruntFrameIndex).getNumofDisplayPoint(num);
				imageView->updateCurves(CurveManager->getCurveData(curruntFrameIndex), num, MaskMat);
				delete[] num;
			}
			int index = CurveManager->getCurveData(curruntFrameIndex).getNumofPoint() - 2;

			if (index > 0)
			{

				beizer = new Beizer;
				CPoint p = CurveManager->getCurveData(curruntFrameIndex).getPoint(index);

				beizer->addControlPoint(p.x, p.y);
				p = CurveManager->getCurveData(curruntFrameIndex).getPoint(index + 1);
				beizer->addControlPoint(p.x, p.y);
				//p = imageView->adjustPoiToImage(tp);
				beizer->addControlPoint(tp.x, tp.y);
				//p = imageView->adjustPoiToImage(start_Point);
				beizer->addControlPoint(start_Point.x, start_Point.y);
				imageView->showBeizerCurve( *beizer, TEMPCURVE, NULL, 0);
				delete beizer;
			}
			



			
			/*CurveManager->getCurveData(curruntFrameIndex).addPoint(p1,  true);
			CurveManager->getCurveData(curruntFrameIndex).addPoint(p2,  true);
			CurveManager->getCurveData(curruntFrameIndex).addPoint(p3,  true);

			CurveManager->getCurveData(curruntFrameIndex).pop_backPoint();
			CurveManager->getCurveData(curruntFrameIndex).pop_backPoint();
			CurveManager->getCurveData(curruntFrameIndex).pop_backPoint();*/
		}

	}

	void CBeizerCurvePaintTool::leftButtonUp( LPARAM lParam )
	{
		if (m_bIsMovingPoint)
		{
			m_bIsMovingPoint = false;
			//cout << "sys" << start_Point.x << " " << start_Point.y << endl;
			/*imageView->adjustPoiToImage(start_Point.x, start_Point.y);
			imageView->adjustPoiToImage(prev_Point.x, prev_Point.y);
			imageView->adjustPoiToImage(curr_Point.x, curr_Point.y);*/
			//cout << "image" << start_Point.x << " " << start_Point.y << endl;
			int index = movePointId / 3 * 3;
			CurveManager->getCurveData(curruntFrameIndex).updatePoint(start_Point, index, true);
			CurveManager->getCurveData(curruntFrameIndex).updatePoint(prev_Point, index + 1, true);
			CurveManager->getCurveData(curruntFrameIndex).updatePoint(curr_Point, index + 2, true);

			imageView->reLoad();
			imageView->updateLine(CurveManager->getCurveData(curruntFrameIndex));
			if (CurveManager->getCurveData(curruntFrameIndex).getNumofCurve() > 0)
			{
				int n = CurveManager->getCurveData(curruntFrameIndex).getNumofCurve();
				int * num = new int[n];
				CurveManager->getCurveData(curruntFrameIndex).getNumofDisplayPoint(num);
				imageView->updateCurves(CurveManager->getCurveData(curruntFrameIndex), num, MaskMat);
				delete[] num;
			}
			keyFrameFlag[curruntFrameIndex] = 1;
			m_bNeedRotoScope = true;
			(*keyFrameUpdateFlag)[curruntFrameIndex] = true;
			//addInteractiveMask(index);
			//addInteractiveMask(index + 1);
			//addInteractiveMask(index + 2);

		}
		else if (m_bIsInsertPoint)
		{
			cout << "Insert! " << endl;
			CPoint tp;
			tp.x = 2 * start_Point.x - curr_Point.x;
			tp.y = 2 * start_Point.y - curr_Point.y;
			imageView->drawLine(tp, curr_Point, false);
			imageView->drawPoint(curr_Point);
			imageView->drawPoint(start_Point);
			imageView->drawPoint(tp);

			prev_Point = curr_Point;
			curr_Point = MakePoint(lParam);
			if (abs(curr_Point.x - start_Point.x) + abs(curr_Point.y - start_Point.y) > 2)
			{
				tp.x = 2 * start_Point.x - curr_Point.x;
				tp.y = 2 * start_Point.y - curr_Point.y;
				imageView->drawLine(tp, curr_Point, true);
				imageView->drawPoint(curr_Point, GPEN, true);
				imageView->drawPoint(start_Point, BPEN, true);
				imageView->drawPoint(tp, GPEN, true);

				/*imageView->adjustPoiToImage(tp.x, tp.y);
				imageView->adjustPoiToImage(start_Point.x, start_Point.y);
				imageView->adjustPoiToImage(curr_Point.x, curr_Point.y);*/

				int index = (insertCurveId + 1) * 3;
				CurveManager->getCurveData(curruntFrameIndex).insertPoint(tp, index ,true);
				CurveManager->getCurveData(curruntFrameIndex).insertPoint(start_Point, index + 1, true);
				CurveManager->getCurveData(curruntFrameIndex).insertPoint(curr_Point, index + 2, true);
				double pdis = sqrt((double)(tp.x - start_Point.x) * (tp.x - start_Point.x) + (tp.y - start_Point.y) * (tp.y - start_Point.y));
				double ldis = sqrt((double)(curr_Point.x - start_Point.x) * (curr_Point.x - start_Point.x) + (curr_Point.y - start_Point.y) * (curr_Point.y - start_Point.y));
				CurveManager->insertPointPropagate((float)insertCurveId + insertPointIndex, pdis, ldis, curruntFrameIndex);

				imageView->reLoad();
				imageView->updateLine(CurveManager->getCurveData(curruntFrameIndex));
				if (CurveManager->getCurveData(curruntFrameIndex).getNumofCurve() > 0)
				{
					int n = CurveManager->getCurveData(curruntFrameIndex).getNumofCurve();
					int * num = new int[n];
					CurveManager->getCurveData(curruntFrameIndex).getNumofDisplayPoint(num);
					imageView->updateCurves(CurveManager->getCurveData(curruntFrameIndex), num, MaskMat);
					delete[] num;
				}
				keyFrameFlag[curruntFrameIndex] = 1;
				m_bNeedRotoScope = true;
				(*keyFrameUpdateFlag)[curruntFrameIndex] = true;
			}
			m_bIsInsertPoint = false;
			
		}
		else if (m_bisDrawing && m_bIsLeftbuttonDown)
		{
			CPoint tp;
			tp.x = 2 * start_Point.x - curr_Point.x;
			tp.y = 2 * start_Point.y - curr_Point.y;
			imageView->drawLine(tp, curr_Point, false);
			imageView->drawPoint(curr_Point, false);
			imageView->drawPoint(start_Point, false);
			imageView->drawPoint(tp, false);

			prev_Point = curr_Point;
			curr_Point = MakePoint(lParam);
			if (abs(curr_Point.x - start_Point.x) + abs(curr_Point.y - start_Point.y) > 2)
			{
				tp.x = 2 * start_Point.x - curr_Point.x;
				tp.y = 2 * start_Point.y - curr_Point.y;
				imageView->drawLine(tp, curr_Point, true);
				imageView->drawPoint(curr_Point, GPEN, true);
				imageView->drawPoint(start_Point, BPEN, true);
				imageView->drawPoint(tp, GPEN, true);

				/*imageView->adjustPoiToImage(tp.x, tp.y);
				imageView->adjustPoiToImage(start_Point.x, start_Point.y);
				imageView->adjustPoiToImage(curr_Point.x, curr_Point.y);*/
				
				CurveManager->getCurveData(curruntFrameIndex).addPoint(tp, true);
				//cout << "add 1" << endl;
				CurveManager->getCurveData(curruntFrameIndex).addPoint(start_Point, true);
				//cout << "add 2" << endl;
				CurveManager->getCurveData(curruntFrameIndex).addPoint(curr_Point, true);
				//cout << "add 3" << endl;
				//cout << CurveManager->getCurveData(curruntFrame).getNumofCurve() << endl;
				
			}

			
			else if (CurveManager->getCurveData(curruntFrameIndex).getNumofCurve() > 0)
			{
				imageView->reLoad();
				imageView->updateLine(CurveManager->getCurveData(curruntFrameIndex));
				int n = CurveManager->getCurveData(curruntFrameIndex).getNumofCurve();
				int * num = new int[n];
				CurveManager->getCurveData(curruntFrameIndex).getNumofDisplayPoint(num);
				imageView->updateCurves(CurveManager->getCurveData(curruntFrameIndex), num, MaskMat);
				delete[] num;

			}

			m_bisDrawing = false;
			m_bIsLeftbuttonDown = false;
			keyFrameFlag[curruntFrameIndex] = 1;
			m_bNeedRotoScope = true;
			(*keyFrameUpdateFlag)[curruntFrameIndex] = true;
			//int index = (CurveManager->getCurveData(curruntFrameIndex).getNumofCurve()) * 3;
			//addInteractiveMask(index);
			//addInteractiveMask(index + 1);
			//addInteractiveMask(index + 2);
		}

	}

	void CBeizerCurvePaintTool::leftButtonDoubleClick()
	{
		CurveManager->setCurvesClosed();
		m_bIsDrawOver = true;
		beizer = new Beizer;
		int index = CurveManager->getCurveData(curruntFrameIndex).getNumofPoint();
		CPoint p = CurveManager->getCurveData(curruntFrameIndex).getPoint(index - 2);
		beizer->addControlPoint(p.x, p.y);
		p = CurveManager->getCurveData(curruntFrameIndex).getPoint(index - 1);
		beizer->addControlPoint(p.x, p.y);
		p = CurveManager->getCurveData(curruntFrameIndex).getPoint(0);
		beizer->addControlPoint(p.x, p.y);
		p = CurveManager->getCurveData(curruntFrameIndex).getPoint(1);
		beizer->addControlPoint(p.x, p.y);
		imageView->showBeizerCurve( *beizer, 10, MaskMat, 0);
		//imageView->showMaskMat(MaskMat);
		delete beizer;
	}

	bool CBeizerCurvePaintTool::isMovingPoint( LPARAM lparam )
	{
		CPoint p = MakePoint(lparam);
		/*imageView->adjustPoiToImage(p.x, p.y);*/
		movePointId = CurveManager->getCurveData(curruntFrameIndex).searchClosestPoint(p);
		if (movePointId == -1)
		{
			return false;
		}
		return true;
	}

	void CBeizerCurvePaintTool::ajudstControlPointTanslation( LPARAM lparam )
	{
		CPoint p = MakePoint(lparam);

		imageView->drawLine(start_Point, curr_Point, false);
		imageView->drawPoint(curr_Point, false);
		imageView->drawPoint(start_Point, false);
		imageView->drawPoint(prev_Point, false);

		LONG dx = p.x - prev_Point.x;
		LONG dy = p.y - prev_Point.y;

		start_Point.x += dx;
		start_Point.y += dy;
		curr_Point.x  += dx;
		curr_Point.y  += dy;
		prev_Point.x  += dx;
		prev_Point.y  += dy;

		imageView->drawLine(start_Point, curr_Point, false);
		imageView->drawPoint(curr_Point, false);
		imageView->drawPoint(start_Point, false);
		imageView->drawPoint(prev_Point, false);

	}

	void CBeizerCurvePaintTool::ajustControlPointoffLine( LPARAM lparam, float distance )
	{
		CPoint p = MakePoint(lparam);
		if (movePointFlag == 3)
		{
			
			imageView->drawLine(start_Point, curr_Point, false);
			imageView->drawPoint(curr_Point, false);
			imageView->drawPoint(start_Point, false);
			imageView->drawPoint(prev_Point, false);

			curr_Point = p;
			/*double d2 = sqrt((double)((curr_Point.x - prev_Point.x) * (curr_Point.x - prev_Point.x) + (curr_Point.y - prev_Point.y) * (curr_Point.y - prev_Point.y)));
			double sin = (double)(curr_Point.x - prev_Point.x) / d2;
			double cos = (double)(curr_Point.y - prev_Point.y) / d2;
			start_Point.x = prev_Point.x - distance * sin;
			start_Point.y = prev_Point.y - distance * cos;*/
			start_Point.x = 2 * prev_Point.x - curr_Point.x;
			start_Point.y = 2 * prev_Point.y - curr_Point.y;

			imageView->drawLine(start_Point, curr_Point, false);
			imageView->drawPoint(curr_Point, false);
			imageView->drawPoint(start_Point, false);
			imageView->drawPoint(prev_Point, false);
		}
		else if(movePointFlag == 2)
		{
	

			imageView->drawLine(start_Point, curr_Point, false);
			imageView->drawPoint(curr_Point, false);
			imageView->drawPoint(start_Point, false);
			imageView->drawPoint(prev_Point, false);

			start_Point = p;
			/*double d2 = sqrt((double)((start_Point.x - prev_Point.x) * (start_Point.x - prev_Point.x) + (start_Point.y - prev_Point.y) * (start_Point.y - prev_Point.y)));
			double sin = (double)(start_Point.x - prev_Point.x) / d2;
			double cos = (double)(start_Point.y - prev_Point.y) / d2;
			curr_Point.x = prev_Point.x - int(distance * sin);
			curr_Point.y = prev_Point.y - int(distance * cos);*/
			curr_Point.x = 2 * prev_Point.x - start_Point.x;
			curr_Point.y = 2 * prev_Point.y - start_Point.y;
			
			

			imageView->drawLine(start_Point, curr_Point, false);
			imageView->drawPoint(curr_Point, false);
			imageView->drawPoint(start_Point, false);
			imageView->drawPoint(prev_Point, false);

		}

	}

	float CBeizerCurvePaintTool::seachClosestPoint( CPoint& p, float maxdis )
	{
		//imageView->adjustPoiToImage(p.x, p.y);
		int x = p.x; 
		int y = p.y;
		bool flag = false;
		float result = -1;
		
		for (int i = 0; i < maxdis; i++)
		{
			int j1 = max(0, y - i);
			int j2 = min(MaskMat->rows - 1, y + i);
			int k1 = max(0, x + i);
			int k2 = min(MaskMat->cols - 1, x + i);
			for (int j = j1; (j <= j2) && !flag; j++)
			{
				if (CV_MAT_ELEM(*MaskMat, float, j, k1) != 0)
				{
					result = CV_MAT_ELEM(*MaskMat, float, j, k1);
					flag = true;
					break;
				}
				if (CV_MAT_ELEM(*MaskMat, float, j, k2) != 0)
				{
					result = CV_MAT_ELEM(*MaskMat, float, j, k2);
					flag = true;
					break;
				}
			}

			for (int k = k1; (k <= k2) && !flag; k++)
			{
				if (CV_MAT_ELEM(*MaskMat, float, j1, k) != 0)
				{
					result = CV_MAT_ELEM(*MaskMat, float, j1, k);
					flag = true;
					break;
				}
				if (CV_MAT_ELEM(*MaskMat, float, j2, k) != 0)
				{
					result = CV_MAT_ELEM(*MaskMat, float, j2, k);
					flag = true;
					break;
				}
			}
			if (flag)
			{
				insertCurveId = int(result);
				insertPointIndex = result - insertCurveId;
				return result;
			}
		}
		insertCurveId = int(result);
		insertPointIndex = result - insertCurveId;
		return result;
	}

	void CBeizerCurvePaintTool::rotoScope()
	{
		cout << "rotoscope!\n" << endl;
		int prev = 0;
		int cur = 0;

		int prevUpdate = 0;
		int curUpdate  = 0;

		


		for (int i = 0; i < pView->fileRange; ++i)
		{
			if (keyFrameFlag[i] == 1)
			{
				prev = cur;
				cur = i;
				
				if ((*keyFrameUpdateFlag)[i])
				{
					
					CurveManager->simplePropagate(0, i, i);
					(*keyFrameUpdateFlag)[i] = false;
					prevUpdate = curUpdate;
					curUpdate = i;
				}
				break;
			}
		}


		for (int i = cur + 1; i < pView->fileRange; ++i)
		{
			if (keyFrameFlag[i] == 1)
			{
				prev = cur;
				cur = i;
				if (prev == curUpdate && cur > prev + 1)
				{
					//rotoScopeInterface = new RotoScopeInterface;
					initializeRotoScopeParameter(rotoScopeInterface, prev, cur);
					//rotoscope!!!!!
					//delete rotoScopeInterface;
				}
				if ((*keyFrameUpdateFlag)[i])
				{
					(*keyFrameUpdateFlag)[i] = false;
					prevUpdate = curUpdate;
					curUpdate = i;
					if (prev != prevUpdate && cur > prev + 1)
					{
						//rotoScopeInterface = new RotoScopeInterface;
						initializeRotoScopeParameter(rotoScopeInterface, prev, cur);
						//rotoscope!!!!!
						//delete rotoScopeInterface;
					}
					
				}
			}
		}

		
		if (curUpdate == cur)
		{
			cout << "backforward Propagate" << endl;
			CurveManager->simplePropagate(cur, pView->fileRange - 1, cur);
		}
		cout << "rotoscop finish!" <<endl;
	}

	void CBeizerCurvePaintTool::rotoScope( int ta, int tb )
	{
		rotoScopeInterface = new RotoScopeInterface;
		initializeRotoScopeParameter(rotoScopeInterface, ta, tb);
		rotoScopeInterface->startRotoscope();
	}

	void CBeizerCurvePaintTool::initializeRotoScopeParameter(RotoScopeInterface * rotointerface, int ta, int tb )
	{
		imageSequence->loadImage(ta, tb - ta + 1);
		
		for (int i = ta; i <= tb; i++)
		{
			rotointerface->loadImage(imageSequence->getImageBuffer(i)->GetImage());
		}
		rotointerface->loadControlPointAndMask(*CurveManager,  ta, tb);
	}



	/*void CBeizerCurvePaintTool::insertPointPropagate(double length)
	{
		for (int i = 0; i < pView->fileRange; i++)
		{
			if (i != curruntFrameIndex)
			{
				beizer.clearControlPoint();
				BeizerCurveData& curveData = CurveManager->getCurveData(i);
				int index = 3 * insertCurveId + 1;
				for (int j = 0; j < 4; j++)
				{
					CPoint p = curveData.getPoint(index + j);
					beizer->addControlPoint(p);
				}

			}
		}
	}*/

	/*void CBeizerCurvePaintTool::addInteractiveMask( int i )
	{
		interactiveMask[curruntFrameIndex].insert(i);
	}

	void CBeizerCurvePaintTool::updateInteractiveMask( int form, int cur )
	{
		interactiveMask[curruntFrameIndex].erase(form);
		interactiveMask[curruntFrameIndex].insert(cur);
	}*/

	//void CBeizerCurvePaintTool::insertMask2InteractiveMask( int index )
	//{
	//	for (set<int>::reverse_iterator riter = interactiveMask->rbegin(); riter != interactiveMask->rend(); )
	//	{
	//		if (*riter >= index)
	//		{
	//			int temp = *riter;
	//			//interactiveMask->erase(r)

	//		}
	//	}
	//}

}