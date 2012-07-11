//Self-includes
\#include "${primary_header_include}"

//Standard includes
\#include <cstdio>
\#include <stdexcept>

#if len($integrator.integrand.include_dependencies) > 0
//Depedency includes
#for $include_dependency in $integrator.integrand.include_dependencies
\#include "$include_dependency"
#end for
#end if

//Standard namespaces
using namespace std;

${integrator.name}::${integrator.name}() :
_monte_carlo_type(${integrator.name}::MonteCarloPlain),
_n_calls(500000),
_device_id(NULL),
_context(NULL),
_command_queue(NULL),
_program(NULL),
_initialization_kernel(NULL),
_plain_kernel(NULL),
_miser_kernel(NULL),
_vegas_kernel(NULL),
_valid(true)
{
    //Try to find a device by type preference
    cl_device_type preferred_device_types[] = {
        CL_DEVICE_TYPE_GPU,
        CL_DEVICE_TYPE_CPU,
        CL_DEVICE_TYPE_ACCELERATOR,
        CL_DEVICE_TYPE_DEFAULT,
        CL_DEVICE_TYPE_ALL
    };
    int i = 0;
    int n_device_types = sizeof(preferred_device_types)/sizeof(cl_device_type);
    int error = CL_DEVICE_NOT_FOUND;
    while(error == CL_DEVICE_NOT_FOUND
          && i < n_device_types)
    {
        error = clGetDeviceIDs(NULL, preferred_device_types[i], 1, &_device_id, NULL);
    }
    if(error != CL_SUCCESS)
    {
        //Unable to find a valid device
        printf("ERROR: Unable to find a valid compute device\n");
        _valid = false;
        return;
    }

    //Create a compute context
    _context = clCreateContext(0, 1, &_device_id, NULL, NULL, &error);
    if(!_context)
    {
        //Unable to create a compute context
        printf("ERROR: Unable to create a compute context.\n");
        _valid = false;
        return;
    }

    //Create a command queue
    _command_queue = clCreateCommandQueue(_context, _device_id, 0, &error);
    if(!_command_queue)
    {
        //Unable to create a command queue
        printf("ERROR: Unable to create a command queue.\n");
        _valid = false;
        return;
    }

    //Compile the OpenCL code
    const char *strings[] = {
        $integrator.name::_fixes_source,
        $integrator.name::_ranlux_source,
        $integrator.name::_initialization_source,
        $integrator.name::_integrand_source,
        $integrator.name::_plain_source,
        $integrator.name::_miser_source,
        $integrator.name::_vegas_source
    };
    cl_uint n_sources = sizeof(strings)/sizeof(const char *);
    _program = clCreateProgramWithSource(_context,
                                         n_sources,
                                         strings,
                                         NULL,
                                         &error);
    if(!_program)
    {
        //Unable to create the OpenCL program
        printf("ERROR: Unable to create the OpenCL program.\n");
        _valid = false;
        return;
    }
    error = clBuildProgram(_program, 
                           0,
                           NULL,
                           NULL,
                           NULL,
                           NULL);
    if(error != CL_SUCCESS)
    {
        //Unable to build (compile) the OpenCL program
        size_t length;
        char buffer[2048];

        clGetProgramBuildInfo(_program, 
                              _device_id, 
                              CL_PROGRAM_BUILD_LOG, 
                              sizeof(buffer), 
                              buffer, 
                              &length);

        printf("ERROR: Unable to build (compile) the OpenCL program (%i):\n", error);
        printf("%s\n", buffer);
        _valid = false;
        return;
    }

    //Grab out the integration kernels
    _initialization_kernel = clCreateKernel(_program, "random_initialize", &error);
    if(!_initialization_kernel || error != CL_SUCCESS)
    {
        printf("ERROR: Unable to create initialization kernel.\n");
        _valid = false;
        return;
    }
    _plain_kernel = clCreateKernel(_program, "plain_integrate", &error);
    if(!_plain_kernel || error != CL_SUCCESS)
    {
        printf("ERROR: Unable to create plain Monte Carlo integration kernel.\n");
        _valid = false;
        return;
    }
    _miser_kernel = clCreateKernel(_program, "miser_integrate", &error);
    if(!_miser_kernel || error != CL_SUCCESS)
    {
        printf("ERROR: Unable to create miser Monte Carlo integration kernel.\n");
        _valid = false;
        return;
    }
    _vegas_kernel = clCreateKernel(_program, "vegas_integrate", &error);
    if(!_vegas_kernel || error != CL_SUCCESS)
    {
        printf("ERROR: Unable to create vegas Monte Carlo integration kernel.\n");
        _valid = false;
        return;
    }

    //TODO: Run random number initialization kernels

}

${integrator.name}::~${integrator.name}()
{
    clReleaseKernel(_vegas_kernel);
    clReleaseKernel(_miser_kernel);
    clReleaseKernel(_plain_kernel);
    clReleaseKernel(_initialization_kernel);
    clReleaseProgram(_program);
    clReleaseCommandQueue(_command_queue);
    clReleaseContext(_context);
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
    if(!_valid)
    {
        printf("ERROR: Integrator did not intialize correctly, and hence cannot integrate.");
        return 0;
    }

    //TODO: Launch the kernel!
    return 1;
}

const char * ${integrator.name}::_fixes_source = 
$fixes_template;
const char * ${integrator.name}::_ranlux_source = 
$ranlux_template;
const char * ${integrator.name}::_initialization_source = 
$initialization_template;
const char * ${integrator.name}::_integrand_source = 
$integrand_template;
const char * ${integrator.name}::_plain_source = 
$plain_template;
const char * ${integrator.name}::_miser_source = 
$miser_template;
const char * ${integrator.name}::_vegas_source = 
$vegas_template;
