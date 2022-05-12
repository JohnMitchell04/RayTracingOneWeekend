#ifndef RAY_H
#define RAY_H

#include "vec3.h"

// Ray class (esentially a line in 3-dimensions)
class ray 
{
public:
	ray() {}
	// Create a ray with an origin and direction
	ray(const point3& origin, const vec3& direction, double time = 0.0) : orig(origin), dir(direction), tm(time) {}

	// Get the origin of a ray
	point3 origin() const { return orig; }
	// Get the direction of a ray
	vec3 direction() const { return dir; }
	// Get the time ray was created
	double time() const{ return tm; }

	// Get the point on a ray t away from the origin
	point3 at(double t) const 
	{
		return orig + t * dir;
	}

public:
	point3 orig;
	vec3 dir;
	double tm;
};

#endif