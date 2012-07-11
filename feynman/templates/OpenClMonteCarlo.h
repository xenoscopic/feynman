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
        ${integrator.name}();
        ~${integrator.name}();

        $integrator.evaluation_function.return_type operator()($integrator.evaluation_function.argument_signature, $integrator.evaluation_function.return_type *error);

    private:
        static const char * _fixes_source;
        static const char * _ranlux_source;
        static const char * _program_source;
        cl_device_id _device_id;
        cl_context _context;
        cl_command_queue _command_queue;
        cl_program _program;
        cl_kernel _kernel;
};

\#endif //$include_guard
