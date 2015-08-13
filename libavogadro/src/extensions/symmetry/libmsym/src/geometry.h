//
//  geometry.h
//  libmsym
//
//  Created by Marcus Johansson on 28/11/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM__GEOMETRY_h
#define __MSYM__GEOMETRY_h

#include <stdio.h>
#include "msym.h"

msym_error_t findGeometry(int length, msym_element_t *elements[length], double cm[3], msym_thresholds_t *thresholds, msym_geometry_t *g, double e[3], double v[3][3]);
msym_error_t findCenterOfMass(int length, msym_element_t *elements[length], double v[3]);
int geometryDegenerate(msym_geometry_t g);
void printGeometry(msym_geometry_t g);

#endif /* defined(__MSYM__GEOMETRY_h) */
