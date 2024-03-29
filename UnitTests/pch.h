// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H


// aligned memory allocations
#ifdef _MSC_VER
#define ALIGN( x ) __declspec( align( x ) )
#define MALLOC64( x ) ( ( x ) == 0 ? 0 : _aligned_malloc( ( x ), 64 ) )
#define FREE64( x ) _aligned_free( x )
#else
#define ALIGN( x ) __attribute__( ( aligned( x ) ) )
#define MALLOC64( x ) ( ( x ) == 0 ? 0 : aligned_alloc( 64, ( x ) ) )
#define FREE64( x ) free( x )
#endif
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define CHECK_RESULT __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define CHECK_RESULT _Check_return_
#else
#define CHECK_RESULT
#endif

// basic types
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
#ifdef _MSC_VER
typedef unsigned char BYTE;		// for freeimage.h
typedef unsigned short WORD;	// for freeimage.h
typedef unsigned long DWORD;	// for freeimage.h
typedef int BOOL;				// for freeimage.h
#endif

#include "cmath"
#include "utility"
#include <iostream>
#include <xmmintrin.h>

namespace Tmpl8 {

}
using namespace Tmpl8;
using namespace std;
// add headers that you want to pre-compile here
#include "tmpl8math.h"

#endif //PCH_H
