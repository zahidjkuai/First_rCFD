/******************************************************************************
 * @internal
 * @file        rCFD_memory.h
 * @copyright   (C) 2022
 *              Particulate Flow Modelling,
 *              Johannes Kepler University, Linz, Austria
 * @website     www.particulate-flow.at
 * @authors     Daniel Queteschiner
 *****************************************************************************/

#ifndef RCFD_MEMORY
#define RCFD_MEMORY

#include <stdlib.h>

#if defined(RCFD_SAFE_MALLOC)
#define smalloc     s_malloc
#define srealloc    s_realloc
#else
#define smalloc     malloc
#define srealloc    realloc
#endif

// safe malloc
void *s_malloc(int nbytes)
{
    if(nbytes <= 0){
        return NULL;
    }
    return malloc(nbytes);
}

// safe realloc
void *s_realloc(void *ptr, int nbytes)
{
    if(nbytes <= 0) {
        free(ptr);
        return NULL;
    }

    void *ptr1 = realloc(ptr, nbytes);

    if(ptr1 == NULL){
        free(ptr);
    }

    return ptr1;
}


// 2d int array
int **malloc_i_2d(int n1, int n2)
{
    int i, n = 0;
    int **array;
    int nbytes = (int)(sizeof(int) * n1 * n2);
    int *data = (int*)smalloc(nbytes);
    nbytes = (int)(sizeof(int*) * n1);
    array = (int**)smalloc(nbytes);

    for(i = 0; i < n1; ++i){
        array[i] = &data[n];
        n += n2;
    }
    return array;
}

// n2 must be the same as in original malloc for data consistency
int **realloc_i_2d(int **array, int n1, int n2)
{
    if(array == NULL){
        return malloc_i_2d(n1, n2);
    }

    int i, n = 0;
    int nbytes = (int)(sizeof(int) * n1 * n2);
    int *data = (int*)srealloc(array[0], nbytes);
    nbytes = (int)(sizeof(int*) * n1);
    array = (int**)srealloc(array, nbytes);

    for (i = 0; i < n1; ++i) {
      array[i] = &data[n];
      n += n2;
    }
    return array;
}

void free_i_2d(int **array)
{
    if (array == NULL) return;
    free(array[0]);
    free(array);
}

// 2d double array
double **malloc_r_2d(int n1, int n2)
{
    int i, n = 0;
    double **array;
    int nbytes = (int)(sizeof(double) * n1 * n2);
    double *data = (double*)smalloc(nbytes);
    nbytes = (int)(sizeof(double*) * n1);
    array = (double**)smalloc(nbytes);

    for(i = 0; i < n1; ++i){
        array[i] = &data[n];
        n += n2;
    }
    return array;
}

double **realloc_r_2d(double **array, int n1, int n2)
{
    if(array == NULL){
        return malloc_r_2d(n1, n2);
    }

    int i, n = 0;
    int nbytes = (int)(sizeof(double) * n1 * n2);
    double *data = (double*)srealloc(array[0], nbytes);
    nbytes = (int)(sizeof(double*) * n1);
    array = (double**)srealloc(array, nbytes);

    for (i = 0; i < n1; ++i) {
      array[i] = &data[n];
      n += n2;
    }
    return array;
}

void free_r_2d(double **array)
{
    if (array == NULL) return;
    free(array[0]);
    free(array);
}

// 3d int array
int ***malloc_i_3d(int n1, int n2, int n3)
{
    int i, j, m, n = 0;
    int **plane;
    int ***array;
    int nbytes = (int)(sizeof(int) * n1 * n2 * n3);
    int *data = (int*)smalloc(nbytes);
    nbytes = (int)(sizeof(int*) * n1 * n2);
    plane = (int**)smalloc(nbytes);
    nbytes = (int)(sizeof(int**) * n1);
    array = (int***)smalloc(nbytes);

    for(i = 0; i < n1; ++i){
        m = i * n2;
        array[i] = &plane[m];
        for(j = 0; j < n2; ++j){
            plane[m+j] = &data[n];
            n += n3;
        }
    }
    return array;
}

void free_i_3d(int ***array)
{
    if (array == NULL) return;
    free(array[0][0]);
    free(array[0]);
    free(array);
}

// 3d double array
double ***malloc_r_3d(int n1, int n2, int n3)
{
    int i, j, m, n = 0;
    double **plane;
    double ***array;
    int nbytes = (int)(sizeof(double) * n1 * n2 * n3);
    double *data = (double*)smalloc(nbytes);
    nbytes = (int)(sizeof(double*) * n1 * n2);
    plane = (double**)smalloc(nbytes);
    nbytes = (int)(sizeof(double**) * n1);
    array = (double***)smalloc(nbytes);

    for(i = 0; i < n1; ++i){
        m = i * n2;
        array[i] = &plane[m];
        for(j = 0; j < n2; ++j){
            plane[m+j] = &data[n];
            n += n3;
        }
    }
    return array;
}

void free_r_3d(double ***array)
{
    if (array == NULL) return;
    free(array[0][0]);
    free(array[0]);
    free(array);
}

// 4d int array
int ****malloc_i_4d(int n1, int n2, int n3, int n4)
{
    int i, j, k;
    int m1, m2, n = 0;
    int **cube;
    int ***plane;
    int ****array;
    int nbytes = (int)(sizeof(int) * n1 * n2 * n3 * n4);
    int *data = (int*)smalloc(nbytes);
    nbytes = (int)(sizeof(int*)) * n1 * n2 * n3;
    cube   = (int**)smalloc(nbytes);
    nbytes = (int)(sizeof(int**)) * n1 * n2;
    plane  = (int***)smalloc(nbytes);
    nbytes = (int)(sizeof(int***)) * n1;
    array  = (int****)smalloc(nbytes);

    for(i = 0; i < n1; ++i){
        m1 = i * n2;
        array[i] = &plane[m1];
        for(j = 0; j < n2; ++j){
            m2 = i * n2 * n3 + j * n3;
            plane[m1+j] = &cube[m2];
            for(k = 0; k < n3; ++k){
                cube[m2+k] = &data[n];
                n += n4;
            }
        }
    }
    return array;
}

void free_i_4d(int ****array)
{
    if (array == NULL) return;
    free(array[0][0][0]);
    free(array[0][0]);
    free(array[0]);
    free(array);
}

#if defined(RCFD_LIB_TEST_MODULES)
// test with valgrind
int test_memory(rCFD_type* rcfd)
{
    int success = 0;

    int n1=2, n2=3, n3=4, n4=5;
    int i, j, k, l;
    int **iarray2d = NULL;
    int ***iarray3d = NULL;
    int ****iarray4d = NULL;
    double **rarray2d = NULL;
    double ***rarray3d = NULL;

    // only test on process 0
    if(myid != 0)
        return success;

    Message0("\nStarting rCFD test 'memory'\n");

    // test free functions with NULL pointers
    free_i_2d(iarray2d);
    free_i_3d(iarray3d);
    free_i_4d(iarray4d);
    free_r_2d(rarray2d);
    free_r_3d(rarray3d);

    // test malloc functions
    iarray2d = malloc_i_2d(n1, n2);
    iarray3d = malloc_i_3d(n1, n2, n3);
    iarray4d = malloc_i_4d(n1, n2, n3, n4);
    rarray2d = malloc_r_2d(n1, n2);
    rarray3d = malloc_r_3d(n1, n2, n3);

    if(!iarray2d) success |= 1;
    if(!iarray3d) success |= 2;
    if(!iarray4d) success |= 4;
    if(!rarray2d) success |= 8;
    if(!rarray3d) success |= 16;

    // fill arrays
    for(i = 0; i < n1; ++i){
        for(j = 0; j < n2; ++j){
            if(iarray2d) iarray2d[i][j] = (i+1)*(j+1);
            if(rarray2d) rarray2d[i][j] = (i+1)*(j+1)*2.0;
            for(k = 0; k < n3; ++k){
                if(iarray3d) iarray3d[i][j][k] = (i+1)*(j+1)*(k+1);
                if(rarray3d) rarray3d[i][j][k] = (i+1)*(j+1)*(k+1)*2.0;
                for(l = 0; l < n4; ++l){
                    if(iarray4d) iarray4d[i][j][k][l] = (i+1)*(j+1)*(k+1)*(l+1);
                }
            }
        }
    }

    // print to screen
    if(iarray2d){
        for(i = 0; i < n1; ++i){
            for(j = 0; j < n2; ++j){
                Message0("iarray2d[%d][%d] = %6d ", i, j, iarray2d[i][j]);
            }
            Message0("\n");
        }
        Message0("\n");
    }

    if(rarray2d){
        for(i = 0; i < n1; ++i){
            for(j = 0; j < n2; ++j){
                Message0("rarray2d[%d][%d] = %8.2f ", i, j, rarray2d[i][j]);
            }
            Message0("\n");
        }
        Message0("\n");
    }

    if(iarray3d){
        for(i = 0; i < n1; ++i){
            for(j = 0; j < n2; ++j){
                for(k = 0; k < n3; ++k){
                    Message0("iarray3d[%d][%d][%d] = %6d ", i, j, k, iarray3d[i][j][k]);
                }
                Message0("\n");
            }
        }
        Message0("\n");
    }

    if(rarray3d){
        for(i = 0; i < n1; ++i){
            for(j = 0; j < n2; ++j){
                for(k = 0; k < n3; ++k){
                    Message0("rarray3d[%d][%d][%d] = %8.2f ", i, j, k, rarray3d[i][j][k]);
                }
                Message0("\n");
            }
        }
        Message0("\n");
    }
    if(iarray4d){
        for(i = 0; i < n1; ++i){
            for(j = 0; j < n2; ++j){
               for(k = 0; k < n3; ++k){
                    for(l = 0; l< n4; ++l){
                        Message0("iarray4d[%d][%d][%d][%d] = %6d ", i, j, k, l, iarray4d[i][j][k][l]);
                    }
                    Message0("\n");
                }
            }
        }
        Message0("\n");
    }

    // test realloc functions
    iarray2d = realloc_i_2d(iarray2d, n1+2, n2);
    rarray2d = realloc_r_2d(rarray2d, n1+2, n2);

    if(!iarray2d) success |= 32;
    if(!rarray2d) success |= 64;

    // fill arrays
    for(i = n1; i < n1+2; ++i){
        for(j = 0; j < n2; ++j){
            if(iarray2d) iarray2d[i][j] = (i+1)*(j+1);
            if(rarray2d) rarray2d[i][j] = (i+1)*(j+1)*2.0;
        }
    }

    // print to screen
    if(iarray2d){
        for(i = 0; i < n1+2; ++i){
            for(j = 0; j < n2; ++j){
                Message0("iarray2d[%d][%d] = %6d ", i, j, iarray2d[i][j]);
            }
            Message0("\n");
        }
        Message0("\n");
    }

    if(rarray2d){
        for(i = 0; i < n1+2; ++i){
            for(j = 0; j < n2; ++j){
                Message0("rarray2d[%d][%d] = %8.2f ", i, j, rarray2d[i][j]);
            }
            Message0("\n");
        }
        Message0("\n");
    }


    // test free functions with non-NULL pointers
    free_i_2d(iarray2d);
    free_i_3d(iarray3d);
    free_i_4d(iarray4d);
    free_r_2d(rarray2d);
    free_r_3d(rarray3d);

    return success;
}
#endif // RCFD_LIB_TEST_MODULES

#endif
