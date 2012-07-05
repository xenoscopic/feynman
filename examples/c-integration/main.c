//Standard includes
#include <stdio.h>

//GSL integration includes
#ifdef HAVE_GSL
#include "plain_integrate_unit_cylinder.h"
#include "plain_integrate_random_walk.h"
#include "miser_integrate_unit_cylinder.h"
#include "miser_integrate_random_walk.h"
#include "vegas_integrate_unit_cylinder.h"
#include "vegas_integrate_random_walk.h"
#endif

#ifdef HAVE_OPENCL
//OpenCL integration includes
#include "opencl_integrate_unit_cylinder.h"
#include "opencl_integrate_random_walk.h"
#endif

int main()
{
    //Print basic information
    int n_techniques = 0;
    int n_functions = 2;
    #ifdef HAVE_GSL
    n_techniques += 3;
    #endif
    #ifdef HAVE_OPENCL
    n_techniques += 1;
    #endif
    printf("Testing %i integration techniques on %i functions each.\n", 
           n_techniques, 
           n_functions);

    return 0;
}
