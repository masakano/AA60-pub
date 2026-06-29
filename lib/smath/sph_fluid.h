//
// SphFluid :
//
#pragma once

#include "bucket_sort.h"
#include "quatf.h"

namespace spu {

class SphFluid {
public:
	enum {
		e_rigid = 0,
		e_fluid = 1,
	};

	struct Particle {
		Vec3f position = ezero();            // Position
		Vec3f velocity = ezero();            // Velocity
		Vec3f velocity_half = ezero();       // Velocity half steps forwarded
		Vec3f acceleration = ezero();        // Acceleration
		float density = 0.0f;                // Density and its reciprocal at particle location
		float pressure = 0.0f;               // Pressure at particle location
		int32_t object_id = 0;               // object ID
		std::vector<Particle *> neighbours;  // neighbourhood particles

		explicit operator Vec3f() const { return position; }
	};

	struct Collision {
		Vec3f normal;
		float dist;
	};

	class Object {
	public:
		int32_t m_type;
		float m_mass;
		float m_stiff;
		float m_densityOffset;

		Particle *m_sp = nullptr;
		Particle *m_ep = nullptr;

		Transformf m_transform;
		std::vector<Vec3f> m_positions;
	};

	class Obstacle {
	public:
		virtual ~Obstacle() {}
		virtual void setNodeworld(const Mat4f &m) = 0;
		virtual std::vector<Collision> collision(const Vec3f &position, const float radius) = 0;
	};

	void init(const std::vector<Object *> &objects, Obstacle *obstacle, float smooth_len = 0.01f);
	void update(float t, int32_t loop_count = 2);

	const std::vector<Object *> &objects() const { return m_objects; }
	const Obstacle *obstacle() const { return m_obstacle; }

private:
	std::vector<Object *> m_objects;
	Obstacle *m_obstacle = nullptr;
	std::vector<Particle> m_particles;
	spu::BucketSort<Particle> m_bucket;

	const Vec3f m_gravity = {0.0f, -9.8f, 0.0f};
	const float m_viscosity = 0.2f;
	const float m_pressurePerDensity = 1.5f;  // Stiffness (need check)

	const float m_collisionDamp = 128.0f;
	const float m_collisionSphereRadius = 0.004f;

	float m_smoothlen;
	float m_poly6Coef;
	float m_lapPoly6Coef;
	float m_gradPoly6Coef;
	float m_gradSpikyCoef;
	float m_lapVisCoef;

	void computeDensity();
	void computeFluidForce(Object &object);
	void processCollision(float t);
	void computeRigidBodyMotion(float t, Object &object);
	void computeFluidMotion(float t, Object &object);
	void getNeighbour(Object *object);
	void precomputeKernelCoeffcients(float h);
};
}  // namespace spu
