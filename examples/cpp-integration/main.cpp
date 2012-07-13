//Standard includes
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>

//Integrand includes (for M_PI only)
#include "sample_integrands.h"

//GSL integration includes
#ifdef HAVE_GSL
#include "unit_cylinder_gsl_integrator.h"
#include "random_walk_gsl_integrator.h"
#endif

//OpenCL integration includes
#ifdef HAVE_OPENCL
#include "unit_cylinder_opencl_integrator.h"
#include "random_walk_opencl_integrator.h"
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

template <typename T>
void run_unit_cylinder_integrator(const char *name, typename T::MonteCarloType mc_type)
{
    //Create the integrator
    T integral;
    integral.set_monte_carlo_type(mc_type);
    integral.set_n_calls(10000000);

    //Run the test
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

template <typename T>
void run_random_walk_integrator(const char *name, typename T::MonteCarloType mc_type)
{
    //Create the integrator
    T integral;
    integral.set_monte_carlo_type(mc_type);
    integral.set_n_calls(10000000);

    //Run the test
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
    //Run the unit cylinder integrations
    #ifdef HAVE_GSL
    run_unit_cylinder_integrator<unit_cylinder_gsl_integrator>("GSL Plain Unit Cylinder", 
        unit_cylinder_gsl_integrator::MonteCarloPlain);
    run_unit_cylinder_integrator<unit_cylinder_gsl_integrator>("GSL Miser Unit Cylinder", 
        unit_cylinder_gsl_integrator::MonteCarloMiser);
    run_unit_cylinder_integrator<unit_cylinder_gsl_integrator>("GSL Vegas Unit Cylinder", 
        unit_cylinder_gsl_integrator::MonteCarloVegas);
    #endif
    #ifdef HAVE_OPENCL
    run_unit_cylinder_integrator<unit_cylinder_opencl_integrator>("OpenCL Plain Unit Cylinder", 
        unit_cylinder_opencl_integrator::MonteCarloPlain);
    #endif

    //Run the random walk integrations
    #ifdef HAVE_GSL
    run_random_walk_integrator<random_walk_gsl_integrator>("GSL Plain Random Walk", 
        random_walk_gsl_integrator::MonteCarloPlain);
    run_random_walk_integrator<random_walk_gsl_integrator>("GSL Miser Random Walk", 
        random_walk_gsl_integrator::MonteCarloMiser);
    run_random_walk_integrator<random_walk_gsl_integrator>("GSL Vegas Random Walk", 
        random_walk_gsl_integrator::MonteCarloVegas);
    #endif
    #ifdef HAVE_OPENCL
    run_random_walk_integrator<random_walk_opencl_integrator>("OpenCL Plain Random Walk",
        random_walk_opencl_integrator::MonteCarloPlain);
    #endif

    return 0;
}
