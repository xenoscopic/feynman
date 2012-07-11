__kernel void plain_integrate(
    $integrator.evaluation_function.argument_signature,
    int points_per_worker,
    __global ranluxcl_state_t *ranluxcl_states,
    __global $integrator.evaluation_function.return_type *result
    )
{
    //The local (workgroup-shared) array where final results
    //will be stored and summed.
    __local $integrator.evaluation_function.return_type results;

    //The work-item-local result variable.
    $integrator.evaluation_function.return_type _result;

    //Download the random number generator
    ranluxcl_state_t ranluxclstate;
    ranluxcl_download_seed(&ranluxclstate, ranluxcl_states);

    //Loop over and evaluate random phase-space points.
    for(int i = 0; i < points_per_worker; i++)
    {

    }

    //Store the local result


    //Upload the random number generator
    ranluxcl_upload_seed(&ranluxclstate, ranluxcl_states);


    //If this is the first thread, go through and sum
    //the results, storing the answer to global memory

}
