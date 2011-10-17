/* ===============================================================================
// CoreConst.h												 �1999-2001, J.T. Frey
// ===============================================================================
// Written:		J.T. Frey, 03/11/2001
// Purpose:		Numerical constants to varying precisions.
//
// Last Mod:	n/a
*/

#ifndef __CORECONSTANTS__
#define __CORECONSTANTS__

// Removed for Avogadro:
// #include <stdc-includes.h>
//
// Replaced with:
#include <math.h>
#include <float.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* Maximum possible number representable by a float:  */
#ifdef FLT_MAX
  #define	kMaxFloat				FLT_MAX
#else
  #define	kMaxFloat				3.40282347e+38F
#endif

/* Minimum possible number representable by a float:  */
#ifdef FLT_MIN
  #define	kMinFloat				FLT_MIN
#else
  #define	kMinFloat				1.17549435e-38F
#endif
/* Maximum possible number representable by a double:  */
#ifdef DBL_MAX
  #define	kMaxDouble				DBL_MAX
#else
  #define	kMaxDouble				1.7976931348623157e+308
#endif
/* Minimum possible number representable by a double:  */
#ifdef DBL_MIN
  #define	kMinDouble				DBL_MIN
#else
  #define	kMinDouble				2.2250738585072014e-308
#endif
/* Smallest-representable float difference, (x1 - x2)  */
#ifdef FLT_EPSILON
  #define	kZeroFloat				FLT_EPSILON
#else
  #define	kZeroFloat				1.1920929e-07F
#endif
/* Smallest-representable double difference, (x1 - x2)  */
#ifdef DBL_EPSILON
  #define	kZeroDouble				DBL_EPSILON
#else
  #define	kZeroDouble				2.2204460492503131e-16
#endif

/*  Trig constants:  */
//  #define kPi						(double)(3.14159265358979323846)
//  #define kPiOver2				(double)(1.57079632679489661923)
//  #define k3PiOver2				(kPi + kPiOver2)
//  #define k2Pi					(kPi + kPi)
#define kPi					M_PI
#define kPiOver2		M_PI_2
#define k3PiOver2		(M_PI + M_PI_2)
#define k2Pi				(M_PI + M_PI)
#define	kDegreesToRadians		(kPi / (double)(180.))
#define	kRadiansToDegrees		((double)(180.) / kPi)

/*  Special numerical macros:  */
#define		Minimum(XXX,YYY)		((XXX >= YYY) ? (YYY) : (XXX))
#define		Maximum(XXX,YYY)		((XXX >= YYY) ? (XXX) : (YYY))

#if defined(__cplusplus)
}
#endif

#endif //__CORECONSTANTS__
