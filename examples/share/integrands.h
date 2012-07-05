#ifndef INTEGRANDS_H
#define INTEGRANDS_H

/* 
 * The unit cylinder which has once of its
 * flat faces in the XY plane, centered at
 * the origin, and which has radius and 
 * height 1.0  The volume of the cylinder
 * should be pi.
 */
float unit_cylinder(float x, float y);

/*
 * The function
 * 
 *  (1 / (2 * pi)^3) * (1 / (1 - (cos(x) * cos(y) * cos(z))))
 * 
 * which appears in the theory of random walks.
 * When integrated from (-pi, -pi, -pi) to
 * (pi, pi, pi), the exact answer (obtained
 * analytically) is
 * 
 *  (Gamma(1 / 4)^4) / (4 * (pi^3))
 */
double random_walk(float x, float y, float z);

#endif //INTEGRANDS_H
