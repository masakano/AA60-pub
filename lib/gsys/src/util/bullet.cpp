//
// GsBullet :
//
#include <btBulletDynamicsCommon.h>
#include <gsys/painter/stdout.h>
#include <gsys/util/bullet.h>

namespace spu {
static constexpr int32_t c_max_proxies = 16384;

// namespace bullet {

btVector3 make_btvector3() { return {0, 0, 0}; }

btVector3 make_btvector3(const Vec3f &v) { return {v.x, v.y, v.z}; }

Vec3f make_vec3f(const btVector3 &v) { return {v.x(), v.y(), v.z()}; }

btQuaternion make_btquaternion() { return {0, 0, 0, 1}; }

btQuaternion make_btquaternion(const Quatf &q) { return {q.x, q.y, q.z, q.w}; }

Quatf make_quatf(const btQuaternion &q) { return {q.x(), q.y(), q.z(), q.w()}; }

btTransform make_bttransform() { return btTransform(btQuaternion(0, 0, 0, 1)); }

btTransform make_bttransform(const Transformf &tr)
{
	return btTransform(make_btquaternion(tr.q), make_btvector3(tr.t));
}

Transformf make_transformf(const btTransform &tr)
{
	return {make_vec3f(tr.getOrigin()), make_quatf(tr.getRotation())};
}

class GsBtRigidBody : public btRigidBody {
public:
	GsBtRigidBody(GsBullet *bt_world, const GsBullet::RigidBodyDesc &desc);
	~GsBtRigidBody() override;

	void restart();
	GsBullet::RigidBodyGeometry getRigidBodyGeometry() const;
	const void *node() const { return m_node; }
	const void *bone() const { return m_bone; }

private:
	void *m_node = nullptr;
	void *m_bone = nullptr;
	float m_mix_rate = 0.0;
	btRigidBody::btRigidBodyConstructionInfo createInfo(
	        GsBullet *bt_world, const GsBullet::RigidBodyDesc &desc);
};

class GsBtConstraint : public btGeneric6DofSpringConstraint {
public:
	GsBtConstraint(GsBullet *bt_world, const GsBullet::ConstraintDesc &desc);
	~GsBtConstraint() override {}
	GsBullet::ConstraintGeometry getConstraintGeometry() const;
	const void *node() const { return m_node; }

private:
	void *m_node = nullptr;
	GsBtRigidBody *m_btBodyA = nullptr;
	GsBtRigidBody *m_btBodyB = nullptr;
};

class GsBtMotionState : public btMotionState {
public:
	GsBtMotionState(Bone3f *bone, const btTransform &bt_massbone, float weight)
	        : m_bone(bone), m_btMassbone(bt_massbone), m_btBonemass(m_btMassbone.inverse()),
	          m_weight(weight)
	{
	}

	void getWorldTransform(btTransform &bt_massbullet) const override
	{
		bt_massbullet = make_bttransform(m_bone->boneworld()) * m_btMassbone;
	}

	void setWorldTransform(const btTransform &bt_massbullet) override
	{
		auto bt_boneworld = bt_massbullet * m_btBonemass;
		m_bone->setBoneworld(lerp(m_bone->boneworld(), make_transformf(bt_boneworld), m_weight));
	}

private:
	Bone3f *m_bone = nullptr;
	const btTransform m_btMassbone;
	const btTransform m_btBonemass;
	const float m_weight = 0;
};

Range3f GsBullet::getRange(const void *node) const
{
	Range3f range;
	range.invalidate();
	for (auto &bt_body: m_btRigidBodies) {
		if (bt_body->node() == node) {
			btTransform bt_center_of_mass;
			bt_body->getMotionState()->getWorldTransform(bt_center_of_mass);

			btVector3 bt_min;
			btVector3 bt_max;
			bt_body->getCollisionShape()->getAabb(bt_center_of_mass, bt_min, bt_max);
			range.expand(make_vec3f(bt_min));
			range.expand(make_vec3f(bt_max));
		}
	}
	return range;
}

GsBtRigidBody *GsBullet::addRigidBody(const GsBullet::RigidBodyDesc &desc)
{
	auto *bt_body = new GsBtRigidBody(this, desc);
	m_btRigidBodies.push_back(bt_body);

	aux_error(
	        m_btRigidBodies.size() >= c_max_proxies, "max proxy (%ld) overflow\n", m_btRigidBodies.size());

	if (desc.group_index == 0 && desc.group_mask == 0) {
		m_btWorld->addRigidBody(bt_body);  // use default
	}
	else {
		m_btWorld->addRigidBody(bt_body, desc.group_index, desc.group_mask);
	}
	bt_body->restart();
	return bt_body;
}

GsBtConstraint *GsBullet::addConstraint(const GsBullet::ConstraintDesc &desc)
{
	auto *bt_constraint = new GsBtConstraint(this, desc);
	m_btConstraints.push_back(bt_constraint);
	m_btWorld->addConstraint(bt_constraint);
	return bt_constraint;
}

void GsBullet::addPlane(const Vec3f &normal, float d, float restitution)
{
	auto *bt_shape = new btStaticPlaneShape(make_btvector3(normal), d);
	auto *bt_motion_state = new btDefaultMotionState(make_bttransform());
	auto rb_info = btRigidBody::btRigidBodyConstructionInfo(0.0, bt_motion_state, bt_shape);
	auto *bt_rigid_body = new btRigidBody(rb_info);

	bt_rigid_body->setRestitution(restitution);
	m_btWorld->addRigidBody(bt_rigid_body);
}

void GsBullet::addStaticMesh(
        int32_t triangle_count, int32_t *index_base, int32_t index_stride, int32_t vertex_count,
        float *vertex_base, int32_t vertex_stride)
{
	auto bt_mesh = new btTriangleIndexVertexArray(
	        triangle_count, index_base, index_stride, vertex_count, vertex_base, vertex_stride);
	auto bt_shape = new btBvhTriangleMeshShape(bt_mesh, true);
	auto bt_mot = new btDefaultMotionState();

	btRigidBody::btRigidBodyConstructionInfo bt_info(0, bt_mot, bt_shape);
	auto bt_body = new btRigidBody(bt_info);

	m_btWorld->addRigidBody(bt_body);
}

void GsBullet::removeNode(void *node)
{
	auto cp = begin(m_btConstraints);
	while (cp != end(m_btConstraints)) {
		if ((*cp)->node() == node) {
			m_btWorld->removeConstraint(*cp);
			delete *cp;
			cp = m_btConstraints.erase(cp);
		}
		else {
			++cp;
		}
	}

	auto bp = begin(m_btRigidBodies);
	while (bp != end(m_btRigidBodies)) {
		if ((*bp)->node() == node) {
			m_btWorld->removeRigidBody(*bp);
			delete *bp;
			bp = m_btRigidBodies.erase(bp);
		}
		else {
			++bp;
		}
	}
}

void GsBullet::reset() { m_doReset = true; }

void GsBullet::applyCentralImpulse(const void *bone, const Vec3f &dir)
{
	for (auto &bt_body: m_btRigidBodies) {
		if (bt_body->bone() == bone) {
			auto bt_dir = make_btvector3(dir);
			bt_body->applyCentralImpulse(bt_dir);
			bt_body->activate();
		}
	}
}

uint32_t GsBullet::getRigidBodyCount() const { return m_btRigidBodies.size(); }

uint32_t GsBullet::getConstraintCount() const { return m_btConstraints.size(); }

void GsBullet::update()
{
	m_seconds.update();

	// NEED PARAMETERIZE
	const auto c_fps = 120.0f;
	const auto c_max_sub_step_count = 120;

	if (m_doReset) {
		m_doReset = false;
		for (auto &bt_body: m_btRigidBodies) {
			bt_body->restart();
		}
	}
	m_btWorld->stepSimulation(m_seconds.delta(), c_max_sub_step_count, 1.0 / c_fps);
}

btRigidBody::btRigidBodyConstructionInfo GsBtRigidBody::createInfo(
        GsBullet *, const GsBullet::RigidBodyDesc &desc)
{
	btScalar bt_mass = 0;
	btCollisionShape *bt_shape = nullptr;

	// shape
	{
		switch (desc.shape_type) {
		case GsBullet::RigidBodyDesc::e_sphere: {
			bt_shape = new btSphereShape(desc.shape_size.x);
			break;
		}
		case GsBullet::RigidBodyDesc::e_box: {
			bt_shape = new btBoxShape(make_btvector3(desc.shape_size));
			break;
		}
		case GsBullet::RigidBodyDesc::e_capsule: {
			bt_shape = new btCapsuleShape(desc.shape_size.x, desc.shape_size.y - desc.shape_size.x);
			break;
		}
		default: aux_error(true, "unsupporte shape(%d)\n", desc.shape_type);
		}
	}

	// rigid body
	{
		auto bt_masslocal = make_bttransform(desc.transform);
		auto bt_motion_state = new GsBtMotionState(desc.bone, bt_masslocal, desc.mix_rate);

		bt_mass = desc.mix_rate > 0 ? desc.mass : 0.0;
		btRigidBody::btRigidBodyConstructionInfo bt_info(bt_mass, bt_motion_state, bt_shape);

		btVector3 bt_local_inertia(0, 0, 0);
		bt_shape->calculateLocalInertia(bt_mass, bt_local_inertia);
		bt_info.m_localInertia = bt_local_inertia;
		bt_info.m_linearDamping = desc.linear_dumping;
		bt_info.m_angularDamping = desc.angular_dumping;
		bt_info.m_restitution = desc.restitution;
		bt_info.m_friction = desc.friction;
		return bt_info;
	}
}

GsBtRigidBody::GsBtRigidBody(GsBullet *bt_world, const GsBullet::RigidBodyDesc &desc)
        : btRigidBody(createInfo(bt_world, desc)), m_node(desc.node), m_bone(desc.bone)
{
	if ((m_mix_rate = desc.mix_rate) == 0.0) {
		setCollisionFlags(getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		setActivationState(DISABLE_DEACTIVATION);
		setSleepingThresholds(0, 0);
	}
	else {
		// setSleepingThresholds(0.01, radians(0.1));  // need parameterize
	}
}

GsBtRigidBody::~GsBtRigidBody()
{
	delete getMotionState();
	delete getCollisionShape();
}

void GsBtRigidBody::restart()
{
	btVector3 bt_zero(0, 0, 0);
	btTransform bt_center_of_mass;

	getMotionState()->getWorldTransform(bt_center_of_mass);

	setLinearVelocity(bt_zero);
	setAngularVelocity(bt_zero);
	setInterpolationLinearVelocity(bt_zero);
	setInterpolationAngularVelocity(bt_zero);
	setCenterOfMassTransform(bt_center_of_mass);
	setInterpolationWorldTransform(bt_center_of_mass);

	clearForces();
	activate();
}

GsBullet::RigidBodyGeometry GsBtRigidBody::getRigidBodyGeometry() const
{
	btVector3 bt_min;
	btVector3 bt_max;
	btTransform bt_center_of_mass;

	getMotionState()->getWorldTransform(bt_center_of_mass);
	getCollisionShape()->getAabb(make_bttransform(), bt_min, bt_max);

	GsBullet::RigidBodyGeometry g;
	g.shape_type = getCollisionShape()->getShapeType();
	g.mix_rate = m_mix_rate;
	g.range.p0 = make_vec3f(bt_min);
	g.range.p1 = make_vec3f(bt_max);
	g.center_of_mass = make_transformf(bt_center_of_mass);

	return g;
}

btTransform getInvTransform(btRigidBody *bt_body, const Transformf &massworld)
{
	return bt_body->getWorldTransform().inverse() * make_bttransform(massworld);
}

GsBtConstraint::GsBtConstraint(GsBullet *bt_world, const GsBullet::ConstraintDesc &desc)
        : btGeneric6DofSpringConstraint(
                  *bt_world->m_btRigidBodies[desc.indexA], *bt_world->m_btRigidBodies[desc.indexB],
                  getInvTransform(bt_world->m_btRigidBodies[desc.indexA], desc.transform),
                  getInvTransform(bt_world->m_btRigidBodies[desc.indexB], desc.transform), true),
          m_node(desc.node), m_btBodyA(bt_world->m_btRigidBodies[desc.indexA]),
          m_btBodyB(bt_world->m_btRigidBodies[desc.indexB])
{
	auto set_stiffness = [&](int32_t id, float value) {
		if (value != 0.0f) {
			enableSpring(id, true);
			setStiffness(id, value);
		}
	};

	setLinearLowerLimit(make_btvector3(desc.linear_lower_limit));
	setLinearUpperLimit(make_btvector3(desc.linear_upper_limit));
	setAngularLowerLimit(make_btvector3(desc.angular_lower_limit));
	setAngularUpperLimit(make_btvector3(desc.angular_upper_limit));

	set_stiffness(0, desc.translate_stiffness.x);
	set_stiffness(1, desc.translate_stiffness.y);
	set_stiffness(2, desc.translate_stiffness.z);

	set_stiffness(3, desc.rotate_stiffness.x);
	set_stiffness(4, desc.rotate_stiffness.y);
	set_stiffness(5, desc.rotate_stiffness.z);
}

GsBullet::ConstraintGeometry GsBtConstraint::getConstraintGeometry() const
{
	GsBullet::ConstraintGeometry g;
	g.geometryA = m_btBodyA->getRigidBodyGeometry();
	g.geometryB = m_btBodyB->getRigidBodyGeometry();
	return g;
}

void GsBullet::newWorld(float world_size, float gravity)
{
	const btVector3 bt_world_min(-world_size, -world_size, -world_size);
	const btVector3 bt_world_max(+world_size, +world_size, +world_size);

	initBullet(bt_world_min, bt_world_max, gravity);
	ms_bullets().insert(this);
}

void GsBullet::initBullet(const btVector3 &bt_world_min, const btVector3 &bt_world_max, float gravity)
{
	m_btCollisionConfig = new btDefaultCollisionConfiguration();
	m_btCollisionDispatcher = new btCollisionDispatcher(m_btCollisionConfig);
	m_btSolver = new btSequentialImpulseConstraintSolver();
	m_btAxisSweep3 = new btAxisSweep3(bt_world_min, bt_world_max, c_max_proxies);
	m_btWorld = new btDiscreteDynamicsWorld(
	        m_btCollisionDispatcher, m_btAxisSweep3, m_btSolver, m_btCollisionConfig);
	m_btWorld->setGravity(btVector3(0.0, -gravity, 0.0));
}

void GsBullet::deleteWorld()
{
	disposeBullet();
	ms_bullets().erase(this);
}

void GsBullet::disposeBullet()
{
	for (auto i = m_btWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
		auto *collision_object = m_btWorld->getCollisionObjectArray()[i];
		m_btWorld->removeCollisionObject(collision_object);
		delete collision_object;
		collision_object = nullptr;
	}
	delete m_btWorld;
	delete m_btSolver;
	delete m_btAxisSweep3;
	delete m_btCollisionDispatcher;
	delete m_btCollisionConfig;

	m_btWorld = nullptr;
	m_btSolver = nullptr;
	m_btAxisSweep3 = nullptr;
	m_btCollisionDispatcher = nullptr;
	m_btCollisionConfig = nullptr;
}

void GsBullet::resetAll()
{
	for (auto *gs_world: ms_bullets()) {
		gs_world->reset();
	}
}

void GsBullet::updateAll()
{
	for (auto *bullet: ms_bullets()) {
		bullet->update();
	}
}

std::vector<GsBullet::RigidBodyGeometry> GsBullet::getRigidBodyGeometries() const
{
	std::vector<GsBullet::RigidBodyGeometry> geometries;
	for (const auto &bt_body: m_btRigidBodies) {
		geometries.push_back(bt_body->getRigidBodyGeometry());
	}
	return geometries;
}

std::vector<GsBullet::ConstraintGeometry> GsBullet::getConstraintGeometries() const
{
	std::vector<GsBullet::ConstraintGeometry> geometries;
	for (const auto &bt_constraint: m_btConstraints) {
		geometries.push_back(bt_constraint->getConstraintGeometry());
	}
	return geometries;
}

void GsBullet::view(const std::vector<Mat4f> &nodeworlds) const
{
	auto *painter = gs_painter::Stdout::get();

	painter->begin();
	painter->setFillAlpha(0.05);

	for (auto &g: getRigidBodyGeometries()) {
		const char *color = g.mix_rate == 1.0 ? "fuchsia" : g.mix_rate == 0.0 ? "aqua" : "yellow";
		painter->setColor(color);
		auto resize_matrix = Mat4f().scale(g.range.span() * 0.5).trans(g.range.center());
		painter->addPrim((Mat4f(g.center_of_mass) * resize_matrix).inverse());
	}
	painter->end();

	// painter->renderMode(GL_TRIANGLES, nodeworlds);
	// painter->renderMode(GL_LINES, nodeworlds);
	painter->draw(GL_TRIANGLES, nodeworlds);
	painter->draw(GL_LINES, nodeworlds);

	painter->begin();
	for (auto &g: getConstraintGeometries()) {
		painter->setColor("white");
		Segment3f segment = {
		        g.geometryA.center_of_mass.t,
		        g.geometryB.center_of_mass.t,
		};
		painter->addPrim(segment);
	}
	painter->end();
	// painter->renderMode(GL_LINES, nodeworlds);
	painter->draw(GL_LINES, nodeworlds);
}
}  // namespace spu
