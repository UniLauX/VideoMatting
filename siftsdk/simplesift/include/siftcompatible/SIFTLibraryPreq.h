  #pragma once
#pragma message("Include Begin: " __FILE__)

//#include <ToolLib/Include/ToolLib.h>
// #include "BaseLibRT/BaseLibRT.h"

#include <string>
#include <map>
#include <fstream>
#include <exception>
#include <vector>
#include <assert.h>
#include <iostream>
#include <utility>
#include <time.h>
#include <iomanip>
#include <sstream>
#include <list>
#include <set>
#include <queue>

#include <WmlVector2.h>
#include <WmlMatrix3.h>
#include <WmlVector3.h>
#include <WmlMatrix2.h>
#include <WmlGMatrix.h>
#include <WmlGVector.h>


#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0500
#include <windows.h>

// #ifdef max
// #undef max
// #endif
// #ifdef min
// #undef min
//#endif
// use max/min/swap or so in algorithm
// if max or min unreachable in code, undefine them right there.
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

// #ifdef PI
// #undef PI
// #endif


// since we will use debug mode in release phase
// and vice versa, so redefine new symbols to represent
// and make clear the developing phase.
#define DEV_PHASE_DEBUG		0
#define DEV_PHASE_RELEASE	1

#define DEV_PHASE DEV_PHASE_RELEASE

#if DEV_PHASE == DEV_PHASE_DEBUG

// in debug phase, we want assert to take place in either
// debug or release mode.

#ifdef _DEBUG
// #define XASSERT(x) do{if(!(x)){LibUtil::LOG("Assertion Failed: " #x);assert(false);}}while(0)
#define XASSERT(x) assert(x)
#else
#define XASSERT(x) assert(x)
//#define XASSERT(x)
#endif

#else

// in release phase, no assert in release mode.
// but using _ASSERT directly will cause code losing !!!
#ifdef _DEBUG
#define XASSERT(x) assert(x)
#else
#define XASSERT(x) (x)
#endif

#endif

#ifdef _ASSERT
#undef _ASSERT
#define _ASSERT XASSERT
#endif

#define XASSERT_EQUAL(a, b) XASSERT((a) == (b))
#define XASSERT_FALSE(x) XASSERT(!(x))
#define XASSERT_TRUE(x) XASSERT(x)

//#define METHOD_NOT_IMPLEMENTED do{XASSERT(!(__FUNCTION__ " not Implemented."));}while(0)
#define METHOD_NOT_IMPLEMENTED assert(NULL == "Function not implemented.")

template<typename T>
std::string TOSTR(T val)
{
	std::stringstream resStr;
	resStr << val;
	return resStr.str();
}

#define TOSTRD(x) TOSTR(double(x))
#define TOSTRI(x) TOSTR(int(x))

// for backward compatible
#define TOSTRF(x) TOSTRD(x)

template<typename T>
inline T Pow2(T x)
{
	return x*x;
}

template<typename T>
inline T Pow3(T x)
{
	return x*x*x;
}

template<typename T>
inline T Pow4(T x)
{
	return Pow2(x) * Pow2(x);
}

template<typename T>
inline T Pow5(T x)
{
	return Pow2(x) * Pow3(x);
}

template<typename T>
inline T Pow6(T x)
{
	return Pow3(x) * Pow3(x);
}

	template<typename T>
	T Max(T left, T right)
	{
		return left > right ? left : right;
	}
	
	template<typename T>
	T Min(T left, T right)
	{
		return left > right ? right : left;
	}

//int round(double value);

#define OUTVAR(var, stream) stream << #var << " = " << var << std::endl
#define OUTARR(var, idx, stream) stream << #var << "[" << idx << "] = " << var[idx] << std::endl

namespace SIFT{

// define classes, in order to prevent including problem.
template<typename T> class ImageArray;
typedef ImageArray<float> ImageArrayf;
typedef ImageArray<double> ImageArrayd;
typedef ImageArray<int> ImageArrayi;


class CKeypoint;
typedef std::vector<CKeypoint*> KeyPointVector;

} // namespace SIFT

#pragma message("Include End: " __FILE__)

