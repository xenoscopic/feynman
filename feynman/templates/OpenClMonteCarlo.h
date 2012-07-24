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

        //OpenCL resources
        MonteCarloType _monte_carlo_type; //The type of Monte Carlo integration
        int _n_calls; //The number of calls for the integration to perform
	cl_platform_id _platform; //The platform to use
        cl_device_id _device; //The device to run on
        size_t _compute_units; //The number of compute units on the device (CL_DEVICE_MAX_COMPUTE_UNITS)
        size_t _max_concurrent_work_groups; //The maximum number of concurrent blocks allowed per multiprocessor
        cl_context _context; //The context in which to execute
        cl_command_queue _command_queue; //The command queue on which to execute commands
        cl_program _program; //The compiled source code
        
        //Utility kernels
        cl_kernel _mem_set; //The memset kernel
        cl_kernel _rng_init; //The random initialization kernel
        size_t _rng_init_work_group_size; //The size to use for random number 
                                          //generation initialization (not a performance
                                          //critical number since this is only run
                                          //once, at initialization or n_call change,
                                          //and not every integration.  I've found that
                                          //it's best to just use the preferred work group
                                          //size multiple for this.  If you use the maximum
                                          //kernel work group size, OpenCL sometimes throws
                                          //a fit, even though technically, that should work.
        
        //Plain integration resources
        cl_kernel _plain; //The plain MC integration kernel
        size_t _plain_work_group_size; //Size of an individual thread block
                                       //Filled by _calculate_kernel_execution_parameters
        size_t _plain_work_item_count; //Global number of work items
        cl_mem _plain_rng_states; //Random number generator states
                                       
        //Miser integration resources
        cl_kernel _miser; //The miser MC integration kernel
        size_t _miser_work_group_size; //Size of an individual thread block
                                       //Filled by _calculate_kernel_execution_parameters
        size_t _miser_work_item_count; //Global number of work items
        cl_mem _miser_rng_states; //Random number generator states

        //Vegas integration resources
        cl_kernel _vegas; //The vegas MC integration kernel
        size_t _vegas_work_group_size; //Size of an individual thread block
                                       //Filled by _calculate_kernel_execution_parameters
        size_t _vegas_work_item_count; //Global number of work items
        cl_mem _vegas_rng_states; //Random number generator states

        //Output resources
        cl_mem _output; //The global result output, 2 * sizeof(float)
                        //Format:
                        //  float sum_of_all_points; //For mean calculation
                        //  float sum_of_all_squares_of_points; //For variance calculation

        //Runs the following method for all kernels
        void _configure_kernels();

        //Calculates the work group size for a particular kernel
        //assuming that the following members have already been
        //set:
        //
        //  _n_calls
        //  _device
        //  _max_device_work_size
        //
        //It uses the following information
        //
        //   CL_DEVICE_MAX_WORK_ITEM_SIZES (max N-dimensional work group sizes, from _max_device_work_size)
        //   CL_KERNEL_WORK_GROUP_SIZE (max work group size for this kernel)
        //   CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE (recommended work 
        //   group size for this kernel)
        //
        //to find the optimial execution size
        void _configure_kernel(cl_kernel kernel,
                               size_t *work_group_size,
                               size_t *work_item_count,
                               cl_mem *rng_buffer);

        //Enqueues an output buffer clear
        void _enqueue_output_buffer_clear();

        //Enqueues an output buffer copy back to the host
        void _enqueue_output_buffer_read(float *host_buffer, 
                                         size_t count);

        //Static source code for the OpenCL program
        static const char * _fixes_source;
        static const char * _ranlux_source;
        static const char * _initialization_source;
        static const char * _integrand_source;
        static const char * _plain_source;
        static const char * _miser_source;
        static const char * _vegas_source;
};

\#endif //$include_guard
