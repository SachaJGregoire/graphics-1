#define _CRT_SECURE_NO_WARNINGS 1
#include <vector>
#include <cmath>
#include <random>
#include <omp.h>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef M_PI
#define M_PI 3.14159265358979323856
#endif

static std::default_random_engine engine[32];
static std::uniform_real_distribution<double> uniform(0, 1);

double sqr(double x) { return x * x; };

class Vector {
public:
	explicit Vector(double x = 0, double y = 0, double z = 0) {
		data[0] = x;
		data[1] = y;
		data[2] = z;
	}
	double norm2() const {
		return data[0] * data[0] + data[1] * data[1] + data[2] * data[2];
	}
	double norm() const {
		return sqrt(norm2());
	}
	void normalize() {
		double n = norm();
		data[0] /= n;
		data[1] /= n;
		data[2] /= n;
	}
	Vector normalized() {
		this->normalize();
		return *this;
	}
	double operator[](int i) const { return data[i]; };
	double& operator[](int i) { return data[i]; };
	double data[3];
};

Vector operator+(const Vector& a, const Vector& b) {
	return Vector(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}
Vector operator-(const Vector& a, const Vector& b) {
	return Vector(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}
Vector operator*(const Vector& a, const Vector& b) {
	return Vector(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
}
Vector operator*(const double a, const Vector& b) {
	return Vector(a*b[0], a*b[1], a*b[2]);
}
Vector operator*(const Vector& a, const double b) {
	return Vector(a[0]*b, a[1]*b, a[2]*b);
}
Vector operator/(const Vector& a, const double b) {
	return Vector(a[0] / b, a[1] / b, a[2] / b);
}
double dot(const Vector& a, const Vector& b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
Vector cross(const Vector& a, const Vector& b) {
	return Vector(a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]);
}

class Ray {
public:
	Ray(const Vector& origin, const Vector& unit_direction) : O(origin), u(unit_direction) {};
	Vector O, u;
};

class Object {
public:
	Object(const Vector& albedo, bool mirror = false, bool transparent = false) : albedo(albedo), mirror(mirror), transparent(transparent) {};

	virtual bool intersect(const Ray& ray, Vector& P, double& t, Vector& N) const = 0;

	Vector albedo;
	bool mirror, transparent;
};

class Sphere : public Object {
public:
	Sphere(const Vector& center, double radius, const Vector& albedo, bool mirror = false, bool transparent = false) : ::Object(albedo, mirror, transparent), C(center), R(radius) {};

	// returns true iif there is an intersection between the ray and the sphere
	// if there is an intersection, also computes the point of intersection P, 
	// t>=0 the distance between the ray origin and P (i.e., the parameter along the ray)
	// and the unit normal N
	bool intersect(const Ray& ray, Vector& P, double &t, Vector& N) const {
		// DONE (lab 1) : compute the intersection (just true/false at the begining of lab 1, then P, t and N as well)
		double delta = sqr(dot(ray.u, ray.O - C)) - ((ray.O - C).norm2() - sqr(R));
		if (delta < 0) return false;
		double base = dot(ray.u, C - ray.O);
		double sqrtdelta = sqrt(delta);
		double t1 = base - sqrtdelta;
		if (t1 >= 0) {
			t = t1;
			P = ray.O + ray.u * t;
			N = (P - C).normalized();
			return true;
		}
		double t2 = base + sqrtdelta;
		if (t2 >= 0) {
			t = t2;
			P = ray.O + ray.u * t;
			N = (P - C).normalized();
			return true;
		}
		return false;
	}

	double R;
	Vector C;
};


// I will provide you with an obj mesh loader (labs 3 and 4)
class TriangleMesh : public Object {
public:
	TriangleMesh(const Vector& albedo, bool mirror = false, bool transparent = false) : ::Object(albedo, mirror, transparent) {};

	bool intersect(const Ray& ray, Vector& P, double& t, Vector& N) const {
		// TODO (labs 3 and 4)
		return false;
	}
};


class Scene {
public:
	Scene() {};
	void addObject(const Object* obj) {
		objects.push_back(obj);
	}

	// returns true iif there is an intersection between the ray and any object in the scene
    // if there is an intersection, also computes the point of the *nearest* intersection P, 
    // t>=0 the distance between the ray origin and P (i.e., the parameter along the ray)
    // and the unit normal N. 
	// Also returns the index of the object within the std::vector objects in object_id
	bool intersect(const Ray& ray, Vector& P, double& t, Vector& N, int &object_id) const  {
		// DONE (lab 1): iterate through the objects and check the intersections with all of them, 
		// and keep the closest intersection, i.e., the one if smallest positive value of t
		t = std::numeric_limits<double>::max();
		Vector P_temp;
		double t_temp;
		Vector N_temp;
		for (int i = 0; i < objects.size(); i++) {
			if (objects[i]->intersect(ray, P_temp, t_temp, N_temp)) {
				if (t_temp < t) {
					P = P_temp;
					t = t_temp;
					N = N_temp;
					object_id = i;
				}
			}
		}
		return (t != std::numeric_limits<double>::max());
	}


	// return the radiance (color) along ray
	Vector getColor(const Ray& ray, int recursion_depth) {

		if (recursion_depth >= max_light_bounce) return Vector(0, 0, 0);

		// DONE (lab 1) : if intersect with ray, use the returned information to compute the color ; otherwise black 
		// in lab 1, the color only includes direct lighting with shadows

		Vector P, N;
		double t;
		int object_id;
		if (intersect(ray, P, t, N, object_id)) {
			double epsilon = 0.00000000001;
			P = P + epsilon * N;
			if (objects[object_id]->mirror) {
				// return getColor in the reflected direction, with recursion_depth+1 (recursively)
				Vector reflected_ray = (ray.u - 2 * dot(ray.u, N) * N).normalized();
				return getColor(Ray(P, reflected_ray), recursion_depth + 1);
			} // else

			if (objects[object_id]->transparent) { // TODO: fix (optional)
				// return getColor in the refraction direction, with recursion_depth+1 (recursively)
				double n1 = 1.003;
				double n2 = 1.330;
				Vector tTT = n1 / n2 * (ray.u - dot(ray.u, N) * N);
				double tN  = -sqrt(1 - sqr(n1/n2) * (1 - sqr(dot(ray.u, N))));
				Vector refracted_ray = (tTT + tN * N).normalized();
				return getColor(Ray(P, refracted_ray), recursion_depth + 1);
			} // else

			// test if there is a shadow by sending a new ray
			// if there is no shadow, compute the formula with dot products etc.
			Vector P_temp = P;
			double t_temp = t;
			Vector N_temp = N;
			int object_id_temp = object_id;
			Vector shadow_ray = (light_position - P).normalized();
			if (intersect(Ray(P, shadow_ray), P_temp, t_temp, N_temp, object_id_temp)) {
				if ((P_temp - P).norm2() <= (light_position - P).norm2()) return Vector(0, 0, 0);
			}

			double attenuation 	= light_intensity / (4 * M_PI * (light_position - P).norm2());
			Vector material 	= objects[object_id]->albedo / M_PI;
			double solid_angle	= dot(N, (light_position - P).normalized());
			if (solid_angle < 0) solid_angle = 0;
			Vector direct = attenuation * material * solid_angle;

			// TODO (lab 2) : add indirect lighting component with a recursive call
			int tid = omp_get_thread_num();
			double r1 = uniform(engine[tid]);
			double r2 = uniform(engine[tid]);
			double x = cos(2.0 * M_PI * r1) * sqrt(1 - r2);
			double y = sin(2.0 * M_PI * r1) * sqrt(1 - r2);
			double z = sqrt(r2);

			Vector T1;
			double min_val = std::min(std::min(abs(N[0]), abs(N[1])), abs(N[2]));
			if 		(min_val == abs(N[0])) { T1 = Vector(0, -N[2], N[1]); }
			else if (min_val == abs(N[1])) { T1 = Vector(N[2], 0, -N[0]); }
			else if (min_val == abs(N[2])) { T1 = Vector(-N[1], N[0], 0); }

			T1.normalize();
			Vector T2 = cross(N, T1);
			Vector wi = x * T1 + y * T2 + z * N;
			Ray indirect_ray(P, wi);
			Vector indirect = objects[object_id]->albedo * getColor(indirect_ray, recursion_depth + 1);

			return direct + indirect;
		}

		

		return Vector(0, 0, 0);
	}

	std::vector<const Object*> objects;

	Vector camera_center, light_position;
	double fov, gamma, light_intensity;
	int max_light_bounce;
};


int main() {
	int W = 512;
	int H = 512;

	for (int i = 0; i<32; i++) {
		engine[i].seed(i);
	}

	Sphere center_sphere(Vector(0, 0, 0), 10., Vector(0.8, 0.8, 0.8));
	center_sphere.transparent = false;
	Sphere wall_left(Vector(-1000, 0, 0), 940, Vector(0.5, 0.8, 0.1));
	Sphere wall_right(Vector(1000, 0, 0), 940, Vector(0.9, 0.2, 0.3));
	Sphere wall_front(Vector(0, 0, -1000), 940, Vector(0.1, 0.6, 0.7));
	Sphere wall_behind(Vector(0, 0, 1000), 940, Vector(0.8, 0.2, 0.9));
	Sphere ceiling(Vector(0, 1000, 0), 940, Vector(0.3, 0.5, 0.3));
	Sphere floor(Vector(0, -1000, 0), 990, Vector(0.6, 0.5, 0.7));

	Scene scene;
	scene.camera_center = Vector(0, 0, 55);
	scene.light_position = Vector(-10,20,40);
	scene.light_intensity = 3E7;
	scene.fov = 60 * M_PI / 180.;
	scene.gamma = 2.2;    // DONE (lab 1) : play with gamma ; typically, gamma = 2.2
	scene.max_light_bounce = 5;

	scene.addObject(&center_sphere);

	scene.addObject(&wall_left);
	scene.addObject(&wall_right);
	scene.addObject(&wall_front);
	scene.addObject(&wall_behind);
	scene.addObject(&ceiling);
	scene.addObject(&floor);

	std::vector<unsigned char> image(W * H * 3, 0);

	double z = -W/(2*tan(scene.fov/2));
	#pragma omp parallel for schedule(dynamic, 1)
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			Vector color(0, 0, 0);

			// DONE (lab 1) : correct ray_direction so that it goes through each pixel (j, i)
			Vector ray_direction(j - W/2 + 0.5, H/2 - i - 0.5, z);
			ray_direction.normalize();

			Ray ray(scene.camera_center, ray_direction);

			// TODO (lab 2) : add Monte Carlo / averaging of random ray contributions here
			// TODO (lab 2) : add antialiasing by altering the ray_direction here
			int num_rays = 10;
			double sigma = 0.5;
			int tid = omp_get_thread_num();
			for (int k = 0; k < num_rays; k++) {
				double r1 = uniform(engine[tid]);
				double r2 = uniform(engine[tid]);
				double x = j - W/2 + 0.5 + sigma * sqrt(-2.0 * log(r1)) * cos(2.0 * M_PI * r2);
				double y = H/2 - i - 0.5 + sigma * sqrt(-2.0 * log(r1)) * sin(2.0 * M_PI * r2);
				Vector new_direction(x, y, z);
				new_direction.normalize();
				Ray new_ray(scene.camera_center, new_direction);
				color = color + scene.getColor(new_ray, 0);
			}
			color = color / num_rays;

			// TODO (lab 2) : add depth of field effect by altering the ray origin (and direction) here (optional)


			image[(i * W + j) * 3 + 0] = std::min(255., std::max(0., 255. * std::pow(color[0] / 255., 1. / scene.gamma)));
			image[(i * W + j) * 3 + 1] = std::min(255., std::max(0., 255. * std::pow(color[1] / 255., 1. / scene.gamma)));
			image[(i * W + j) * 3 + 2] = std::min(255., std::max(0., 255. * std::pow(color[2] / 255., 1. / scene.gamma)));
		}
	}
	stbi_write_png("image.png", W, H, 3, &image[0], 0);

	return 0;
}