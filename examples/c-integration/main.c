//Standard includes
#include <stdio.h>
#include <math.h>

//Integrand includes (for M_PI only)
#include "integrands.h"

//GSL integration includes
#ifdef HAVE_GSL
#include "plain_integrate_unit_cylinder.h"
#include "plain_integrate_random_walk.h"
// #include "miser_integrate_unit_cylinder.h"
// #include "miser_integrate_random_walk.h"
// #include "vegas_integrate_unit_cylinder.h"
// #include "vegas_integrate_random_walk.h"
#endif

#ifdef HAVE_OPENCL
//OpenCL integration includes
// #include "opencl_integrate_unit_cylinder.h"
// #include "opencl_integrate_random_walk.h"
#endif

void run_unit_cylinder_integral(
    const char *name,
    float (*integral)(float, float, float, float, float *)
    )
{
    printf("Running %s:\n", name);
    float result, error;
    result = integral(-1.0, 1.0, -1.0, 1.0, &error);
    printf("Result: %f\n", result);
    printf("Error:  %f\n", error);
    printf("Actual: %f\n", M_PI);
    printf("Diff:   %f (%f sigma)\n", 
           result - M_PI, 
           fabs(result - M_PI)/error);
}


int main()
{
    #ifdef HAVE_GSL
    run_unit_cylinder_integral("GSL Plain Unit Cylinder", 
                               &plain_integrate_unit_cylinder);

    #endif

    return 0;
}
