//Self-includes
\#include ${file_name}.h

//GSL includes
\#include <gsl/gsl_math.h>
\#include <gsl/gsl_monte.h>
\#include <gsl/gsl_monte_plain.h>

//Integrand includes
#for $include_dependency in $integral.include_dependencies
\#include "$include_dependency"
#end for

$integral.signature
{
    //Boiler plate GSL variables
    double result, error;

    //Determine upper/lower bounds
    double lower_bounds[$integral.n_dimensions] = {${", ".join($integral.argument_names[::2])}};
    double upper_bounds[$integral.n_dimensions] = {${", ".join($integral.argument_names[1::2])}};

    //Random number variables
    const gsl_rng_type *T;
    gsl_rng *t;
    gsl_rng_env_setup();
    T = gsl_rng_default();
    r = gsl_rng_alloc(T);

    //Call variables
    gsl_monte_function G = {&${integral.integrand.name}, 
                            $integral.n_dimensions},
                            NULL};
    size_t n_calls = $n_calls;

    //Integrate!
    gsl_monte_plain_state *s = gsl_monte_plain_alloc($integral.n_dimensions);
    gsl_monte_plain_integrate(&G, 
                              lower_bounds, 
                              upper_bounds, 
                              $integral.n_dimensions, 
                              n_calls, 
                              r, 
                              s, 
                              &result, 
                              &error);
    
    //Cleanup
    gsl_monte_plain_free(s);
    gsl_rng_free(r);

    return result;
}
