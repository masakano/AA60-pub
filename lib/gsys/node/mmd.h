//
// MMDFrame :
//
#pragma once

#include <smath/mmd.h>
#include <smath/bone3f.h>
#include <gsys/node.h>
#include <gsys/util/bullet.h>
#include <gsys/decorator/instance.h>

namespace spu::gs_node {

namespace mmd {
struct PMDFile;
struct PMXFile;
struct ActorFile;
extern MMDAnimation *createFromVMDFile(const std::filesystem::path &path);  // need fix
}  // namespace mmd

struct MMDFrame {
	Range1f range = {0, 9999};
	float current = 0;
	float frame_per_sec = 24;
	int32_t advance = huge<int32_t>();

	bool update(float delta);
};

class MMDBone : public MMDObject<Transformf>, public Bone3f {
public:
	using value_t = Transformf;

	Transformf animate(const Transformf &local) override;

	void link();
	void limit();
	void updateIK();
	void updateGrant();
	void setLimitAngle(
	        bool is_limit, const Vec3f &lower_limit = Vec3f(0), const Vec3f &upper_limit = Vec3f(0));
	void reset() override;
	bool isAnimated() const { return m_isAnimated; }
	void addToDebugPainter(GsNode *actor) const;

protected:
	friend class mmd::PMDFile;
	friend class mmd::PMXFile;

	std::vector<MMDBone *> m_ikBones;
	MMDBone *m_effectorBone = nullptr;
	MMDBone *m_grantorBone = nullptr;
	bool m_grantRotate;
	bool m_grantTranslate;
	float m_grantWeight;
	int32_t m_iterationCount;
	float m_rotateLimit;

	bool m_isLimitAngle = false;
	bool m_isAnimated = false;
	Vec3f m_lowerLimit = Vec3f(0);
	Vec3f m_upperLimit = Vec3f(0);
};

class MMDActor;
class MMDMorph : public MMDObject<float> {
public:
	struct SpuFactor {
		int32_t index;
		int32_t type;
		int32_t spu_index;
	};

	using value_t = float;
	float animate(const float &rate) override { return m_value = rate; }

	const value_t &get() const { return m_value; }
	virtual void update(MMDActor *actor);
	std::vector<SpuFactor> &getSpuFactors() { return m_spuFactors; }

protected:
	float m_value;
	std::vector<SpuFactor> m_spuFactors;
};

// KEEP THIS INHERIT ORDER
class MMDActor : public GsNode, public MMDClips {
public:
	static constexpr hash32_t e_grant = "grant";
	static constexpr hash32_t e_ik = "ik";
	static constexpr hash32_t e_bone = "bone";
	static constexpr hash32_t e_morph = "morph";
	static constexpr hash32_t e_debug_render_bone = "debug_render_bone";
	static constexpr hash32_t e_debug_render_grant = "debug_render_grant";
	static constexpr hash32_t e_debug_render_ik = "debug_render_ik";
	static constexpr hash32_t e_debug_render_wireframe = "debug_render_wireframe";

	void replacePainter(GsPainter *painter) override;
	void addAnimation(MMDAnimation *animation);
	void update() override;

	std::vector<std::vector<Transformf>> rehearsal(
	        const std::vector<std::string> &bone_names, const Range1f &master_range) override;

	const MMDBone *baseBone() const { return m_baseBone; }
	const MMDBone *centerBone() const { return m_centerBone; }

	const std::vector<MMDAnimation *> &animations() const { return m_animations; }
	std::vector<MMDBone *> &getBones() { return m_bones; }
	std::vector<MMDMorph *> &getMorphs() { return m_morphs; }
	std::vector<gs_decorator::Instance::Morph> &getSpuMorphs() { return m_spuMorphs; }

protected:
	friend class MMDMaster;

	explicit MMDActor(const char *name = nullptr) : GsNode(name), MMDClips(ezero()) {}
	explicit MMDActor(const Attrs &attrs);
	~MMDActor();

	std::filesystem::path m_path;
	std::vector<MMDBone *> m_bones;
	std::vector<MMDBone *> m_ikBones;
	std::vector<MMDMorph *> m_morphs;

	std::vector<gs_decorator::Instance::Morph> m_spuMorphs;
	std::vector<Vec4f> m_morphTargets;
	MMDBone *m_baseBone = nullptr;
	MMDBone *m_centerBone = nullptr;

	void setDecorator(const std::vector<MMDBone *> &bones);
	void setCommon(GsPainter *painter, const mmd::ActorFile &desc, bool is_relative);
	void doRender() override;
	void doDebugRender() override;
	static SpuTexture loadTexture(const std::filesystem::path &path, const Attrs &texture_attrs = Attrs());

private:
	std::vector<MMDAnimation *> m_animations;
	GsBullet *m_bullet = nullptr;
	float *m_masterFrame = nullptr;
	void animate(float master_frame);
};

class MMDMasterInspector;
class MMDMaster : public GsNode {
public:
	static constexpr hash32_t e_debug_render_body = "debug_render_body";
	static constexpr hash32_t e_physics = "physics";
	static constexpr int32_t e_auto_frame_advance = huge<int32_t>();

	explicit MMDMaster(const char *name = nullptr) : GsNode(name)
	{
		setProperty(e_physics, 1);
		setProperty(e_debug_render_body, 0);
	}

	explicit MMDMaster(const Attrs &attrs) : MMDMaster() { init(attrs); }

	~MMDMaster();

	MMDActor *addActor(
	        int32_t index, const std::string &path, GsPainter *painter, const Attrs &node_attrs = Attrs());

	void fitRange()
	{
		auto &frame_range = m_frame.range;
		frame_range.invalidate();
		for (auto &node: getChildren()) {
			auto *actor = dynamic_cast<MMDActor *>(node);
			if (actor) {
				frame_range.expand(actor->fitRange(actor->animations()));
			}
		}
	}
	MMDFrame &getFrame() { return m_frame; }
	const MMDFrame &getFrame() const { return m_frame; }

	GsBullet &getBullet() { return m_bullet; }
	const GsBullet &getBullet() const { return m_bullet; }

	void init(const Attrs &) override;
	void set(const Attrs &attrs) override;
	void update() override;
	void startInspector() override;

protected:
	void doDebugRender() override;

private:
	friend MMDMasterInspector;
	GsBullet m_bullet;
	MMDFrame m_frame;
};
}  // namespace spu::gs_node
