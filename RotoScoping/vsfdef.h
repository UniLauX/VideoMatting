
#ifndef _INC_VSFDEF_H
#define _INC_VSFDEF_H

#include<stddef.h>

#ifndef _VSF_API

#ifdef VSF_EXPORTS
#define _VSF_API __declspec(dllexport)
#else
#define _VSF_API __declspec(dllimport)
#endif

#endif

typedef unsigned char uchar;

#endif

