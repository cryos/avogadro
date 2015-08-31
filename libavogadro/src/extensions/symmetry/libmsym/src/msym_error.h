//
//  msym_error.h
//  libmsym
//
//  Created by Marcus Johansson on 30/01/15.
//  Copyright (c) 2015 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#ifndef __MSYM__ERROR_H
#define __MSYM__ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

    enum _msym_error {
        MSYM_SUCCESS = 0,
        MSYM_INVALID_INPUT = -1,
        MSYM_INVALID_CONTEXT = -2,
        MSYM_INVALID_THRESHOLD = -3,
        MSYM_INVALID_ELEMENTS = -4,
        MSYM_INVALID_ORBITALS = -5,
        MSYM_INVALID_POINT_GROUP = -6,
        MSYM_INVALID_EQUIVALENCE_SET = -7,
        MSYM_INVALID_PERMUTATION = -8,
        MSYM_INVALID_GEOMETRY = -9,
        MSYM_INVALID_CHARACTER_TABLE = -10,
        MSYM_INVALID_SUBSPACE = -11,
        MSYM_INVALID_AXES = -12,
        MSYM_SYMMETRY_ERROR = -13,
        MSYM_PERMUTATION_ERROR = -14,
        MSYM_POINT_GROUP_ERROR = -15,
        MSYM_SYMMETRIZATION_ERROR = -16,
        MSYM_ORBITAL_ERROR = -17
    };
    
    typedef enum _msym_error msym_error_t;
    
    const char *msymErrorString(msym_error_t error);
    void msymSetErrorDetails(const char *format, ...);
    const char *msymGetErrorDetails();
    
    
#ifdef __cplusplus
}
#endif

    
#endif /* defined(__MSYM__ERROR_H) */
    
