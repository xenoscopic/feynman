\#ifndef $include_guard
\#define $include_guard

//OpenCL includes
\#ifdef __APPLE__
\#include <OpenCL/opencl.h>
\#else
\#include <CL/opencl.h>
\#endif

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
        cl_device_id _device_id;
        cl_context _context;
        cl_command_queue _command_queue;
        cl_program _program;
        cl_kernel _plain_kernel;
        cl_kernel _miser_kernel;
        cl_kernel _vegas_kernel;
        bool _valid;
        static const char * _fixes_source;
        static const char * _ranlux_source;
        static const char * _integrand_source;
        static const char * _plain_source;
        static const char * _miser_source;
        static const char * _vegas_source;
};

\#endif //$include_guard
