//Self-includes
\#include "${primary_header_include}"

//Standard includes
\#include <stdexcept>

#if len($integrator.integrand.include_dependencies) > 0
//Depedency includes
#for $include_dependency in $integrator.integrand.include_dependencies
\#include "$include_dependency"
#end for
#end if

//Standard namespaces
using namespace std;

${integrator.name}::${integrator.name}()
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
        throw runtime_error("Unable to find a valid compute device.");
    }

    //Create a compute context
    _context = clCreateContext(0, 1, &_device_id, NULL, NULL, &error);
    if(!_context)
    {
        throw runtime_error("Unable to create a compute context.");
    }

    //Create a command queue
    _command_queue = clCreateCommandQueue(_context, _device_id, 0, &error);
    if(!_command_queue)
    {
        throw runtime_error("Unable to create a command queue.");
    }
}

${integrator.name}::~${integrator.name}()
{
    clReleaseCommandQueue(_command_queue);
    clReleaseContext(_context);
}

$integrator.evaluation_function.return_type ${integrator.name}::operator()($integrator.evaluation_function.argument_signature)
{
    return 1;
}

const char * ${integrator.name}::_ranlux_source = 
$ranlux_template;
const char * ${integrator.name}::_program_source = 
$program_template;
