#include "integrands.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338328
#endif

float unit_cylinder(float x, float y)
{
    //Cylinder with height 1.0 and radius
    //1.0 with bottom face's center at
    //the origin.
    float r = sqrt(pow(x, 2) + pow(y, 2));
    if(r <= 1.0)
    {
        //Inside the unit circle
        return 1.0;
    }

    //Outside the unit circle
    return 0.0;
}

double random_walk(float x, float y, float z)
{
    double result = 1.0/(M_PI * M_PI * M_PI);
    return result/(1.0 - cos(x) * cos(y) * cos(z));
}
