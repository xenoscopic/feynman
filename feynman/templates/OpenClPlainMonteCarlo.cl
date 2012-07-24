#define MAX_PLAIN_MONTE_CARLO_WORK_GROUP_SIZE 512

__kernel void plain_integrate(
    unsigned int points_per_worker,
    __global ranluxcl_state_t *ranluxcl_states,
    __global float *result,
    $integrator.evaluation_function.argument_signature
    )
{
    //The local (workgroup-shared) array where final results
    //will be stored and summed.
    __local float local_sums[MAX_PLAIN_MONTE_CARLO_WORK_GROUP_SIZE];
    __local float local_square_sums[MAX_PLAIN_MONTE_CARLO_WORK_GROUP_SIZE];

    //Thread-local variables
    unsigned int local_id = get_local_id(0);
    float private_sum = 0.0;
    float private_square_sum = 0.0;

    //Download the random number generator
    ranluxcl_state_t ranluxcl_state;
    ranluxcl_download_seed(&ranluxcl_state, ranluxcl_states);

    //Loop over and evaluate random phase-space points.
    for(unsigned int i = 0; i < points_per_worker; i++)
    {
        //Generate a random phase-space point
        #set $n_blocks = len($integrator.integrand.argument_types) / 4
        #set $n_blocks = $n_blocks + 1 if len($integrator.integrand.argument_types) % 4 else 0
        float4 phase_space[$n_blocks];
        for(unsigned int p = 0; p < $n_blocks; p++)
        {
            phase_space[p] = ranluxcl32(&ranluxcl_state);
        }

        //Evaluate the phase space point and add it to the sum
        #set $struct_accessors = ["s%i" % i for i in xrange(0, 4)]
        #set $n_args = len($integrator.integrand.argument_types)
        #set $variable_specs = ["phase_space[%i].%s * (%s - %s) + %s" % (i/4, $struct_accessors[i % 4], $integrator.evaluation_function.argument_names[2*i + 1], $integrator.evaluation_function.argument_names[2*i], $integrator.evaluation_function.argument_names[2*i]) for i in xrange(0, $n_args)]
        float value = ${integrator.integrand.name}(
            ${",\n".join($variable_specs)}
        );
        private_sum += value;
        private_square_sum += (value * value);
    }

    //Store the local result
    local_sums[local_id] = private_sum;
    local_square_sums[local_id] = private_square_sum;

    //Make sure everyone stores their results
    barrier(CLK_LOCAL_MEM_FENCE);

    //Upload the random number generator
    ranluxcl_upload_seed(&ranluxcl_state, ranluxcl_states);

    //If this is the first thread, go through and sum
    //the results, storing the answer to global memory
    if(local_id == 0)
    {
        float local_sum = 0.0;
        float local_square_sum = 0.0;
        unsigned int local_size = get_local_size(0);
        for(unsigned int i = 0; i < local_size; i++)
        {
            local_sum += local_sums[i];
            local_square_sum += local_square_sums[i];
        }

        atomic_add_float(result, local_sum);
        atomic_add_float(result + 1, local_square_sum);
    }
}
