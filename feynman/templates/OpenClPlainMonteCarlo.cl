#define MAX_PLAIN_MONTE_CARLO_WORK_GROUP_SIZE 1024

__kernel void plain_integrate(
    size_t points_per_worker,
    __global ranluxcl_state_t *ranluxcl_states,
    __global float *result,
    $integrator.evaluation_function.argument_signature
    )
{
    //The local (workgroup-shared) array where final results
    //will be stored and summed.
    __local float local_sums[MAX_PLAIN_MONTE_CARLO_WORK_GROUP_SIZE];

    //Thread-local variables
    size_t local_id = get_local_id(0);
    float private_sum = 0.0;

    //Download the random number generator
    ranluxcl_state_t ranluxcl_state;
    ranluxcl_download_seed(&ranluxcl_state, ranluxcl_states);

    //Loop over and evaluate random phase-space points.
    for(size_t i = 0; i < points_per_worker; i++)
    {
        //Generate a random phase-space point
        #set $n_blocks = len($integrator.integrand.argument_types) / 4
        #set $n_blocks = $n_blocks + 1 if len($integrator.integrand.argument_types) % 4 else 0
        float4 phase_space[$n_blocks];
        for(size_t p = 0; p < $n_blocks; p++)
        {
            phase_space[p] = ranluxcl32(&ranluxcl_state);
        }

        //Evaluate the phase space point and add it to the sum
        #set $struct_accessors = ["s%i" % i for i in xrange(0, 4)]
        #set $n_args = len($integrator.integrand.argument_types)
        #set $variable_specs = ["phase_space[%i].%s * (%s - %s) + %s" % (i/4, $struct_accessors[i % 4], $integrator.evaluation_function.argument_names[2*i + 1], $integrator.evaluation_function.argument_names[2*i], $integrator.evaluation_function.argument_names[2*i]) for i in xrange(0, $n_args)]
        private_sum += ${integrator.integrand.name}(
            ${",\n".join($variable_specs)}
        );
    }

    //Store the local result
    local_sums[local_id] = private_sum;

    //Make sure everyone stores their results
    barrier(CLK_LOCAL_MEM_FENCE);

    //Upload the random number generator
    ranluxcl_upload_seed(&ranluxcl_state, ranluxcl_states);

    //If this is the first thread, go through and sum
    //the results, storing the answer to global memory
    if(local_id == 0)
    {
        float local_sum = 0.0;
        size_t local_size = get_local_size(0);
        for(size_t i = 0; i < local_size; i++)
        {
            local_sum += local_sums[i];
        }

        atomic_add_float(result, local_sum);
    }
}
