//
// SphFluid :
//
#include <smath/sph_fluid.h>

namespace spu {

void SphFluid::init(
        const std::vector<SphFluid::Object *> &objects, SphFluid::Obstacle *obstacle, float smooth_len)
{
	const auto h = m_smoothlen = smooth_len;

	m_bucket.init(h);

	auto particles_count = 0;
	for (auto *object: objects) {
		particles_count += object->m_positions.size();
	}
	m_particles.resize(particles_count);

	auto *p = m_particles.data();
	auto object_id = 0;
	for (auto *object: objects) {
		object->m_sp = p;
		for (auto &position: object->m_positions) {
			p->position = position;
			p->object_id = object_id;
			p++;
		}
		object->m_ep = p;
		m_objects.push_back(object);
		object_id++;
	}

	m_bucket.sort(&m_particles.front(), &m_particles.back());
	for (auto &object: m_objects) {
		if (object->m_type == e_rigid) {
			getNeighbour(object);
		}
	}

	m_obstacle = obstacle;
	precomputeKernelCoeffcients(h);
}

void SphFluid::computeDensity()
{
	auto h2 = m_smoothlen * m_smoothlen;

	for (auto &p0: m_particles) {
		p0.density = 0;  // p1 is always less than p0
		auto mass0 = m_objects[p0.object_id]->m_mass;
		for (auto pp1: p0.neighbours) {
			assert(pp1 <= &p0);

			auto &p1 = *pp1;  // pp1: pointer
			auto delta = p0.position - p1.position;
			auto mass1 = m_objects[p1.object_id]->m_mass;
			auto d2 = dot(delta, delta);

			if (h2 > d2) {
				auto h2_r2 = h2 - d2;
				auto density = h2_r2 * h2_r2 * h2_r2;

				p0.density += mass1 * density;
				if (&p0 != &p1) p1.density += mass0 * density;
			}
		}
	}

	for (auto &p: m_particles) {
		auto density_offset = m_objects[p.object_id]->m_densityOffset;
		p.density *= m_poly6Coef;
		p.pressure = m_pressurePerDensity * std::max(p.density - density_offset, 0.0f);
		p.acceleration = ezero();
	}
}

void SphFluid::computeFluidForce(SphFluid::Object &object)
{
	auto h = m_smoothlen;
	for (auto pp0 = object.m_sp; pp0 < object.m_ep; ++pp0) {
		auto &p0 = *pp0;
		for (auto pp1: p0.neighbours) {
			auto &p1 = *pp1;

			assert(pp1 <= pp0);

			// skip self
			if (pp1 == p0.neighbours[0]) {
				continue;
			}
			auto delta = p0.position - p1.position;
			auto r = length(delta);
			auto mass0 = m_objects[p0.object_id]->m_mass;
			auto mass1 = m_objects[p1.object_id]->m_mass;

			if (r < h) {
				auto h_r = h - r;

				auto pressure = p0.pressure + p1.pressure;
				auto delta_velocity = p1.velocity - p0.velocity;

				auto force = delta * (-0.5f * pressure * m_gradSpikyCoef * h_r / r);
				force = force + delta_velocity * (m_viscosity * m_lapVisCoef);
				force = force * h_r / (p0.density * p1.density);

				p0.acceleration = p0.acceleration + mass1 * force;
				p1.acceleration = p1.acceleration - mass0 * force;
			}
		}
	}
}

void SphFluid::processCollision(float t)
{
	for (auto &p: m_particles) {
		auto stiff = m_objects[p.object_id]->m_stiff;
		auto predicted_position = p.position + t * p.velocity_half;
		auto collisions = m_obstacle->collision(predicted_position, m_collisionSphereRadius);

		for (auto &c: collisions) {
			p.acceleration
			        += (stiff * c.dist - m_collisionDamp * dot(c.normal, p.velocity)) * c.normal;
		}
	}
}

void SphFluid::getNeighbour(SphFluid::Object *object)
{
	for (auto p0 = object->m_sp; p0 != object->m_ep; ++p0) {
		p0->neighbours.clear();
		p0->neighbours.reserve(32);
		p0->neighbours.push_back(p0);  // push self first
		m_bucket.getNeighbour(p0, p0->neighbours);
	}
}

void SphFluid::precomputeKernelCoeffcients(float h)
{
	// Precompute kernel coefficients
	m_poly6Coef = 315.0f / (64.0f * pi() * powf(h, 9));
	m_gradPoly6Coef = 945.0f / (32.0f * pi() * powf(h, 9));
	m_lapPoly6Coef = 945.0f / (32.0f * pi() * powf(h, 9));
	m_gradSpikyCoef = -45.0f / (pi() * h * h * h * h * h * h);
	m_lapVisCoef = +45.0f / (pi() * h * h * h * h * h * h);
}

void SphFluid::computeRigidBodyMotion(float t, SphFluid::Object &object)
{
	const auto sp = object.m_sp;
	const auto ep = object.m_ep;

	std::vector<Vec3f> local_positions;  // relative
	std::vector<Vec3f> local_velocities;

	auto velocity = ezero();
	auto angular_velocity = ezero();
	auto angular_velocity_weight = ezero();

	for (auto p = sp; p != ep; ++p) {
		local_velocities.push_back(p->velocity_half + t * (p->acceleration + m_gravity));
		velocity += local_velocities.back();
	}
	velocity /= object.m_ep - object.m_sp;

	for (auto p = sp; p != ep; ++p) {
		auto d = p->position - object.m_transform.t;
		auto v = local_velocities[p - sp];

		angular_velocity_weight += dot(d, d);
		angular_velocity += cross(d, v);
		local_positions.push_back(d);
	}
	angular_velocity /= angular_velocity_weight;

	object.m_transform.t = object.m_transform.t + t * velocity;
	object.m_transform.q
	        = object.m_transform.q * Quatf(t * length(angular_velocity), normalize(angular_velocity));

	for (auto i = 0u; i < local_positions.size(); i++) {
		auto &p = *(sp + i);
		auto velocity_half = cross(angular_velocity, local_positions[i]) + velocity;
		p.velocity = (p.velocity_half + velocity_half) * 0.5;
		p.velocity_half = velocity_half;
		p.position = object.m_transform * object.m_positions[i];
	}
}

void SphFluid::computeFluidMotion(float t, SphFluid::Object &object)
{
	for (auto p = object.m_sp; p != object.m_ep; ++p) {
		auto velocity_half = p->velocity_half + t * (p->acceleration + m_gravity);
		p->position = p->position + t * velocity_half;
		p->velocity = p->velocity_half + velocity_half;
		p->velocity = 0.5f * p->velocity;
		p->velocity_half = velocity_half;
	}
}

void SphFluid::update(float t, int32_t loop_count)
{
	m_bucket.sort(&m_particles.front(), &m_particles.back());
	for (auto *object: m_objects) {
		if (object->m_type == e_fluid) {
			getNeighbour(object);
		}
	}

	for (auto i = 0; i < loop_count; i++) {
		computeDensity();

		for (auto *object: m_objects) {
			if (object->m_type == e_fluid) {
				computeFluidForce(*object);
			}
		}

		processCollision(t);

		for (auto *object: m_objects) {
			if (object->m_type == e_rigid) {
				computeRigidBodyMotion(t, *object);
			}
			else if (object->m_type == e_fluid) {
				computeFluidMotion(t, *object);
			}
		}
		// break; // debug
	}
}
}  // namespace spu
