//
//	TubeGen - Single-Wall Carbon Nanotube Generator
//
//  Bitmap
//  Pseudo-class which maintains what is effectively a minimal-storage
//  array of boolean values.  Written because the native C++ "bitset"
//  class stinks.
//
//  Created by Jeffrey Frey on Thu Jan 13 2005.
//  Doren Research Group
//  University of Delaware
//  Copyright (c) 2003-2005
//

#ifndef __BITMAP__
#define __BITMAP__

// Removed for Avogadro:
// #include <stdc-includes.h>
//
// Replaced with:
#include <stdlib.h>
#include <stdio.h>
#ifdef _MSC_VER
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint8_t Bit;

typedef struct _Bitmap* BitmapRef;

BitmapRef BitmapCreateWithCapacity(unsigned capacity);
void BitmapDestroy(BitmapRef aBitmap);

void BitmapSummarizeToStream(FILE* stream,BitmapRef aBitmap);

unsigned BitmapGetCapacity(BitmapRef aBitmap);

Bit BitmapGetBit(BitmapRef aBitmap,unsigned aBit);
void BitmapSetBit(BitmapRef aBitmap,unsigned aBit,Bit value);
void BitmapFlipBit(BitmapRef aBitmap,unsigned aBit);

unsigned BitmapGetNextBitWithValue(BitmapRef aBitmap,unsigned startAtBit,Bit value);

#if defined(__cplusplus)
}
#endif

#endif /* __BITMAP__ */
