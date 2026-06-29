//
// MMDActor :
//
#include "internal.h"
#include <smath/color_chart.h>
#include <gsys/painter/stdout.h>

namespace spu::gs_node {

void MMDBone::setLimitAngle(bool is_limit, const Vec3f &lower_limit, const Vec3f &upper_limit)
{
	m_isLimitAngle = is_limit;
	m_lowerLimit = lower_limit;
	m_upperLimit = upper_limit;
}

Transformf MMDBone::animate(const Transformf &local)
{
	m_isAnimated = true;
	setBoneparent(orgBoneparent() * local);
	return boneparent();
}

void MMDBone::link()
{
	if (parent()) {
		if (!vector_is_find(parent()->getChildren(), this)) {
			parent()->getChildren().push_back(this);
		}
		setOrgBoneparent(parent()->orgBoneworld().inverse() * orgBoneworld());
	}
	reset();
}

void MMDBone::limit()
{
	if (m_isLimitAngle) {
		// auto t0 = boneparent().t;
		auto q0 = boneparent().q;
		auto euler0 = q0.to_euler();
		auto euler1 = clamp(euler0, m_lowerLimit, m_upperLimit);
		auto q1 = Quatf::from_eulerZYX(euler1);

		if (q0 != q1) {
			rotate(q0.inverse() * q1);
			setBoneparent(boneparent());
		}
	}
}

void MMDBone::updateIK()
{
	if (m_effectorBone == nullptr) {
		return;
	}
	assert(m_effectorBone);
	for (auto n = 0; n < m_iterationCount; n++) {
		for (auto &ik_bone: m_ikBones) {  // assumes ik_link is sorted
			auto worldbone = ik_bone->boneworld().inverse();
			auto target_local = normalize((worldbone * boneworld()).t);
			auto effect_local = normalize((worldbone * m_effectorBone->boneworld()).t);

			auto rotation = Quatf::from_target(target_local, effect_local, m_rotateLimit);

			ik_bone->rotate(rotation);
			ik_bone->limit();
			ik_bone->sync();

			const float min_sin = sinf(radians(0.01f));
			if (length(cross(target_local, effect_local)) < min_sin) {
				return;
			}
		}
	}
}

void MMDBone::updateGrant()
{
	if (m_grantorBone == nullptr) {
		return;
	}

	auto *grantor_bone = m_grantorBone;
	while (grantor_bone && !grantor_bone->isAnimated() && grantor_bone->m_grantorBone) {
		grantor_bone = grantor_bone->m_grantorBone;
	}

	if (grantor_bone && grantor_bone->isAnimated()) {
		if (m_grantRotate) {
			lerp(grantor_bone->boneparent().q, m_grantWeight);
		}
		if (m_grantTranslate) {
			lerp(grantor_bone->boneparent().t, m_grantWeight);
		}
		MMDBone::sync();
	}
}

void MMDBone::reset()
{
	m_isAnimated = false;
	Bone3f::reset();
}

void MMDBone::addToDebugPainter(GsNode *actor) const
{
	auto *painter = gs_painter::Stdout::get();

	auto add_bone = [&](const MMDBone *bone, const char *color, float alpha = 1.0) {
		if (parent()) {
			painter->setColor(color, alpha);
			auto p0 = bone->boneworld().t;
			auto p1 = bone->parent()->boneworld().t;
			painter->addJoint(p0, p1);
		}
	};
	if (m_effectorBone && actor->getProperty("debug_render_ik")) {
		add_bone(this, "magenta");
	}
	else if (actor->getProperty("debug_render_bone")) {
		if (isAnimated()) {
			add_bone(this, "gray");
		}
		else {
			add_bone(this, "dimgray", 0.1);
		}
	}
	if (m_grantorBone && m_grantorBone->isAnimated() && actor->getProperty("debug_render_grant")) {
		add_bone(m_grantorBone, "yellow");
	}
}

void MMDMorph::update(MMDActor *actor)
{
	float rate = m_value;
	if (rate == 0.0) {
		return;
	}
	auto &spuMorphs = actor->getSpuMorphs();
	for (auto &factor: m_spuFactors) {
		spuMorphs[factor.index] = {
		        factor.type,
		        factor.spu_index,
		        rate,
		};
	}
}

MMDActor::MMDActor(const Attrs &attrs) : MMDActor()
{
	setProperty(e_grant, 1);
	setProperty(e_ik, 1);
	setProperty(e_bone, 1);
	setProperty(e_morph, 1);
	setProperty(e_debug_render_bone, 0);
	setProperty(e_debug_render_grant, 0);
	setProperty(e_debug_render_ik, 0);
	setProperty(e_debug_render_wireframe, 1);

	m_masterFrame = attrs.get<float *>("master_frame", nullptr);
	aux_error(m_masterFrame == nullptr, "no master frame\n");

	m_bullet = attrs.get<GsBullet *>("bullet", nullptr);
	aux_error(m_bullet == nullptr, "no bullet\n");

	m_path = attrs.get("path", "");
	aux_error(m_path.empty(), "no path\n");
	attrs.peek("painter", "use 'replacePainter()' instead");
}

MMDActor::~MMDActor()
{
	delete m_baseBone;

	for (auto &bone: m_bones) {
		delete bone;
	}
	for (auto &morph: m_morphs) {
		delete morph;
	}
	for (auto &animation: m_animations) {
		delete animation;
	}
	m_bullet->removeNode(this);
}

void MMDActor::replacePainter(GsPainter *painter)
{
	GsNode::replacePainter(painter);
	m_bullet->reset();
}

void MMDActor::addAnimation(MMDAnimation *animation)
{
	animation->bind(m_bones, m_morphs);
	m_animations.push_back(animation);
}

std::vector<std::vector<Transformf>> MMDActor::rehearsal(
        const std::vector<std::string> &bone_names, const Range1f &master_range)
{
	std::vector<std::vector<Transformf>> transforms_list;
	std::vector<const MMDBone *> bones;

	bones.reserve(bone_names.size());

	for (auto *bone: getBones()) {
		for (const auto &bone_name: bone_names) {
			if (bone_name == bone->name()) {
				bones.push_back(bone);
				break;
			}
		}
	}
	transforms_list.resize(bones.size());

	auto frame_range = frameRange();
	for (float frame = master_range.p0; frame < master_range.p1; frame++) {
		auto sticky_frame = std::clamp(frame, frame_range.p0, frame_range.p1);
		animate(sticky_frame);
		for (auto i = 0u; i < bones.size(); i++) {
			transforms_list[i].push_back(bones[i]->boneworld());
		}
	}

	return transforms_list;
}

void MMDActor::update()
{
	animate(*m_masterFrame);
	GsNode::update();
}

void MMDActor::animate(float master_frame)
{
	// const auto &props = properties();
	auto is_bone = getProperty(e_bone);
	auto is_morph = getProperty(e_morph);
	for (auto &animation: m_animations) {
		animation->enable(is_bone, is_morph);
	}

	for (auto &bone: getBones()) {
		bone->reset();
	}

	for (auto &spu_morph: m_spuMorphs) {
		spu_morph = {0, 0, 0.0};
	}

	MMDBlendMotion<Transformf> bone_blend(getBones());
	MMDBlendMotion<float> morph_blend(getMorphs());
	MMDBlend<Transformf> center_blend;

	for (auto &clip: clips()) {
		auto weight = clip.getWeight(master_frame);
		if (weight > 0 && clip.animation_id < int32_t(m_animations.size())) {
			auto *animation = m_animations.at(clip.animation_id);
			auto frame = master_frame + clip.frame_offset;
			auto clip_transform = clip.getTransform(master_frame);
			auto animation_transform = animation->rehearsal(frame, "センター");
			center_blend.add(weight, clip_transform * animation_transform);
			animation->animate(frame);

			bone_blend.add(weight, animation->boneMotions());
			morph_blend.add(weight, animation->morphMotions());
		}
	}

	bone_blend.animate(getBones());
	morph_blend.animate(getMorphs());

	if (center_blend.weight() > 0) {
		center_blend.blend();
		m_baseBone->setBoneworld(center_blend.value() * m_centerBone->boneparent().inverse());
		m_baseBone->sync();

		for (auto &morph: m_morphs) {
			morph->update(this);
		}

		if (getProperty(e_ik)) {
			for (auto &ik_bone: m_ikBones) {
				ik_bone->updateIK();
			}
		}

		if (getProperty(e_grant)) {
			for (auto &ik_bone: m_ikBones) {
				ik_bone->updateGrant();
			}
		}
		// return true;
		return;
	}
	// return false;
}

SpuTexture MMDActor::loadTexture(const std::filesystem::path &path, const Attrs &texture_attrs)
{
	SpuTexture texture;
	if (!path.empty()) {
		auto full_path = File::searchPath(path, false);
		if (std::filesystem::exists(full_path)) {
			Attrs attrs = {
			        {"iformat", GL_SRGB8_ALPHA8},
			        {"flip",    "y"            },
			};
			attrs.append(texture_attrs);
			texture.init(full_path.string().c_str(), attrs);
			return texture;
		}
		aux_message(0, "'%s': not found\n", path.string().c_str());
	}
	return GsObject::defaultWhiteTexture();
}

void MMDActor::setCommon(GsPainter *painter, const mmd::ActorFile &desc, bool is_relative)
{
	// bone
	{
		m_baseBone = desc.base_bone;
		m_centerBone = desc.center_bone;
		aux_error(m_centerBone == nullptr, "center bone not found\n");
		m_bones = desc.bones;
		for (auto &bone: m_bones) {
			bone->link();
		}
		m_ikBones = desc.ik_bones;
	}

	// morph
	{
		m_morphs = desc.morphs;
		m_morphTargets = desc.morph_targets;
		m_spuMorphs.resize(desc.vertices.size(), {0, 0, 0.0});
	}

	// rigidBody
	{
		auto rigid_body_base = m_bullet->getRigidBodyCount();  // must be here
		for (auto desc: desc.rigid_body_descs) {               // copy
			desc.node = this;
			desc.bone = desc.index >= 0 ? m_bones[desc.index] : m_centerBone;
			if (is_relative) {
				desc.transform.t -= desc.bone->orgBoneworld().t;
			}
			m_bullet->addRigidBody(desc);
		}
		// m_hasPhysics = !desc.rigid_body_descs.empty();

		for (auto desc: desc.constraint_descs) {  // copy
			desc.node = this;
			desc.indexA += rigid_body_base;
			desc.indexB += rigid_body_base;
			m_bullet->addConstraint(desc);  // register inside
		}
	}
	// vertices, indices and decorator
	{
		auto decorators = select_objects<gs_decorator::Instance *>(painter->getDecorators());
		assert(decorators.size() == 1);

		std::vector<Mesh::Vertex> mesh_vertices;
		std::vector<int32_t> indices;
		for (auto &vertex: desc.vertices) {
			Mesh::Vertex v;
			v.p = vertex.position;
			v.n = vertex.normal;
			v.t = vertex.texcoord;
			mesh_vertices.push_back(v);
		}
		for (auto &face: desc.faces) {
			indices.push_back(face.vertices[0]);
			indices.push_back(face.vertices[1]);
			indices.push_back(face.vertices[2]);
		}
		painter->send(mesh_vertices, indices);

		std::vector<gs_decorator::Instance::Bone> spuBones;
		for (auto &vertex: desc.vertices) {
			gs_decorator::Instance::Bone spuBone = {
			        vertex.bone,
			        vertex.weight,
			};
			spuBones.push_back(spuBone);
		}

		decorators.at(0)->sendBones(spuBones);
		decorators.at(0)->sendMorphs(m_spuMorphs);
		decorators.at(0)->sendMorphTargets(m_morphTargets);
	}
}

void MMDActor::setDecorator(const std::vector<MMDBone *> &bones)
{
	// auto *decorator = getPainter()->selectDecorator<gs_decorator::Instance *>();
	auto decorators = select_objects<gs_decorator::Instance *>(getPainter()->getDecorators());
	std::vector<Mat4f> bone_matrices;
	for (auto &bone: bones) {
		auto bone_matrix = bone->boneworld() * bone->orgBoneworld().inverse();
		bone_matrices.push_back(bone_matrix);
	}
	decorators.at(0)->sendBoneMatrices(bone_matrices);
	decorators.at(0)->sendMorphs(m_spuMorphs);
}

void MMDActor::doRender()
{
	setDecorator(m_bones);
	GsNode::doRender();
}

void MMDActor::doDebugRender()
{
	auto is_debug_render = getProperty(e_debug_render_ik) | getProperty(e_debug_render_grant)
	                     | getProperty(e_debug_render_bone);

	if (is_debug_render) {
		auto *painter = gs_painter::Stdout::get();
		painter->begin();
		for (auto &bone: m_bones) {
			bone->addToDebugPainter(this);
		}
		painter->end();
		auto nodeworlds = instancedNodeworlds().at(0);  // slot #0 only
		painter->draw(GL_TRIANGLES, nodeworlds);
		painter->draw(GL_LINES, nodeworlds);
	}
	setDecorator(m_bones);
	if (getProperty(e_debug_render_wireframe)) {
		GsNode::doDebugRender();
	}
}

}  // namespace spu::gs_node
