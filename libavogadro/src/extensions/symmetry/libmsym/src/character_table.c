//
//  character_table.c
//  libmsym
//
//  Created by Marcus Johansson on 28/11/14.
//  Copyright (c) 2014 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#include <stdlib.h>
#include <math.h>

#include "character_table.h"
#include "point_group.h"

msym_error_t characterTableUnknown(int n, CharacterTable *ct){
    msymSetErrorDetails("Character table unknown");
    return MSYM_INVALID_CHARACTER_TABLE;
}

msym_error_t characterTableTd(int n, CharacterTable *ct){
    ct->l = sizeof(TdIrrep)/sizeof(TdIrrep[0]);
    ct->irrep = malloc(ct->l*sizeof(IrreducibleRepresentation));
    for(int i = 0; i < ct->l;i++){
        enum IrreducibleRepresentationEnum irrep = TdIrrep[i];
        ct->irrep[i].name = IrreducibleRepresentationName[irrep];
        ct->irrep[i].v = TdTable[irrep];
        ct->irrep[i].d = Degeneracy[irrep];
        ct->irrep[i].l = sizeof(TdTable[irrep])/sizeof(TdTable[irrep][0]);
    }
    return MSYM_SUCCESS;
}

msym_error_t characterTableIh(int n, CharacterTable *ct){
    ct->l = sizeof(IhIrrep)/sizeof(IhIrrep[0]);
    ct->irrep = malloc(ct->l*sizeof(IrreducibleRepresentation));
    for(int i = 0; i < ct->l;i++){
        enum IrreducibleRepresentationEnum irrep = IhIrrep[i];
        ct->irrep[i].name = IrreducibleRepresentationName[irrep];
        ct->irrep[i].v = IhTable[irrep];
        ct->irrep[i].d = Degeneracy[irrep];
        ct->irrep[i].l = sizeof(IhTable[irrep])/sizeof(IhTable[irrep][0]);
    }
    return MSYM_SUCCESS;
}

msym_error_t characterTableCnv(int n, CharacterTable *ct){
    msym_error_t ret = MSYM_SUCCESS;
    switch(n) {
        case 3 : {
            ct->l = sizeof(C3vIrrep)/sizeof(C3vIrrep[0]);
            ct->irrep = malloc(ct->l*sizeof(IrreducibleRepresentation));
            for(int i = 0; i < ct->l;i++){
                enum IrreducibleRepresentationEnum irrep = C3vIrrep[i];
                ct->irrep[i].name = IrreducibleRepresentationName[irrep];
                ct->irrep[i].v = C3vTable[irrep];
                ct->irrep[i].d = Degeneracy[irrep];
                ct->irrep[i].l = sizeof(C3vTable[irrep])/sizeof(C3vTable[irrep][0]);
            }
            break;
        }
        case 4 : {
            ct->l = sizeof(C4vIrrep)/sizeof(C4vIrrep[0]);
            ct->irrep = malloc(ct->l*sizeof(IrreducibleRepresentation));
            for(int i = 0; i < ct->l;i++){
                enum IrreducibleRepresentationEnum irrep = C4vIrrep[i];
                ct->irrep[i].name = IrreducibleRepresentationName[irrep];
                ct->irrep[i].v = C4vTable[irrep];
                ct->irrep[i].d = Degeneracy[irrep];
                ct->irrep[i].l = sizeof(C4vTable[irrep])/sizeof(C4vTable[irrep][0]);
            }
            break;
        }

        default:
            msymSetErrorDetails("Cannot find C%dv character table",n);
            ret = MSYM_INVALID_CHARACTER_TABLE;
    }
    
    return ret;
}

msym_error_t characterTableCnh(int n, CharacterTable *ct){
    msym_error_t ret = MSYM_SUCCESS;
    switch(n) {
        case 2 : {
            ct->l = sizeof(C2hIrrep)/sizeof(C2hIrrep[0]);
            ct->irrep = malloc(ct->l*sizeof(IrreducibleRepresentation));
            for(int i = 0; i < ct->l;i++){
                enum IrreducibleRepresentationEnum irrep = C2hIrrep[i];
                ct->irrep[i].name = IrreducibleRepresentationName[irrep];
                ct->irrep[i].v = C2hTable[irrep];
                ct->irrep[i].d = Degeneracy[irrep];
                ct->irrep[i].l = sizeof(C2hTable[irrep])/sizeof(C2hTable[irrep][0]);
            }
            break;
        }
            
        default:
            msymSetErrorDetails("Cannot find C%dh character table",n);
            ret = MSYM_INVALID_CHARACTER_TABLE;
    }
    
    return ret;
}

msym_error_t characterTableDnh(int n, CharacterTable *ct){
    msym_error_t ret = MSYM_SUCCESS;
    switch(n) {
        case 2 : {
            ct->l = sizeof(D2hIrrep)/sizeof(D2hIrrep[0]);
            ct->irrep = malloc(ct->l*sizeof(IrreducibleRepresentation));
            for(int i = 0; i < ct->l;i++){
                enum IrreducibleRepresentationEnum irrep = D2hIrrep[i];
                ct->irrep[i].name = IrreducibleRepresentationName[irrep];
                ct->irrep[i].v = D2hTable[irrep];
                ct->irrep[i].d = Degeneracy[irrep];
                ct->irrep[i].l = sizeof(D2hTable[irrep])/sizeof(D2hTable[irrep][0]);
            }
            break;
        }
        case 4 : {
            ct->l = sizeof(D4hIrrep)/sizeof(D4hIrrep[0]);
            ct->irrep = malloc(ct->l*sizeof(IrreducibleRepresentation));
            for(int i = 0; i < ct->l;i++){
                enum IrreducibleRepresentationEnum irrep = D4hIrrep[i];
                ct->irrep[i].name = IrreducibleRepresentationName[irrep];
                ct->irrep[i].v = D4hTable[irrep];
                ct->irrep[i].d = Degeneracy[irrep];
                ct->irrep[i].l = sizeof(D4hTable[irrep])/sizeof(D4hTable[irrep][0]);
            }
            break;
        }
        case 6 : {
            ct->l = sizeof(D6hIrrep)/sizeof(D6hIrrep[0]);
            ct->irrep = malloc(ct->l*sizeof(IrreducibleRepresentation));
            for(int i = 0; i < ct->l;i++){
                enum IrreducibleRepresentationEnum irrep = D6hIrrep[i];
                ct->irrep[i].name = IrreducibleRepresentationName[irrep];
                ct->irrep[i].v = D6hTable[irrep];
                ct->irrep[i].d = Degeneracy[irrep];
                ct->irrep[i].l = sizeof(D6hTable[irrep])/sizeof(D6hTable[irrep][0]);
            }
            break;
        }
        default:
            msymSetErrorDetails("Cannot find D%dh character table",n);
            ret = MSYM_INVALID_CHARACTER_TABLE;
    }
    return ret;
}

void printCharacterTable(CharacterTable *ct){
    printf("Character Table:\n");
    for(int i = 0; i < ct->l;i++){
        printf("\t %d%s",ct->classc[i],ct->name[i]);
    }
    printf("\n");
    for(int i = 0; i < ct->l;i++){
        printf("%s:\t",ct->irrep[i].name);
        for(int j = 0; j < ct->irrep[i].l; j++){
            char *pre = signbit(ct->irrep[i].v[j]) == 1 ? "" : " ";
            printf("%s%.3lf\t",pre,ct->irrep[i].v[j]);
        }
        printf("\n");
    
    }
}
