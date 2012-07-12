#ifndef OPENCL_MONTE_CARLO_FIXES_CL
#define OPENCL_MONTE_CARLO_FIXES_CL

/* 
 * A collection of macros to help normalize differet
 * OpenCL implementations to the point where they are
 * easily suitable for scientific computation.
 */

#ifndef M_PI
#define M_PI M_PI_F
#endif //M_PI

//Forward declaration to make Apple's compiler happy
inline void atomic_add_float(volatile __global float *source, const float operand);

inline void atomic_add_float(volatile __global float *source, const float operand)
{
    union
    {
        unsigned int int_value;
        float float_value;
    } new_value;
    union
    {
        unsigned int int_value;
        float float_value;
    } prev_value;
    do
    {
        prev_value.float_value = *source;
        new_value.float_value = prev_value.float_value + operand;
    }
    while(atomic_cmpxchg((volatile __global unsigned int *)source, prev_value.int_value, new_value.int_value) != prev_value.int_value);
}

#endif //OPENCL_MONTE_CARLO_FIXES_CL
