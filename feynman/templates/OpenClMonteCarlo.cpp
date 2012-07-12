//Self-includes
\#include "${primary_header_include}"

//Standard includes
\#include <cstdlib>
\#include <cstdio>
\#include <stdexcept>
\#include <ctime>

#if len($integrator.integrand.include_dependencies) > 0
//Depedency includes
#for $include_dependency in $integrator.integrand.include_dependencies
\#include "$include_dependency"
#end for
#end if

//Standard namespaces
using namespace std;

//Useful macros
#define RELEASE_CL_CONTEXT_SAFE(mem)\
    {\
        if(mem != NULL)\
        {\
            clReleaseContext(mem);\
            mem = NULL;\
        }\
    }\

#define RELEASE_CL_COMMAND_QUEUE_SAFE(mem)\
    {\
        if(mem != NULL)\
        {\
            clReleaseCommandQueue(mem);\
            mem = NULL;\
        }\
    }\

#define RELEASE_CL_PROGRAM_SAFE(mem)\
    {\
        if(mem != NULL)\
        {\
            clReleaseProgram(mem);\
            mem = NULL;\
        }\
    }\

#define RELEASE_CL_KERNEL_SAFE(mem)\
    {\
        if(mem != NULL)\
        {\
            clReleaseKernel(mem);\
            mem = NULL;\
        }\
    }\

#define RELEASE_CL_MEMORY_SAFE(mem)\
    {\
        if(mem != NULL)\
        {\
            clReleaseMemObject(mem);\
            mem = NULL;\
        }\
    }\

#define CHECK_CL_OPERATION(operation, error_message)\
    {\
        cl_int CHECK_CL_OPERATION_ERROR = (operation);\
        if(CHECK_CL_OPERATION_ERROR  != CL_SUCCESS){\
            fprintf(stderr, "ERROR: OpenCL error (%d) in %s (line %d): %s\n",\
                CHECK_CL_OPERATION_ERROR, __FILE__, __LINE__, error_message);\
            exit(EXIT_FAILURE);\
        }\
    }\

#define RANLUXCL_STATE_SIZE 112

${integrator.name}::${integrator.name}() :
_monte_carlo_type(${integrator.name}::MonteCarloPlain),
_n_calls(500000),
_device_id(NULL),
_context(NULL),
_command_queue(NULL),
_program(NULL),
_ran_init(NULL),
_plain(NULL),
_miser(NULL),
_vegas(NULL),
_sum(NULL),
_output(NULL),
_plain_resources(NULL),
_miser_resources(NULL),
_vegas_resources(NULL)
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
    cl_int error = CL_DEVICE_NOT_FOUND;
    while(error == CL_DEVICE_NOT_FOUND
          && i < n_device_types)
    {
        error = clGetDeviceIDs(NULL, preferred_device_types[i], 1, &_device_id, NULL);
    }
    CHECK_CL_OPERATION(error, "Unable to find a valid compute device");

    //Create a compute context
    _context = clCreateContext(0, 1, &_device_id, NULL, NULL, &error);
    CHECK_CL_OPERATION(error, "Unable to create a compute context");

    //Create a command queue
    _command_queue = clCreateCommandQueue(_context, _device_id, 0, &error);
    CHECK_CL_OPERATION(error, "Unable to create a command queue");

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
    CHECK_CL_OPERATION(error, "Unable to create the OpenCL program");
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
    }
    CHECK_CL_OPERATION(error, "Unable to compile OpenCL source");

    //Grab out the integration kernels
    _ran_init = clCreateKernel(_program, "random_initialize", &error);
    CHECK_CL_OPERATION(error, "Unable to create initialization kernel");
    _plain = clCreateKernel(_program, "plain_integrate", &error);
    CHECK_CL_OPERATION(error, "Unable to create plain Monte Carlo integration kernel");
    _miser = clCreateKernel(_program, "miser_integrate", &error);
    CHECK_CL_OPERATION(error, "Unable to create miser Monte Carlo integration kernel");
    _vegas = clCreateKernel(_program, "vegas_integrate", &error);
    CHECK_CL_OPERATION(error, "Unable to create vegas Monte Carlo integration kernel");

    //Create the global output buffer
    _output = clCreateBuffer(_context, CL_MEM_WRITE_ONLY, sizeof(float), NULL, &error);
    CHECK_CL_OPERATION(error, "Unable to create output buffer");

    //Configure kernel resources
    _create_kernel_resources();
}

${integrator.name}::~${integrator.name}()
{
    _release_kernel_resources();
    RELEASE_CL_MEMORY_SAFE(_output);
    RELEASE_CL_KERNEL_SAFE(_sum);
    RELEASE_CL_KERNEL_SAFE(_vegas);
    RELEASE_CL_KERNEL_SAFE(_miser);
    RELEASE_CL_KERNEL_SAFE(_plain);
    RELEASE_CL_KERNEL_SAFE(_ran_init);
    RELEASE_CL_PROGRAM_SAFE(_program);
    RELEASE_CL_COMMAND_QUEUE_SAFE(_command_queue);
    RELEASE_CL_CONTEXT_SAFE(_context);
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
    _release_kernel_resources();
    _create_kernel_resources();
}

int ${integrator.name}::n_calls()
{
    return _n_calls;
}

$integrator.evaluation_function.return_type ${integrator.name}::operator()($integrator.evaluation_function.argument_signature, $integrator.evaluation_function.return_type *error)
{
    //Boilerplate variables
    cl_int _error;
    size_t work_item_count = 1;

    //Enqueue the appropriate kernel
    if(_monte_carlo_type == MonteCarloPlain)
    {
        work_item_count = _plain_resources->work_group_size * _plain_resources->work_group_count;
        CHECK_CL_OPERATION(clSetKernelArg(_plain, 0, sizeof(size_t), &(_plain_resources->work_item_point_count)), 
                           "Unable to set number of integration points");
        CHECK_CL_OPERATION(clSetKernelArg(_plain, 1, sizeof(cl_mem), &(_plain_resources->ranluxcl_states)), 
                           "Couldn't set random number state buffer");
        CHECK_CL_OPERATION(clSetKernelArg(_plain, 2, sizeof(cl_mem), &_output), 
                           "Couldn't set output buffer");
        #for $i, ($a_t, $a_n) in enumerate(zip($integrator.evaluation_function.argument_types, $integrator.evaluation_function.argument_names))
        CHECK_CL_OPERATION(clSetKernelArg(_plain, ${$i + 3}, sizeof($a_t), &$a_n), 
                           "Couldn't set output buffer");
        #end for
        CHECK_CL_OPERATION(clEnqueueNDRangeKernel(_command_queue, 
                                                  _plain, 
                                                  1, 
                                                  NULL, 
                                                  &work_item_count,
                                                  &(_plain_resources->work_group_size),
                                                  0, 
                                                  NULL, 
                                                  NULL),
                           "Unable to queue random number initialization kernel");
    }
    else if(_monte_carlo_type == MonteCarloMiser)
    {
        printf("ERROR: Miser integration not yet implemented.\n");
        return 0.0;
    }
    else if(_monte_carlo_type == MonteCarloVegas)
    {
        printf("ERROR: Vegas integration not yet implemented.\n");
        return 0.0;
    }
    else
    {
        printf("ERROR: Unknown integration type.\n");
        return 0.0;
    }

    //Enqueue the answer copy
    float result;
    CHECK_CL_OPERATION(clEnqueueReadBuffer(_command_queue, 
                                           _output, 
                                           CL_FALSE, 
                                           0, 
                                           sizeof(float), 
                                           &result, 
                                           0, 
                                           NULL, 
                                           NULL), 
                       "Unable to read output buffer");

    //Wait for all operations to finish
    CHECK_CL_OPERATION(clFinish(_command_queue), "Unable to execute integration");

    //Adjust for integral volume
    float volume = ${"*".join(["(%s - %s)" % ($integrator.evaluation_function.argument_names[2*i + 1], $integrator.evaluation_function.argument_names[2*i]) for i in xrange(0, len($integrator.integrand.argument_types))])};
    result *= volume;

    //We now need to compute the total number of calls
    //that were made, and divide the result by that
    if(_monte_carlo_type == MonteCarloPlain)
    {
        size_t total_n_calls = work_item_count * _plain_resources->work_item_point_count;
        printf("total_n_calls: %zu, result: %f\n", total_n_calls, result);
        result /= ((float)total_n_calls);
    }

    return result;
}

void ${integrator.name}::_create_kernel_resources()
{
    if(_plain_resources == NULL)
    {
        _plain_resources = new _${integrator.name}KernelResources(_context, _command_queue, _ran_init, _plain, _device_id, _n_calls);
    }
    if(_miser_resources == NULL)
    {
        _miser_resources = new _${integrator.name}KernelResources(_context, _command_queue, _ran_init, _miser, _device_id, _n_calls);
    }
    if(_vegas_resources == NULL)
    {
        _vegas_resources = new _${integrator.name}KernelResources(_context, _command_queue, _ran_init, _vegas, _device_id, _n_calls);
    }
}

void ${integrator.name}::_release_kernel_resources()
{
    if(_plain_resources != NULL)
    {
        delete _plain_resources;
        _plain_resources = NULL;
    }
    if(_miser_resources != NULL)
    {
        delete _miser_resources;
        _miser_resources = NULL;
    }
    if(_vegas_resources != NULL)
    {
        delete _vegas_resources;
        _vegas_resources = NULL;
    }
}

${integrator.name}::_${integrator.name}KernelResources::_${integrator.name}KernelResources() :
work_item_point_count(0),
work_group_size(0),
work_group_count(0),
ranluxcl_states(NULL)
{

}

${integrator.name}::_${integrator.name}KernelResources::_${integrator.name}KernelResources(cl_context context,
                                                                                           cl_command_queue command_queue,
                                                                                           cl_kernel random_init_kernel,
                                                                                           cl_kernel method_kernel,
                                                                                           cl_device_id device,
                                                                                           int n_calls) :
work_group_size(0),
work_group_count(0),
work_item_point_count(0),
ranluxcl_states(NULL)
{
    //Kernel/Device execution specs
    //The preferred work group size multiple for the random 
    //number initializtaion kernel on the device
    size_t preferred_random_init_work_group_size_multiple;
    //The preferred work group size multiple for the integration
    //kernel on the device
    size_t preferred_work_group_size_multiple;
    //Number of dimensions of work items that the device supports, guaranteed to be at least 3
    const size_t max_work_item_dimensions = 3;
    //Maximum number of work items along the first dimension
    size_t max_work_items_in_1d;
    
    //Grab execution specs
    CHECK_CL_OPERATION(clGetKernelWorkGroupInfo(random_init_kernel,
                                                device,
                                                CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                                sizeof(size_t),
                                                &preferred_random_init_work_group_size_multiple,
                                                NULL),
                       "Unable to determine preferred work group size multiple for initialization kernel");
    CHECK_CL_OPERATION(clGetKernelWorkGroupInfo(method_kernel,
                                                device,
                                                CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                                sizeof(size_t),
                                                &preferred_work_group_size_multiple,
                                                NULL),
                       "Unable to determine preferred work group size multiple for integration kernel");

    //HACK: There seems to be a bug in this method, at least on OS X,
    //where if you make this query twice for the same device, it returns
    //a huge number (2251765453950976) for the number of dimensions.
    //Since the number of dimensions is likely going to remain 3 for
    //a while, I'll just fix that above.
    // CHECK_CL_OPERATION(clGetDeviceInfo(device, 
    //                                    CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, 
    //                                    sizeof(size_t), 
    //                                    &max_work_item_dimensions, 
    //                                    NULL),
    //                    "Unable to determine maximum number of work item dimensions");
    size_t *max_global_work_items = new size_t[max_work_item_dimensions];
    CHECK_CL_OPERATION(clGetDeviceInfo(device, 
                                       CL_DEVICE_MAX_WORK_ITEM_SIZES, 
                                       max_work_item_dimensions * sizeof(size_t), 
                                       max_global_work_items, 
                                       NULL),
                       "Unable to determine maximum number of work item dimensions");
    max_work_items_in_1d = max_global_work_items[0];
    delete [] max_global_work_items;
    max_global_work_items = NULL;
    
    //Calculate execution parameters
    size_t work_item_count = max_work_items_in_1d;

    //Calculate the work group size
    work_group_size = preferred_work_group_size_multiple;

    //Calculate the number of work groups
    work_group_count = work_item_count / work_group_size;

    //Calculate the number of points each work item will
    //have to calculate
    work_item_point_count = n_calls / work_item_count;
    if(work_item_point_count * work_item_count < n_calls)
    {
        work_item_point_count++; //Take care of division remainder
    }

    //Print execution information
    // printf("Work Group Size:  %zu\n", work_group_size);
    // printf("Work Group Count: %zu\n", work_group_count);
    // printf("Total Work Items: %zu\n", work_item_count);
    // printf("Points per Work Item: %zu\n", work_item_point_count);

    //Create the random number generator and output buffers
    cl_int error;
    ranluxcl_states = clCreateBuffer(context,
                                     CL_MEM_READ_WRITE, 
                                     work_item_count * RANLUXCL_STATE_SIZE, 
                                     NULL, 
                                     &error);
    CHECK_CL_OPERATION(error, "Unable to create random number generator");

    //Run the random number initialization kernel
    cl_uint seed;
    time_t t;
    time(&t);
    seed = t;
    
    CHECK_CL_OPERATION(clSetKernelArg(random_init_kernel, 0, sizeof(seed), &seed), 
                       "Unable to set random number seed");
    CHECK_CL_OPERATION(clSetKernelArg(random_init_kernel, 1, sizeof(cl_mem), &ranluxcl_states), 
                       "Couldn't set random number state buffer");
    CHECK_CL_OPERATION(clEnqueueNDRangeKernel(command_queue, 
                                              random_init_kernel, 
                                              1, 
                                              NULL, 
                                              &work_item_count,
                                              &preferred_random_init_work_group_size_multiple,
                                              0, 
                                              NULL, 
                                              NULL),
                       "Unable to queue random number initialization kernel");
    CHECK_CL_OPERATION(clFinish(command_queue), 
                       "Unable to execute random number initialization kernel");
}

${integrator.name}::_${integrator.name}KernelResources::~_${integrator.name}KernelResources()
{
    RELEASE_CL_MEMORY_SAFE(ranluxcl_states);
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
