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
        //Don't allow copying due to buffer resources
        ${integrator.name}( const ${integrator.name}& );
        const ${integrator.name}& operator=( const ${integrator.name}& );

        //These values are controlled by the constructor/
        //destructor
        bool _valid;
        MonteCarloType _monte_carlo_type;
        int _n_calls;
        cl_device_id _device_id;
        cl_context _context;
        cl_command_queue _command_queue;
        cl_program _program;
        cl_kernel _ran_init; //The random initialization kernel
        cl_kernel _plain; //The plain MC integration kernel
        cl_kernel _miser; //The miser MC integration kernel
        cl_kernel _vegas; //The vegas MC integration kernel
        cl_kernel _sum; //The global array summation kernel
        cl_mem _output; //The global result output, sizeof(float)

        //These values are controlled by the two methods below
        struct _${integrator.name}KernelResources
        {   
            _${integrator.name}KernelResources();
            _${integrator.name}KernelResources(cl_context context,
                                               cl_kernel kernel,
                                               cl_device_id device,
                                               int n_calls);
            ~_${integrator.name}KernelResources();
            size_t work_group_size; //Number of work items per work group
            size_t work_group_count; //Number of work groups
            size_t work_item_point_count; //Number of phase space points per work item
            cl_mem ranluxcl_states; //Random number generator states
            cl_mem group_outputs; //The global output buffer for each work group, one entry per group
            bool valid;

            private:
                //Don't allow copying due to buffer resources
               _${integrator.name}KernelResources( const _${integrator.name}KernelResources& );
                const _${integrator.name}KernelResources& operator=( const _${integrator.name}KernelResources& );
        } * _plain_resources, * _miser_resources, * _vegas_resources;

        void _create_kernel_resources();
        void _release_kernel_resources();

        static const char * _fixes_source;
        static const char * _ranlux_source;
        static const char * _initialization_source;
        static const char * _integrand_source;
        static const char * _plain_source;
        static const char * _miser_source;
        static const char * _vegas_source;
};

\#endif //$include_guard
