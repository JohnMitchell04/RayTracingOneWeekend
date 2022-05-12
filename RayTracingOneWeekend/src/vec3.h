#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

using std::sqrt;

// 3-dimensional vector class
class vec3
{
public:
	// Create an empty vec3
	vec3() : e{ 0,0,0 } {}
	// Create a vec3 with values
	vec3(double e0, double e1, double e2) : e{ e0, e1, e2 } {}

	// Methods for getting x, y and z of a vec 3
	double x() const { return e[0]; }
	double y() const { return e[1]; }
	double z() const { return e[2]; }

	// Determines if vector is close to 0
	bool near_zero() const 
	{
		const auto s = 1e-8;
		return (fabs(e[0] < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s));
	}

	inline static vec3 random()
	{
		return vec3(random_double(), random_double(), random_double());
	}

	inline static vec3 random(double min, double max)
	{
		return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
	}

	// Get the negative of a vector
	vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
	// Get the value in specified position as a constant
	double operator[](int i) const { return e[i]; }
	// Get a pointer to the value at the specified position
	double& operator[](int i) { return e[i]; }

	// Multiplication of a vector by a real number
	vec3& operator*=(const double t)
	{
		e[0] *= t;
		e[1] *= t;
		e[2] *= t;
		return *this;
	}

	// Addition of two vectors
	vec3& operator+=(const vec3& v) {
		e[0] += v.e[0];
		e[1] += v.e[1];
		e[2] += v.e[2];
		return *this;
	}

	// Division of a vector by a 
	vec3& operator/=(const double t)
	{
		return *this *= 1 / t;
	}

	// Return the length of the vector
	double length() const 
	{
		return sqrt(length_squared());
	}

	// Return the squared length of the vector
	double length_squared() const 
	{
		return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
	}

public:
	double e[3];
};

// Type aliases for vec3
using point3 = vec3;   // 3D point
using colour = vec3;    // RGB color


// vec3 utility functions

// Writes the values of the vec3 to the stream
inline std::ostream& operator<<(std::ostream& out, const vec3& v)
{
	return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

// Return the addition of two vec3 as a vec3
inline vec3 operator+(const vec3& u, const vec3& v) 
{
	return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

// Return the subtraction of two vec3 as a vec3
inline vec3 operator-(const vec3& u, const vec3& v)
{
	return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

// Return the multiplication of two vec3 as a vec3
inline vec3 operator*(const vec3& u, const vec3& v)
{
	return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

// Return the multiplication of a vec3 and a double as a vec3
inline vec3 operator*(double t, const vec3& v)
{
	return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}


// Return the multiplication of a vec3 and a double as a vec3
inline vec3 operator*(const vec3& v, double t)
{
	return t * v;
}

// Return the division of a vec3 by a double as a vec3
inline vec3 operator/(vec3 v, double t)
{
	return (1 / t) * v;
}

// Return the dot product of two vec3 as a double
inline double dot(const vec3& u, const vec3& v)
{
	return u.e[0] * v.e[0]
		+ u.e[1] * v.e[1]
		+ u.e[2] * v.e[2];
}

// Return the cross product of two vec 3 as a new vec3
inline vec3 cross(const vec3& u, const vec3& v)
{
	return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
		u.e[2] * v.e[0] - u.e[0] * v.e[2],
		u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

// Return the unit vector of a vec3 as a new vec3
inline vec3 unit_vector(vec3 v)
{
	return v / v.length();
}

// Return a random point in a unit sphere as a vec3
inline vec3 random_in_unit_sphere()
{
	while (true)
	{
		auto p = vec3::random(-1, 1);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

// Return a random unit vector as a vec3
inline vec3 random_unit_vector()
{
	return unit_vector(random_in_unit_sphere());
}

// Return the direction of a reflacted as a vec3
inline vec3 reflect(const vec3& v, const vec3& n)
{
	return v - 2 * dot(v, n) * n;
}

// Return the direction of a refracted ray as a vec3
inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat)
{
	auto cos_theta = fmin(dot(-uv, n), 1.0);
	vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
	vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
	return r_out_perp + r_out_parallel;
}

// Return a random vec 3 within a disc as a vec3
inline vec3 random_in_unit_disc()
{
	while (true)
	{
		auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

#endif