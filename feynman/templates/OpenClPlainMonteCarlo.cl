$integrator.integrand.text

__kernel void integrate(
    $integrator.evaluation_function.argument_signature,
    int points_per_worker,
    __global $integrator.evaluation_function.return_type *result
    )
{

}
