#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtweekend.h"
#include "perlin.h"

// Basic texture with ability to find the colour at a specific point
class texture
{
public:
	virtual colour value(double u, double v, const point3& p) const = 0;
};

// A solid colour texture
class solid_colour : public texture
{
public:
	solid_colour() {}
	solid_colour(colour c) : colour_value(c) {}

	solid_colour(double red, double green, double blue) : solid_colour(colour(red, green, blue)) {}

	virtual colour value(double u, double v, const vec3& p) const override
	{
		return colour_value;
	}

public:
	colour colour_value;
};

// A checkered texture
class checker_texture : public texture
{
public:
	checker_texture() {}

	checker_texture(shared_ptr<texture> _even, shared_ptr<texture> _odd) : even(_even), odd(_odd) {}
	checker_texture(colour c1, colour c2) : even(make_shared<solid_colour>(c1)), odd(make_shared<solid_colour>(c2)) {}

	// Get the colour value at a point
	virtual colour value(double u, double v, const point3& p) const override
	{
		auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
		if (sines < 0)
			return odd->value(u, v, p);
		else
			return even->value(u, v, p);
	}

public:
	shared_ptr<texture> odd;
	shared_ptr<texture> even;
};

// Perlin noise texture
class noise_texture : public texture
{
public:
	noise_texture() {}
	virtual colour value(double u, double v, const point3& p) const override
	{
		return colour(1, 1, 1) * noise.noise(p);
	}

public:
	perlin noise;
};

#endif