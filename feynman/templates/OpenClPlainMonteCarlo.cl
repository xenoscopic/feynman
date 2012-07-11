$integrator.integrand.text

__kernel void integrate(
    ${", ".join($integrator.evaluation_function.argument_names)},
    int points_per_worker,
    __global float *result
    )
{

}
