//Self-includes
\#include "${header_include_name}"

//Standard includes
\#include <stdlib.h>

//GSL includes
\#include <gsl/gsl_math.h>
\#include <gsl/gsl_monte.h>
\#include <$integration_header_name>

#if len($integral.include_dependencies) > 0
//Depedency includes
#for $include_dependency in $integral.include_dependencies
\#include "$include_dependency"
#end for
#end if

double _${integral.name}_wrapper(double *x, size_t dim, void *params)
{
    return ${integral.integrand.name}(${", ".join(["x[%i]" % i for i in xrange(0, len($integral.integrand.argument_types))])});
}

$integral.signature
{
    //Boiler plate GSL variables
    double result, _error;

    //Determine upper/lower bounds
    double lower_bounds[$integral.n_dimensions] = {${", ".join($integral.argument_names[:-1:2])}};
    double upper_bounds[$integral.n_dimensions] = {${", ".join($integral.argument_names[1:-1:2])}};

    //Random number variables
    const gsl_rng_type *T;
    gsl_rng *r;
    gsl_rng_env_setup();
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);

    //Call variables
    gsl_monte_function G = {&_${integral.name}_wrapper, 
                            $integral.n_dimensions,
                            NULL};
    size_t n_calls = $n_calls;

    //Integrate!
    {
$integration_template
    }
    
    //Cleanup
    gsl_rng_free(r);

    //Store results
    if(error != NULL)
    {
        *error = _error;
    }

    return result;
}
