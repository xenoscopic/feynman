__kernel void vegas_integrate(
    cl_uint points_per_worker,
    __global ranluxcl_state_t *ranluxcl_states,
    __global float *result,
    $integrator.evaluation_function.argument_signature
    )
{
    
}
