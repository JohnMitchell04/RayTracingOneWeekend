#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

// A sphere class with a centre and radius that is hittable
class sphere : public hittable
{
public:
	sphere(point3 cen, double r, shared_ptr<material> m) : centre(cen), radius(r), mat_ptr(m) {}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

public:
	point3 centre;
	double radius;
	shared_ptr<material> mat_ptr;

private:
	static void get_sphere_uv(const point3& p, double& u, double& v)
	{
		auto theta = acos(-p.y());
		auto phi = atan2(-p.z(), p.x()) + pi;

		u = phi / (2 * pi);
		v = theta / pi;
	}
};

// The hit function for a sphere
bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	// Basic math to find if ray hits sphere
	vec3 oc = r.origin() - centre;
	auto a = r.direction().length_squared();
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius * radius;

	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0) return false;
	auto sqrtd = sqrt(discriminant);

	// Find nearest root that lies in acceptable range
	auto root = (-half_b - sqrtd) / a;
	if (root < t_min || t_max < root)
	{
		root = (-half_b + sqrtd) / a;
		if (root < t_min || t_max < root)
			return false;
	}

	rec.t = root;
	rec.p = r.at(rec.t);
	vec3 outward_normal = (rec.p - centre) / radius;
	rec.set_face_normal(r, outward_normal);
	get_sphere_uv(outward_normal, rec.u, rec.v);
	rec.mat_ptr = mat_ptr;

	return true;
}

// Create a boudning box around a sphere
bool sphere::bounding_box(double time0, double time1, aabb& output_box) const
{
	output_box = aabb(centre - vec3(radius, radius, radius), centre + vec3(radius, radius, radius));
	return true;
}

#endif