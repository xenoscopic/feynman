__kernel void miser_integrate(
    int points_per_worker,
    __global ranluxcl_state_t *ranluxcl_states,
    __global float *result,
    $integrator.evaluation_function.argument_signature
    )
{
    
}
