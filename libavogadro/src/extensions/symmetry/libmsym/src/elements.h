//
//  elements.h
//  libmsym
//
//  Created by Marcus Johansson on 17/02/15.
//  Copyright (c) 2015 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM__ELEMENTS_h
#define __MSYM__ELEMENTS_h

#include <stdio.h>

#include "msym.h"

void printElement(msym_element_t *element);
msym_error_t complementElementData(msym_element_t *element);

#endif /* defined(__MSYM__ELEMENTS_h) */
