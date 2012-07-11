//Self-includes
\#include "${primary_header_include}"

#if len($integrator.integrand.include_dependencies) > 0
//Depedency includes
#for $include_dependency in $integrator.integrand.include_dependencies
\#include "$include_dependency"
#end for
#end if

${integrator.name}::${integrator.name}() :
_monte_carlo_type(${integrator.name}::MonteCarloPlain),
_n_calls(500000),
_plain_state(gsl_monte_plain_alloc($integrator.n_dimensions)),
_miser_state(gsl_monte_miser_alloc($integrator.n_dimensions)),
_vegas_state(gsl_monte_vegas_alloc($integrator.n_dimensions))
{
    gsl_rng_env_setup();
    _random_number_generator_type = gsl_rng_default;
    _random_number_generator = gsl_rng_alloc(_random_number_generator_type);
}

${integrator.name}::~${integrator.name}()
{
    //Cleanup
    gsl_rng_free(_random_number_generator);
    gsl_monte_vegas_free(_vegas_state);
    gsl_monte_miser_free(_miser_state);
    gsl_monte_plain_free(_plain_state);
}

void ${integrator.name}::set_monte_carlo_type(${integrator.name}::MonteCarloType t)
{
    _monte_carlo_type = t;
}

${integrator.name}::MonteCarloType ${integrator.name}::monte_carlo_type()
{
    return _monte_carlo_type;
}

void ${integrator.name}::set_n_calls(int n)
{
    _n_calls = n;
}

int ${integrator.name}::n_calls()
{
    return _n_calls;
}

$integrator.evaluation_function.return_type ${integrator.name}::operator()($integrator.evaluation_function.argument_signature, $integrator.evaluation_function.return_type *error)
{
    //Boiler plate GSL variables
    double result, _error;

    //Determine upper/lower bounds
    double lower_bounds[$integrator.n_dimensions] = {${", ".join($integrator.evaluation_function.argument_names[:-1:2])}};
    double upper_bounds[$integrator.n_dimensions] = {${", ".join($integrator.evaluation_function.argument_names[1::2])}};

    //Call variables
    gsl_monte_function G = {&${integrator.name}::_wrapper, 
                            $integrator.n_dimensions,
                            NULL};

    //Integrate!
    if(_monte_carlo_type == MonteCarloPlain)
    {
        gsl_monte_plain_integrate(&G, 
                                  lower_bounds, 
                                  upper_bounds, 
                                  $integrator.n_dimensions, 
                                  _n_calls, 
                                  _random_number_generator, 
                                  _plain_state, 
                                  &result, 
                                  &_error);
    }
    else if(_monte_carlo_type == MonteCarloMiser)
    {
        gsl_monte_miser_integrate(&G, 
                                  lower_bounds, 
                                  upper_bounds, 
                                  $integrator.n_dimensions, 
                                  _n_calls, 
                                  _random_number_generator, 
                                  _miser_state, 
                                  &result, 
                                  &_error);
    }
    else if(_monte_carlo_type == MonteCarloVegas)
    {
        gsl_monte_vegas_integrate(&G, 
                                  lower_bounds, 
                                  upper_bounds, 
                                  $integrator.n_dimensions, 
                                  10000, 
                                  _random_number_generator, 
                                  _vegas_state,
                                  &result,
                                  &_error);
        //Now actually converge on a result
        do
        {
            gsl_monte_vegas_integrate(&G, 
                                      lower_bounds, 
                                      upper_bounds, 
                                      $integrator.n_dimensions, 
                                      _n_calls/5, 
                                      _random_number_generator, 
                                      _vegas_state,
                                      &result, 
                                      &_error);
        }
        while(fabs(gsl_monte_vegas_chisq(_vegas_state) - 1.0) > 0.5);
    }
    else
    {
        result = 0.0;
        _error = 0.0;
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
