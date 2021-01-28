#include "../GrabCut/Cutting.h"
#include "../GrabCut/gmm.h"

//
//int Run()
//{
//	CxImage image;
//	Cutting cut;
//
//	image.Load("Data/image1.jpg");
//	cut.Init(&image);
//	cut.GetCuttingRegion(110, 60, 370, 270); //1
////	cut.GetCuttingRegion(92, 45, 350, 390);  //2
////	cut.GetCuttingRegion(42, 0, 227, 143);   //3
////	cut.GetCuttingRegion(26, 19, 482, 328);   //4
////	cut.GetCuttingRegion(143, 26, 311, 372);   //5
////	cut.GetCuttingRegion(130, 76, 305, 486);   //6
////	cut.GetCuttingRegion(90, 25, 255, 460);   //7
////	cut.GetCuttingRegion(305, 120, 613, 505);   //8
////	cut.GetCuttingRegion(200, 0, 486, 386);   //9
////	cut.GetCuttingRegion(132, 54, 480, 384);   //10
//
//	cut.Calculate();
//
//	return 0;
//}