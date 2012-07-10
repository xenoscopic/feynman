\#ifndef $include_guard
\#define $include_guard

//Standard includes
\#include <cstdlib>

//GSL includes
\#include <gsl/gsl_math.h>
\#include <gsl/gsl_monte.h>
\#include <$gsl_method_header>

class $integrator.name
{
    public:
        ${integrator.name}();
        ~${integrator.name}();

        $integrator.evaluation_function.return_type operator()($integrator.evaluation_function.argument_signature);

    private:
        const gsl_rng_type *_random_number_generator_type;
        gsl_rng *_random_number_generator;
        static double _wrapper(double *x, size_t dim, void *params);
};

\#endif //$include_guard
