#ifndef __WMLMATHLIB__H
#define __WMLMATHLIB__H

#ifndef LIBRARY_NOAUTOLIB
#ifdef _DEBUG
#pragma comment(lib, "magicmath_d.lib")
#else
#pragma comment(lib, "magicmath.lib")
#endif
#endif

#include "WmlSystem.h"
#include "WmlGMatrix.h"
#include "WmlGVector.h"
#include "WmlMatrix.h"
#include "WmlMatrix2.h"
#include "WmlMatrix3.h"
#include "WmlMatrix4.h"
#include "WmlVector.h"
#include "WmlVector2.h"
#include "WmlVector3.h"
#include "WmlVector4.h"
#include "WmlQuaternion.h"


#endif