//
//  elements.c
//  libmsym
//
//  Created by Marcus Johansson on 17/02/15.
//  Copyright (c) 2015 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#include <string.h>
#include <math.h>

#include "elements.h"


const struct _periodic_table {
    int n;
    char *name;
    int massnr;
} periodic_table[] = {
    { 1, "H", 1 },
    { 1, "D", 2 },
    { 1, "T", 3 },
    { 2, "He", 4 },
    { 3, "Li", 7 },
    { 4, "Be", 9 },
    { 5, "B", 11 },
    { 6, "C", 12 },
    { 7, "N", 14 },
    { 8, "O", 16 },
    { 9, "F", 19 },
    { 10, "Ne", 20 },
    { 11, "Na", 23 },
    { 12, "Mg", 24 },
    { 13, "Al", 27 },
    { 14, "Si", 28 },
    { 15, "P", 31 },
    { 16, "S", 32 },
    { 17, "Cl", 35 },
    { 18, "Ar", 40 },
    { 19, "K", 30 },
    { 20, "Ca", 40 },
    { 21, "Sc", 45 },
    { 22, "Ti", 48 },
    { 23, "V", 51 },
    { 24, "Cr", 52 },
    { 25, "Mn", 55 },
    { 26, "Fe", 56 },
    { 27, "Co", 58 },
    { 28, "Ni", 58 },
    { 29, "Cu", 64 },
    { 30, "Zn", 65 },
    { 31, "Ga", 70 },
    { 32, "Ge", 73 },
    { 33, "As", 75 },
    { 34, "Se", 79 },
    { 35, "Br", 80 },
    { 36, "Kr", 84 },
    { 37, "Rb", 85 },
    { 38, "Sr", 88 },
    { 39, "Y", 89 },
    { 40, "Zr", 91 },
    { 41, "Nb", 93 },
    { 42, "Mo", 96 },
    { 43, "Tc", 98 },
    { 44, "Ru", 101 },
    { 45, "Rh", 103 },
    { 46, "Pd", 106 },
    { 47, "Ag", 108 },
    { 48, "Cd", 112 },
    { 49, "In", 115 },
    { 50, "Sn", 119 },
    { 51, "Sb", 122 },
    { 52, "Te", 128 },
    { 53, "I", 127 },
    { 54, "Xe", 131 },
    { 55, "Cs", 133 },
    { 56, "Ba", 137 },
    { 57, "La", 139 },
    { 58, "Ce", 140 },
    { 59, "Pr", 141 },
    { 60, "Nd", 144 },
    { 61, "Pm", 145 },
    { 62, "Sm", 150 },
    { 63, "Eu", 152 },
    { 64, "Gd", 157 },
    { 65, "Tb", 159 },
    { 66, "Dy", 163 },
    { 67, "Ho", 165 },
    { 68, "Er", 167 },
    { 69, "Tm", 169 },
    { 70, "Yb", 173 },
    { 71, "Lu", 175 },
    { 72, "Hf", 178 },
    { 73, "Ta", 181 },
    { 74, "W", 184 },
    { 75, "Re", 186 },
    { 76, "Os", 190 },
    { 77, "Ir", 192 },
    { 78, "Pt", 195 },
    { 79, "Au", 197 },
    { 80, "Hg", 201 },
    { 81, "Tl", 204 },
    { 82, "Pb", 207 },
    { 83, "Bi", 209 },
    { 84, "Po", 209 },
    { 85, "At", 210 },
    { 86, "Rn", 222 },
    { 87, "Fr", 223 },
    { 88, "Ra", 226 },
    { 89, "Ac", 227 },
    { 90, "Th", 232 },
    { 91, "Pa", 231 },
    { 92, "U", 238 },
    { 93, "Np", 237 },
    { 94, "Pu", 244 },
    { 95, "Am", 243 },
    { 96, "Cm", 247 },
    { 97, "Bk", 247 },
    { 98, "Cf", 251 },
    { 99, "Es", 252 },
    { 100, "Fm", 257 },
    { 101, "Md", 258 },
    { 102, "No", 259 },
    { 103, "Lr", 262 },
    { 104, "Rf", 261 },
    { 105, "Db", 268 },
    { 106, "Sg", 263 },
    { 107, "Bh", 264 },
    { 108, "Hs", 269 },
    { 109, "Mt", 268 },
    { 110, "Ds", 272 },
    { 111, "Rg", 273 },
    { 112, "Uub", 277 },
    { 113, "Uut", 286 },
    { 114, "Uuq", 289 },
    { 115, "Uup", 288 },
    { 116, "Uuh", 292 },
    { 117, "Uus", 292 },
    { 118, "Uuo", 293 }
};

void printElement(msym_element_t *element){
    printf("%s (nuclear charge:%d, mass:%lf) [%lf;%lf;%lf]\n",element->name, element->n, element->m, element->v[0], element->v[1], element->v[2]);
}

msym_error_t complementElementData(msym_element_t *element){
    msym_error_t ret = MSYM_SUCCESS;
    size_t strl = strnlen(element->name, sizeof(element->name));
    if(strl <= 0 && element->n <= 0 && element->m <= 0.0){
        msymSetErrorDetails("Element has no mass, name or nuclear charge");
        ret = MSYM_INVALID_ELEMENTS;
        goto err;
    }
    if(element->n >= sizeof(periodic_table)/sizeof(periodic_table[0]) && element->m <= 0.0 && strl <= 0){
        msymSetErrorDetails("Element has no mass or name and a nuclear charge of %d",element->n);
        ret = MSYM_INVALID_ELEMENTS;
        goto err;
    }
    
    if(element->n > 0 && (element->m <= 0.0 || strl <= 0)){
        int fi, fil = sizeof(periodic_table)/sizeof(periodic_table[0]);
        for(fi = 0; fi < fil;fi++){
            if(periodic_table[fi].n == element->n) {
                if(element->m <= 0.0) element->m = (double) periodic_table[fi].massnr;
                if(strl <= 0) snprintf(element->name, sizeof(element->name), "%s",periodic_table[fi].name);
                break;
            }
        }
        
        if(fi == fil){
            msymSetErrorDetails("Unknown element with nuclear charge %d",fi);
            ret = MSYM_INVALID_ELEMENTS;
            goto err;
        }
    } else if(strl > 0 && (element->m <= 0.0 || element->n <= 0)){
        int fi, fil = sizeof(periodic_table)/sizeof(periodic_table[0]);
        for(fi = 0; fi < fil;fi++){
            if(0 == strncmp(periodic_table[fi].name, element->name, strnlen(periodic_table[fi].name, sizeof(element->name)))) {
                if(element->m <= 0.0) element->m = (double) periodic_table[fi].massnr;
                if(element->n <= 0) element->n = periodic_table[fi].n;
                break;
            }
        }
        
        if(fi == fil){
            char buf[sizeof(element->name)];
            snprintf(buf, sizeof(element->name), "%s",element->name); //in case someone forgets to null terminate
            msymSetErrorDetails("Unknown element with name %s",buf);
            ret = MSYM_INVALID_ELEMENTS;
            goto err;
        }
    } else if(element->m > 0.0 && (strl <= 0 || element->n <= 0)){
        int fi, fil = sizeof(periodic_table)/sizeof(periodic_table[0]);
        double last = -1.0;
        for(fi = 0; fi < fil;fi++){
            double diff = fabs(periodic_table[fi].massnr - element->m);
            if(diff < last || last < 0.0){
                last = diff;
            } else {
                if(strl <= 0) snprintf(element->name, sizeof(element->name), "%s",periodic_table[fi].name);
                if(element->n <= 0) element->n = periodic_table[fi].n;
                break;
            }
        }
        
        if(fi == fil){
            msymSetErrorDetails("Cannot determine element from mass %lf",element->m); //This won't happen, but may want to have some other checking later
            ret = MSYM_INVALID_ELEMENTS;
            goto err;
        }
    }
    
err:
    return ret;
    
}
