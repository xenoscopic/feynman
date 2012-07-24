//Self-includes
\#include "${primary_header_include}"

//Standard includes
\#include <vector>
\#include <cstdlib>
\#include <cstdio>
\#include <ctime>
\#include <cmath>

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
_platform(NULL),
_device(NULL),
_compute_units(0),
_max_concurrent_work_groups(0),
_context(NULL),
_command_queue(NULL),
_program(NULL),
_mem_set(NULL),
_rng_init(NULL),
_rng_init_work_group_size(0),
_plain(NULL),
_plain_work_group_size(0),
_plain_work_item_count(0),
_plain_rng_states(NULL),
_miser(NULL),
_miser_work_group_size(0),
_miser_work_item_count(0),
_miser_rng_states(NULL),
_vegas(NULL),
_vegas_work_group_size(0),
_vegas_work_item_count(0),
_vegas_rng_states(NULL),
_output(NULL)
{
    //Grab all available platforms
    cl_uint num_platforms;
    CHECK_CL_OPERATION(clGetPlatformIDs(0, NULL, &num_platforms),
                       "Unable to get platform count");
    if(num_platforms == 0)
    {
        fprintf(stderr, "ERROR: Unable to find any OpenCL platform\n");
        exit(EXIT_FAILURE);
    }
    vector<cl_platform_id> platforms(num_platforms, NULL);
    CHECK_CL_OPERATION(clGetPlatformIDs(num_platforms, &platforms[0], NULL),
                       "Unable to find a valid platform");

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
        vector<cl_platform_id>::iterator plat_it;
        for(plat_it = platforms.begin();
            plat_it != platforms.end();
            plat_it++)
        {
            error = clGetDeviceIDs(*plat_it, preferred_device_types[i], 1, &_device, NULL);
            if(error != CL_DEVICE_NOT_FOUND)
            {
                _platform = *plat_it;
                break;
            }
        }
    }
    CHECK_CL_OPERATION(error, "Unable to find a valid compute device");

    //Query the device's compute units
    CHECK_CL_OPERATION(clGetDeviceInfo(_device,
                                       CL_DEVICE_MAX_COMPUTE_UNITS,
                                       sizeof(size_t),
                                       &_compute_units,
                                       NULL),
                       "Unable to query device compute unit count");
    _max_concurrent_work_groups = 2; //TODO: Find a way to query this

    //Create a compute context
    _context = clCreateContext(0, 1, &_device, NULL, NULL, &error);
    CHECK_CL_OPERATION(error, "Unable to create a compute context");

    //Create a command queue
    _command_queue = clCreateCommandQueue(_context, _device, 0, &error);
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

        CHECK_CL_OPERATION(clGetProgramBuildInfo(_program, 
                                                 _device, 
                                                 CL_PROGRAM_BUILD_LOG, 
                                                 sizeof(buffer), 
                                                 buffer, 
                                                 &length),
                           "Build failure.  Unable to get build failure information");

        printf("ERROR: Unable to build (compile) the OpenCL program (%i):\n%s\n", error, buffer);
    }
    CHECK_CL_OPERATION(error, "Unable to compile OpenCL source");

    //Grab out all kernels from the program
    _rng_init = clCreateKernel(_program, "random_initialize", &error);
    CHECK_CL_OPERATION(error, "Unable to create initialization kernel");
    _mem_set = clCreateKernel(_program, "mem_set", &error);
    CHECK_CL_OPERATION(error, "Unable to create memory setting kernel");
    _plain = clCreateKernel(_program, "plain_integrate", &error);
    CHECK_CL_OPERATION(error, "Unable to create plain Monte Carlo integration kernel");
    _miser = clCreateKernel(_program, "miser_integrate", &error);
    CHECK_CL_OPERATION(error, "Unable to create miser Monte Carlo integration kernel");
    _vegas = clCreateKernel(_program, "vegas_integrate", &error);
    CHECK_CL_OPERATION(error, "Unable to create vegas Monte Carlo integration kernel");

    //Configure the random number generation intialization
    //kernel.
    CHECK_CL_OPERATION(clGetKernelWorkGroupInfo(_rng_init,
                                                _device,
                                                CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                                sizeof(size_t),
                                                &_rng_init_work_group_size,
                                                NULL),
                       "Unable to determine preferred kernel work group size multiple for " \
                       "random number generator initialization.");

    //Configure kernel execution parameters
    _configure_kernels();

    //Create the global output buffer
    _output = clCreateBuffer(_context, CL_MEM_WRITE_ONLY, 2 * sizeof(float), NULL, &error);
    CHECK_CL_OPERATION(error, "Unable to create output buffer");
}

${integrator.name}::~${integrator.name}()
{
    RELEASE_CL_MEMORY_SAFE(_output);

    RELEASE_CL_MEMORY_SAFE(_vegas_rng_states);
    RELEASE_CL_KERNEL_SAFE(_vegas);

    RELEASE_CL_MEMORY_SAFE(_miser_rng_states);
    RELEASE_CL_KERNEL_SAFE(_miser);

    RELEASE_CL_MEMORY_SAFE(_plain_rng_states);
    RELEASE_CL_KERNEL_SAFE(_plain);
    
    RELEASE_CL_KERNEL_SAFE(_mem_set);
    RELEASE_CL_KERNEL_SAFE(_rng_init);
    
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
}

int ${integrator.name}::n_calls()
{
    return _n_calls;
}

$integrator.evaluation_function.return_type ${integrator.name}::operator()($integrator.evaluation_function.argument_signature, $integrator.evaluation_function.return_type *error)
{
    //Boilerplate variables
    cl_int _error;
    cl_uint points_per_work_item;
    cl_uint total_n_calls;
    
    //Queue the memset kernel
    _enqueue_output_buffer_clear();

    //Enqueue the appropriate kernel
    if(_monte_carlo_type == MonteCarloPlain)
    {
        points_per_work_item = _n_calls / _plain_work_item_count;
        if(_n_calls % _plain_work_item_count)
        {
            points_per_work_item++;
        }
        total_n_calls = points_per_work_item * _plain_work_item_count;
        CHECK_CL_OPERATION(clSetKernelArg(_plain, 0, sizeof(cl_uint), &points_per_work_item), 
                           "Unable to set number of integration points");
        CHECK_CL_OPERATION(clSetKernelArg(_plain, 1, sizeof(cl_mem), &_plain_rng_states), 
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
                                                  &_plain_work_item_count,
                                                  &_plain_work_group_size,
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
    float result[2];
    _enqueue_output_buffer_read(result, sizeof(result) / sizeof(float));

    //Wait for all operations to finish
    CHECK_CL_OPERATION(clFinish(_command_queue), "Unable to execute integration");

    //Calculate the total number of calls and the volume
    float volume = ${"*".join(["(%s - %s)" % ($integrator.evaluation_function.argument_names[2*i + 1], $integrator.evaluation_function.argument_names[2*i]) for i in xrange(0, len($integrator.integrand.argument_types))])};
    
    //The following calculations are based upon this documentation:
    //http://mathworld.wolfram.com/MonteCarloIntegration.html

    //Calculate mean
    float mean = volume * result[0] / ((float)total_n_calls);

    //Calculate variance
    float variance = volume * sqrt((result[1] - (result[0] * result[0] / ((float)total_n_calls))) / (((float)total_n_calls) * ((float)total_n_calls)));
    if(error != NULL)
    {
        *error = variance;
    }

    return mean;
}

void ${integrator.name}::_configure_kernels()
{
    _configure_kernel(_plain,
                      &_plain_work_group_size,
                      &_plain_work_item_count,
                      &_plain_rng_states);
    _configure_kernel(_miser,
                      &_miser_work_group_size,
                      &_miser_work_item_count,
                      &_miser_rng_states);
    _configure_kernel(_vegas,
                      &_vegas_work_group_size,
                      &_vegas_work_item_count,
                      &_vegas_rng_states);
}

void ${integrator.name}::_configure_kernel(cl_kernel kernel,
                                           size_t *work_group_size,
                                           size_t *work_item_count,
                                           cl_mem *rng_buffer)
{
    //Clear any prior information
    RELEASE_CL_MEMORY_SAFE(*rng_buffer);

    //Calculate the maximum work group size
    //and preferred work group size multiple
    //for this device and kernel
    size_t max_work_group_size;
    CHECK_CL_OPERATION(clGetKernelWorkGroupInfo(kernel,
                                                _device,
                                                CL_KERNEL_WORK_GROUP_SIZE,
                                                sizeof(size_t),
                                                &max_work_group_size,
                                                NULL),
                       "Unable to determine maximum kernel work group size");

    size_t preferred_work_group_size_multiple;
    CHECK_CL_OPERATION(clGetKernelWorkGroupInfo(kernel,
                                                _device,
                                                CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                                sizeof(size_t),
                                                &preferred_work_group_size_multiple,
                                                NULL),
                       "Unable to determine preferred kernel work group size multiple");

    //Run the work group size up until <= to the maximum
    *work_group_size = preferred_work_group_size_multiple;
    while(*work_group_size <= max_work_group_size)
    {
        *work_group_size += preferred_work_group_size_multiple;
    }
    *work_group_size -= preferred_work_group_size_multiple;

    //Calculate the global work item count
    *work_item_count = _compute_units * (*work_group_size) * _max_concurrent_work_groups;

    //Create the random number buffer
    cl_int error;
    *rng_buffer = clCreateBuffer(_context,
                                 CL_MEM_READ_WRITE, 
                                 (*work_item_count) * RANLUXCL_STATE_SIZE, 
                                 NULL, 
                                 &error);
    CHECK_CL_OPERATION(error, "Unable to create random number generator state buffer");

    //Print some information
    // printf("--------\n");
    // printf("Work Group Size: %zu\n", *work_group_size);
    // printf("Work Item Count: %zu\n", *work_item_count);
    // printf("RNG Buffer: %p\n", rng_buffer);
    // printf("RNG Init Work Group Size: %zu\n", _rng_init_work_group_size);
    // printf("--------\n");

    //Initialize the random number buffer
    //Run the random number initialization kernel
    cl_uint seed;
    time_t t;
    time(&t);
    seed = t;
    
    CHECK_CL_OPERATION(clSetKernelArg(_rng_init, 0, sizeof(seed), &seed), 
                       "Unable to set random number seed");
    CHECK_CL_OPERATION(clSetKernelArg(_rng_init, 1, sizeof(cl_mem), rng_buffer), 
                       "Couldn't set random number state buffer");
    //HACK: Technically the work item counts for the integration kernel
    //might not jive with the random number generator initialization
    //kernel work group size, but I'm not going to bother fixing this
    //until it becomes a problem.
    CHECK_CL_OPERATION(clEnqueueNDRangeKernel(_command_queue, 
                                              _rng_init, 
                                              1, 
                                              NULL, 
                                              work_item_count,
                                              &_rng_init_work_group_size,
                                              0, 
                                              NULL, 
                                              NULL),
                       "Unable to queue random number initialization kernel");
    CHECK_CL_OPERATION(clFinish(_command_queue), 
                       "Unable to execute random number initialization kernel");
}

void ${integrator.name}::_enqueue_output_buffer_clear()
{
    float mem_value = 0;
    size_t memory_count = 2;
    CHECK_CL_OPERATION(clSetKernelArg(_mem_set, 0, sizeof(float), &mem_value), 
                       "Unable to set memory set value");
    CHECK_CL_OPERATION(clSetKernelArg(_mem_set, 1, sizeof(cl_mem), &_output), 
                       "Couldn't set memory set buffer");
    CHECK_CL_OPERATION(clEnqueueNDRangeKernel(_command_queue, 
                                              _mem_set, 
                                              1, 
                                              NULL, 
                                              &memory_count,
                                              &memory_count,
                                              0, 
                                              NULL, 
                                              NULL),
                       "Unable to queue random number memory setting kernel");
}

void ${integrator.name}::_enqueue_output_buffer_read(float *host_buffer, 
                                                     size_t count)
{
    CHECK_CL_OPERATION(clEnqueueReadBuffer(_command_queue, 
                                           _output, 
                                           CL_FALSE, 
                                           0, 
                                           count * sizeof(float), 
                                           host_buffer, 
                                           0, 
                                           NULL, 
                                           NULL), 
                       "Unable to read output buffer");
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
