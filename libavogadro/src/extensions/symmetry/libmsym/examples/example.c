//
//  example.c
//  libmsym
//
//  Created by Marcus Johansson on 24/04/15.
//  Copyright (c) 2015 Marcus Johansson. 
//
//  Distributed under the MIT License ( See LICENSE file or copy at http://opensource.org/licenses/MIT )
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msym.h"

int example(const char* in_file);
int read_xyz(const char *name, msym_element_t **ratoms);

int main(int argc, const char * argv[]) {
    int ret = 1;
    if(argc == 2){
        ret = example(argv[1]);
        fflush(stdout);
    } else {
        printf("usage msym_example <xyz-file>");
    }
    return ret;
}

int example(const char* in_file){
    msym_error_t ret = MSYM_SUCCESS;
    msym_element_t *elements = NULL;
    msym_orbital_t *orbitals = NULL, **porbitals = NULL;
    
    const char *error = NULL;
    char point_group[6];
    double cm[3], radius = 0.0, symerr = 0.0;
    
    /* Do not free these variables */
    msym_element_t *melements = NULL;
    msym_symmetry_operation_t *msops = NULL;
    msym_subgroup_t *msg = NULL;
    int msgl = 0, msopsl = 0, mlength = 0;
    
    /* This function reads xyz files.
     * It initializes an array of msym_element_t to 0,
     * then sets the coordinates and name of the elements */
    int length = read_xyz(in_file, &elements);
    if(length <= 0) return -1;
    
    double (*coefficients)[length] = NULL;
    
    /* Allocate and initialize memory for orbitals */
    orbitals = calloc(length, sizeof(msym_orbital_t));
    porbitals = calloc(length, sizeof(msym_orbital_t*));
    
    /* Add a 1s orbital to each atom */
    for(int i = 0;i < length;i++){
        /* You can also just set orbitals[i].n = 1 */
        snprintf(orbitals[i].name,sizeof(orbitals[i].name),"1s");
        porbitals[i] = &orbitals[i];
        elements[i].ao = &porbitals[i];
        elements[i].aol = 1;
    }
    
    /* Create a context */
    msym_context ctx = msymCreateContext();
    
    /* Use default thresholds otherwise call:
     * msymSetThresholds(msym_context ctx, msym_thresholds_t *thresholds); */
    
    /* Set elements and orbitals */
    if(MSYM_SUCCESS != (ret = msymSetElements(ctx, length, elements))) goto err;
    
    /* These are no longer needed, internal versions of these are kept in the context,
     * They are indexed in the same way that they have been allocated.
     * I.e. during orbital symmetrization or when getting the symmetrized LCAO,
     * the coefficients will correspond to the same indexing as "orbitals",
     * this is the main reason for the two levels of indirection */
    free(elements);  elements = NULL;
    free(orbitals);  orbitals = NULL;
    free(porbitals); porbitals = NULL;
    
    /* Some trivial information */
    if(MSYM_SUCCESS != (ret = msymGetCenterOfMass(ctx,cm))) goto err;
    if(MSYM_SUCCESS != (ret = msymGetRadius(ctx,&radius))) goto err;
    
    printf("Molecule has center of mass [%lf; %lf; %lf] "
           "and a radius of %lf\n",cm[0],cm[1],cm[2],radius);
    
    /* Find molecular symmetry */
    if(MSYM_SUCCESS != (ret = msymFindSymmetry(ctx))) goto err;
    
    /* Get the point group name */
    if(MSYM_SUCCESS != (ret = msymGetPointGroup(ctx, sizeof(char[6]), point_group))) goto err;
    if(MSYM_SUCCESS != (ret = msymGetSubgroups(ctx, &msgl, &msg))) goto err;
    printf("Found point group [0] %s select subgroup\n",point_group);
    for(int i = 0; i < msgl;i++) printf("\t [%d] %s\n",i+1,msg[i].name);
    int ssg = 0;
    printf("\nEnter point group[0-%d]:",msgl);
    while(scanf("%d", &ssg) <= 0 && ssg >= 0 && ssg <= msgl) printf("\nEnter point group[0-%d]:",msgl);
    if(ssg > 0){
        ssg--;
        printf("Selected point group %s\n",msg[ssg].name);
        if(MSYM_SUCCESS != (ret = msymSelectSubgroup(ctx, &msg[ssg]))) goto err;
        /* Retreive the symmetry operations again.
         * Everything has been rebuilt, and the old msops is no longer valid
         * Neither are the equivalence sets
         */
        if(MSYM_SUCCESS != (ret = msymGetSymmetryOperations(ctx, &msopsl, &msops))) goto err;
        
    }
    
    /* Set pointgroup to the D2h subgroup if it has Th symmetry
     * using the same alignment as the original.
     * If specific axes are wanted the alignment axes can be set instead
     * And of course you can keep Th if you want =D */
    if(0 == strncmp(point_group, "Ih", 2) && ssg == 0){
        double transform[3][3];
        printf("Changing pointgroup from Th -> D2h\n");
        if(MSYM_SUCCESS != (ret = msymGetAlignmentTransform(ctx, transform))) goto err;
        if(MSYM_SUCCESS != (ret = msymSetPointGroup(ctx, "D2h"))) goto err;
        if(MSYM_SUCCESS != (ret = msymSetAlignmentTransform(ctx, transform))) goto err;
        if(MSYM_SUCCESS != (ret = msymFindSymmetry(ctx))) goto err;
        if(MSYM_SUCCESS != (ret = msymGetPointGroup(ctx, sizeof(char[6]), point_group))) goto err;
    }
    
    /* Retreive the symmetry operations */
    if(MSYM_SUCCESS != (ret = msymGetSymmetryOperations(ctx, &msopsl, &msops))) goto err;
    
    for(int i = 0; i < msopsl;i++){
        if(msops[i].type == PROPER_ROTATION && msops[i].order == 3 && msops[i].power == 1){
            printf("Found a C3^1 axis, YEY!\n");
        }
    }
    
    coefficients = malloc(sizeof(double[length][length]));
    
    /* Get the subspaces for this point group (in order of irreducible representation) */
    if(MSYM_SUCCESS != (ret = msymGetOrbitalSubspaces(ctx,length,coefficients))) goto err;
    
    /* Mess them up a bit */
    for(int i = 0;i < length;i++){
        for(int j = 0;j < length;j++){
            coefficients[i][j] += i*0.001/length - j*0.001/length;
        }
    }
    
    /* And symmetrize them */
    if(MSYM_SUCCESS != (ret = msymSymmetrizeOrbitals(ctx,length,coefficients))) goto err;

    free(coefficients); coefficients = NULL;
    
    /* Aligning axes prior to orbital symmetrization will
     * change the orientation of orbitals with l >= 1 */
    if(MSYM_SUCCESS != (ret = msymAlignAxes(ctx))) goto err;
    
    /* Symmetrize the molecule.
     * You can do this before orbital symmetrization as well,
     * but the permutations are already built, so you don't need to */
    if(MSYM_SUCCESS != (ret = msymSymmetrizeMolecule(ctx, &symerr))) goto err;
    
    printf("Molecule has been symmetrized to point group %s "
           "with an error of %lf\n",point_group, symerr);
    
    if(MSYM_SUCCESS != (ret = msymGetElements(ctx, &mlength, &melements))) goto err;
    if(mlength != length){ printf("Not possible!\n"); goto err;}
    
    
    
    printf("New element coordinates:\n%d\n\n",mlength);
    for(int i = 0;i < mlength;i++){
        printf("%s %12.9lf %12.9lf %12.9lf\n",
               melements[i].name,
               melements[i].v[0],
               melements[i].v[1],
               melements[i].v[2]);
    }
    
    /* Make a new element with the same type as the first one we read */
    msym_element_t myelement;
    memset(&myelement,0,sizeof(msym_element_t));
    myelement.n = melements[0].n;
    myelement.v[0] = melements[0].v[0];
    myelement.v[1] = melements[0].v[1];
    myelement.v[2] = melements[0].v[2];
    
    /* Generate some new elements of the same point group */
    if(MSYM_SUCCESS != (ret = msymGenerateElements(ctx,1,&myelement))) goto err;
    
    /* This is not a memory leak, context keeps track of melements,
     * and it should never be freed, msymReleaseContext does this. */
    if(MSYM_SUCCESS != (ret = msymGetElements(ctx, &mlength, &melements))) goto err;
    
    printf("Generated element coordinates:\n%d\n\n",mlength);
    for(int i = 0;i < mlength;i++){
        printf("%s %lf %lf %lf\n",
               melements[i].name,
               melements[i].v[0],
               melements[i].v[1],
               melements[i].v[2]);
    }
    
    msymReleaseContext(ctx);
    printf("We're done!\n");
    
    return ret;
err:
    free(elements);
    free(orbitals);
    free(porbitals);
    free(coefficients);
    error = msymErrorString(ret);
    fprintf(stderr,"Error %s: ",error);
    error = msymGetErrorDetails();
    fprintf(stderr,"%s\n",error);
    return ret;
}

int read_xyz(const char *name, msym_element_t **ratoms) {
    FILE *fp = fopen(name,"r");
    msym_element_t *a;
    int l;
    size_t cl;
    char buf[1024];
    if (NULL == fgets(buf, sizeof(buf), fp) || sscanf(buf," %d ",&l) != 1){
        fprintf(stderr,"Unable to read file %s\n",name);
        return -1;
    }
    if(l < 300000) {
        a = malloc(l*sizeof(msym_element_t));
        memset(a,0,l*sizeof(msym_element_t));
    } else {
        fprintf(stderr, "Too many elements in file %d\n",l);
        return -1;
    }
    
    //char * fgets ( comment, sizeof(comment), fp );
    if(NULL != fgets(buf, sizeof(buf), fp)){
        printf("Comment: %.*s", sizeof(buf), buf);
    }
    
    for (int i = 0; fgets(buf, sizeof(buf), fp) && sscanf(buf, "%s %lf %lf %lf", a[i].name, &(a[i].v[0]),  &(a[i].v[1]),  &(a[i].v[2])) == 4 && i < l; i++) {}
    *ratoms = a;
    return l;
    
}
