kernel void random_initialize(__private uint seed, 
                              __global ranluxcl_state_t *ranluxcltab)
{
    ranluxcl_initialization(seed, ranluxcltab);
}
