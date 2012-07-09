//Standard includes
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>

//Integrand includes (for M_PI only)
#include "integrands.h"

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
// #include "opencl_integrate_unit_cylinder.h"
// #include "opencl_integrate_random_walk.h"
#endif

bool diff_timevals(struct timeval *result, 
                   struct timeval *end, 
                   struct timeval *start)
{
    //Perform the carrstart for the later subtraction bstart updating start.
    if(end->tv_usec < start->tv_usec)
    {
        int nsec = (start->tv_usec - end->tv_usec) / 1000000 + 1;
        start->tv_usec -= 1000000 * nsec;
        start->tv_sec += nsec;
    }
    if(end->tv_usec - start->tv_usec > 1000000)
    {
        int nsec = (end->tv_usec - start->tv_usec) / 1000000;
        start->tv_usec += 1000000 * nsec;
        start->tv_sec -= nsec;
    }

    //Compute the time remaining to wait. tv_usec is certainly positive.
    result->tv_sec = end->tv_sec - start->tv_sec;
    result->tv_usec = end->tv_usec - start->tv_usec;

    //Return 1 if result is negative.
    return end->tv_sec < start->tv_sec;
}

void run_unit_cylinder_integral(
    const char *name,
    float (*integral)(float, float, float, float, float *)
    )
{
    printf("Running %s:\n", name);
    float result, error;
    struct timeval start, stop, diff;
    struct timezone tz;
    gettimeofday(&start, &tz);
    result = integral(-1.0, 1.0, -1.0, 1.0, &error);
    gettimeofday(&stop, &tz);
    diff_timevals(&diff, &stop, &start);
    printf("Time:   %f seconds\n", 
           ((double)diff.tv_sec) + (((double)diff.tv_usec)/1000000.0));
    printf("Result: %f\n", result);
    printf("Error:  %f\n", error);
    printf("Actual: %f\n", M_PI);
    printf("Diff:   %f (%f sigma)\n\n", 
           result - M_PI, 
           fabs(result - M_PI)/error);
}

void run_random_walk_integral(
    const char *name,
    double (*integral)(float, float, float, float, float, float, double *)
    )
{
    printf("Running %s:\n", name);
    const double exact = 1.3932039296856768591842462603255;
    double result, error;
    struct timeval start, stop, diff;
    struct timezone tz;
    gettimeofday(&start, &tz);
    result = integral(-M_PI, M_PI, -M_PI, M_PI, -M_PI, M_PI, &error);
    gettimeofday(&stop, &tz);
    diff_timevals(&diff, &stop, &start);
    printf("Time:   %f seconds\n", 
           ((double)diff.tv_sec) + (((double)diff.tv_usec)/1000000.0));
    printf("Result: %f\n", result);
    printf("Error:  %f\n", error);
    printf("Actual: %f\n", exact);
    printf("Diff:   %f (%f sigma)\n\n", 
           result - exact, 
           fabs(result - exact)/error);
}

int main()
{
    #ifdef HAVE_GSL
    run_unit_cylinder_integral("GSL Plain Unit Cylinder", 
                               &plain_integrate_unit_cylinder);
    run_random_walk_integral("GSL Plain Random Walk", 
                             &plain_integrate_random_walk);
    run_unit_cylinder_integral("GSL Miser Unit Cylinder", 
                               &miser_integrate_unit_cylinder);
    run_random_walk_integral("GSL Miser Random Walk", 
                             &miser_integrate_random_walk);
    run_unit_cylinder_integral("GSL Vegas Unit Cylinder", 
                               &vegas_integrate_unit_cylinder);
    run_random_walk_integral("GSL Vegas Random Walk", 
                             &vegas_integrate_random_walk);
    #endif

    return 0;
}
