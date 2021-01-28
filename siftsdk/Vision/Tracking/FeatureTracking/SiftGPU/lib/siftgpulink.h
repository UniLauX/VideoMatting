#pragma once

#define _LIBPATH_	"Z:\\Vision\\Tracking\\FeatureTracking\\siftgpu\\lib\\"

#if _MSC_VER >= 1400 // for vc8
	#ifdef _DEBUG
	#pragma comment(lib, ""_LIBPATH_"siftgpu_d.lib")
	#else if
	#pragma comment(lib, ""_LIBPATH_"siftgpu.lib")
	#endif
#endif

#undef _LIBPATH_