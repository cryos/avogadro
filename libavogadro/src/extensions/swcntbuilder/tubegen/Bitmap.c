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

#include "Bitmap.h"

typedef unsigned _BitmapElement;

static size_t _BitmapBitsPerElement = 8 * sizeof(_BitmapElement);

static _BitmapElement _BitmapElementAll = (_BitmapElement)(0xFFFFFFFFUL);
static _BitmapElement _BitmapElementNone = (_BitmapElement)(0x00000000UL);

typedef struct _Bitmap {
  unsigned        flags;
  unsigned        capacity;
  unsigned        elements;
  _BitmapElement  store[1];
} Bitmap;

//

  BitmapRef
  BitmapCreateWithCapacity(
    unsigned  capacity
  )
  {
    Bitmap*     newBitmap = NULL;
    size_t      bytes = sizeof(Bitmap);
    unsigned    storeSize = (capacity + _BitmapBitsPerElement - 1) / _BitmapBitsPerElement;
    
    if (storeSize) {
      bytes += (storeSize - 1) * sizeof(_BitmapElement);
      if (newBitmap = (Bitmap*)calloc(1,bytes)) {
        newBitmap->capacity = storeSize * _BitmapBitsPerElement;
        newBitmap->elements = storeSize;
      }
    }
    return (BitmapRef)newBitmap;
  }
  
//

  void
  BitmapDestroy(
    BitmapRef aBitmap
  )
  {
    free(aBitmap);
  }

//

  unsigned
  BitmapGetCapacity(
    BitmapRef aBitmap
  )
  {
    return aBitmap->capacity;
  }
  
//

  void
  BitmapSummarizeToStream(
    FILE*     stream,
    BitmapRef aBitmap
  )
  {
    unsigned    eIdx = 0;
    
    while (eIdx < aBitmap->elements) {
      unsigned        bIdx = _BitmapBitsPerElement;
      _BitmapElement  element = aBitmap->store[eIdx++];
      _BitmapElement  mask = 1;
      
      while (bIdx--) {
        if (element & mask)
          fputc('1',stream);
        else
          fputc('0',stream);
        mask <<= 1;
      }
    }
  }

//

  Bit
  BitmapGetBit(
    BitmapRef aBitmap,
    unsigned  aBit
  )
  {
    if (aBit < aBitmap->capacity) {
      unsigned  eIdx = aBit / _BitmapBitsPerElement;
      unsigned  bIdx = aBit % _BitmapBitsPerElement;
      
      if ((aBitmap->store[eIdx]) & (1 << bIdx))
        return 1;
    }
    return 0;
  }
  
//

  void
  BitmapSetBit(
    BitmapRef aBitmap,
    unsigned  aBit,
    Bit       value
  )
  {
    if (aBit < aBitmap->capacity) {
      unsigned  eIdx = aBit / _BitmapBitsPerElement;
      unsigned  bIdx = aBit % _BitmapBitsPerElement;
      
      if (value)
        aBitmap->store[eIdx] |= (1 << bIdx);
      else
        aBitmap->store[eIdx] &= ~(1 << bIdx);
    }
  }
  
//

  void
  BitmapFlipBit(
    BitmapRef aBitmap,
    unsigned  aBit
  )
  {
    if (aBit < aBitmap->capacity) {
      unsigned  eIdx = aBit / _BitmapBitsPerElement;
      unsigned  bIdx = aBit % _BitmapBitsPerElement;
      
      aBitmap->store[eIdx] ^= (1 << bIdx);
    }
  }
  
//

  unsigned
  BitmapGetNextBitWithValue(
    BitmapRef aBitmap,
    unsigned  startAtBit,
    Bit       value
  )
  {
    unsigned  eIdx = startAtBit / _BitmapBitsPerElement;
    unsigned  bIdx = startAtBit % _BitmapBitsPerElement;
    
    if (eIdx < aBitmap->elements) {
      if (value) {
        //  Eliminate all that are completely zero:
        while (eIdx < aBitmap->elements && aBitmap->store[eIdx] == _BitmapElementNone) {
          eIdx++;
          bIdx = 0;
        }
        //  Search through whatever remains:
        while (eIdx < aBitmap->elements) {
          _BitmapElement  mask = 1 << bIdx;
          _BitmapElement  element = aBitmap->store[eIdx];
          
          while (mask) {
            if (element & mask)
              return ((eIdx * _BitmapBitsPerElement) + bIdx);
            mask <<= 1;
          }
          bIdx = 0;
          eIdx++;
        }
      } else {
        //  Eliminate all that are completely set:
        while (eIdx < aBitmap->elements && aBitmap->store[eIdx] == _BitmapElementAll) {
          eIdx++;
          bIdx = 0;
        }
        //  Search through whatever remains:
        while (eIdx < aBitmap->elements) {
          _BitmapElement  mask = 1 << bIdx;
          _BitmapElement  element = aBitmap->store[eIdx];
          
          while (mask) {
            if (!(element & mask))
              return ((eIdx * _BitmapBitsPerElement) + bIdx);
            mask <<= 1;
          }
          bIdx = 0;
          eIdx++;
        }
      }
    }
    return (unsigned)-1;
  }
