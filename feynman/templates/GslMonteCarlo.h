\#ifndef $include_guard
\#define $include_guard

//Standard includes
\#include <cstdlib>

//GSL includes
\#include <gsl/gsl_math.h>
\#include <gsl/gsl_monte.h>
\#include <gsl/gsl_monte_plain.h>
\#include <gsl/gsl_monte_miser.h>
\#include <gsl/gsl_monte_vegas.h>

class $integrator.name
{
    public:
        enum MonteCarloType
        {
            MonteCarloPlain,
            MonteCarloMiser,
            MonteCarloVegas
        };

        ${integrator.name}();
        ~${integrator.name}();

        void set_monte_carlo_type(MonteCarloType t);
        MonteCarloType monte_carlo_type();

        void set_n_calls(int n);
        int n_calls();

        $integrator.evaluation_function.return_type operator()($integrator.evaluation_function.argument_signature, $integrator.evaluation_function.return_type *error);

    private:
        MonteCarloType _monte_carlo_type;
        int _n_calls;
        gsl_monte_plain_state *_plain_state;
        gsl_monte_miser_state *_miser_state;
        gsl_monte_vegas_state *_vegas_state;
        const gsl_rng_type *_random_number_generator_type;
        gsl_rng *_random_number_generator;
        static double _wrapper(double *x, size_t dim, void *params);
};

\#endif //$include_guard
