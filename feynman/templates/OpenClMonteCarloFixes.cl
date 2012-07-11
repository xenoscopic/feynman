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

#endif //OPENCL_MONTE_CARLO_FIXES_CL
