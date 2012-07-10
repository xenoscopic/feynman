//Self-includes
\#include "${primary_header_include}"

#if len($integrator.integrand.include_dependencies) > 0
//Depedency includes
#for $include_dependency in $integrator.integrand.include_dependencies
\#include "$include_dependency"
#end for
#end if

${integrator.name}::${integrator.name}()
{
    gsl_rng_env_setup();
    _random_number_generator_type = gsl_rng_default;
    _random_number_generator = gsl_rng_alloc(_random_number_generator_type);
}

${integrator.name}::~${integrator.name}()
{
    //Cleanup
    gsl_rng_free(_random_number_generator);
}

$integrator.evaluation_function.return_type ${integrator.name}::operator()($integrator.evaluation_function.argument_signature)
{
    //Boiler plate GSL variables
    double result, _error;

    //Determine upper/lower bounds
    double lower_bounds[$integrator.n_dimensions] = {${", ".join($integrator.evaluation_function.argument_names[:-1:2])}};
    double upper_bounds[$integrator.n_dimensions] = {${", ".join($integrator.evaluation_function.argument_names[1:-1:2])}};

    //Call variables
    gsl_monte_function G = {&${integrator.name}::_wrapper, 
                            $integrator.n_dimensions,
                            NULL};
    size_t n_calls = $n_calls;

    //Integrate!
    {
$method_template
    }
    
    //Store results
    if(error != NULL)
    {
        *error = _error;
    }

    return result;
}

double ${integrator.name}::_wrapper(double *x, size_t dim, void *params)
{
    return ${integrator.integrand.name}(${", ".join(["x[%i]" % i for i in xrange(0, len($integrator.integrand.argument_types))])});
}
