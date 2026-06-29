//
// GsBullet :
//
#pragma once

#include <smath/range.h>
#include <smath/quatf.h>
#include <smath/bone3f.h>

class btVector3;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btAxisSweep3;
class btDiscreteDynamicsWorld;

namespace spu {

class GsBtRigidBody;
class GsBtConstraint;

class GsBullet {
public:
	struct RigidBodyDesc {
		enum {
			e_sphere = 0,
			e_box,
			e_capsule,
		};

		void *node;
		union {
			int32_t index;
			Bone3f *bone;
		};

		uint8_t shape_type;
		uint8_t group_index;
		uint16_t group_mask;

		float mix_rate;
		float mass;
		float linear_dumping;
		float angular_dumping;
		float restitution;
		float friction;
		Vec3f shape_size;
		Transformf transform;
	};

	struct ConstraintDesc {
		enum {
			e_spring_dof6,
			e_dof6,
			e_p2p,
			e_cone_twist,
			e_slider,
			e_hinge,
		};

		void *node;
		int32_t indexA;
		int32_t indexB;
		uint8_t spring_type;  // fixed: 0
		Transformf transform;
		Vec3f linear_lower_limit;
		Vec3f linear_upper_limit;
		Vec3f angular_lower_limit;
		Vec3f angular_upper_limit;
		Vec3f translate_stiffness;
		Vec3f rotate_stiffness;
	};

	struct RigidBodyGeometry {
		int32_t shape_type;
		float mix_rate;
		Range3f range;
		Transformf center_of_mass = Transformf(ezero());
	};

	struct ConstraintGeometry {
		RigidBodyGeometry geometryA;
		RigidBodyGeometry geometryB;
	};

	~GsBullet() { deleteWorld(); }

	void newWorld(float world_size, float gravity);
	void deleteWorld();
	void removeNode(void *node);
	void addPlane(const Vec3f &normal, float d, float restitution);

	void addStaticMesh(
	        int32_t triangle_count, int32_t *index_base, int32_t index_stride, int32_t vertex_count,
	        float *vertex_base, int32_t vertex_stride);

	GsBtRigidBody *addRigidBody(const RigidBodyDesc &desc);
	GsBtConstraint *addConstraint(const ConstraintDesc &desc);

	void reset();
	void applyCentralImpulse(const void *bone, const Vec3f &dir);
	void update();
	void view(const std::vector<Mat4f> &nodeworlds = {Mat4f()}) const;

	Range3f getRange(const void *node) const;
	uint32_t getRigidBodyCount() const;
	uint32_t getConstraintCount() const;

	std::vector<RigidBodyGeometry> getRigidBodyGeometries() const;
	std::vector<ConstraintGeometry> getConstraintGeometries() const;

	static void resetAll();
	static void updateAll();

private:
	friend class GsBtRigidBody;
	friend class GsBtConstraint;

	static std::unordered_set<GsBullet *> &ms_bullets()
	{
		static std::unordered_set<GsBullet *> v;
		return v;
	}

	void initBullet(const btVector3 &bt_world_min, const btVector3 &bt_world_max, float gravity);
	void disposeBullet();

	btDefaultCollisionConfiguration *m_btCollisionConfig = nullptr;
	btCollisionDispatcher *m_btCollisionDispatcher = nullptr;
	btSequentialImpulseConstraintSolver *m_btSolver = nullptr;
	btAxisSweep3 *m_btAxisSweep3 = nullptr;
	btDiscreteDynamicsWorld *m_btWorld = nullptr;

	std::vector<GsBtRigidBody *> m_btRigidBodies;
	std::vector<GsBtConstraint *> m_btConstraints;

	bool m_doReset = false;
	Seconds m_seconds;
};
}  // namespace spu
