//
// Bone3f :
//
#pragma once
#include "quatf.h"

namespace spu {

class Bone3f {
public:
	Bone3f() = default;
	virtual ~Bone3f() = default;

	Bone3f(Bone3f *parent, const Transformf &transform, bool is_local = true)
	{
		init(parent, transform, is_local);
	}

	virtual void init(Bone3f *parent, const Transformf &transform, bool is_local = true)
	{
		if (parent) {
			m_boneparent = is_local ? transform : parent->m_boneworld.inverse() * transform;
			parent->m_children.push_back(this);
		}
		else {
			m_boneparent = transform;
		}
		m_boneworld = parent ? parent->boneworld() * m_boneparent : m_boneparent;
		m_boneinstance = parent ? parent->boneinstance() * m_boneparent : m_boneparent;
		m_orgBoneparent = m_boneparent;
		m_orgBoneworld = m_boneworld;
		m_parent = parent;
	}

	virtual void reset()
	{
		m_boneparent = m_orgBoneparent;
		m_boneworld = m_orgBoneworld;
	}

	virtual void sync()
	{
		m_boneworld = m_parent ? m_parent->boneworld() * m_boneparent : m_boneparent;
		for (auto &bone: getChildren()) {
			bone->sync();
		}
	}

	void setParent(Bone3f *parent) { m_parent = parent; }
	void setOrgBoneparent(const Transformf &org_boneparent) { m_orgBoneparent = org_boneparent; }
	void setOrgBoneworld(const Transformf &org_boneworld) { m_orgBoneworld = org_boneworld; }

	void setBoneparent(const Transformf &boneparent)
	{
		m_boneparent = boneparent;
		m_boneworld = m_parent ? m_parent->m_boneworld * m_boneparent : m_boneparent;
	}

	void setBoneworld(const Transformf &boneworld)
	{
		m_boneworld = boneworld;
		m_boneparent = m_parent ? m_parent->m_boneworld.inverse() * m_boneworld : m_boneworld;
	}

	void setBoneInstance(const Transformf &boneinstance)
	{
		assert(m_children.empty());
		m_boneinstance = boneinstance;
	}

	void translate(const Vec3f &translation) { m_boneparent.t += translation; }
	void rotate(const Quatf &rotation) { m_boneparent.q *= rotation; }

	void lerp(const Vec3f &translation, float rate)
	{
		m_boneparent.t = spu::lerp(m_boneparent.t, translation, rate);
	}

	void lerp(const Quatf &rotation, float rate)
	{
		m_boneparent.q = spu::lerp(m_boneparent.q, rotation, rate);
	}

	const Bone3f *parent() const { return m_parent; }
	Bone3f *parent() { return m_parent; }

	const std::vector<Bone3f *> &getChildren() const { return m_children; }
	std::vector<Bone3f *> &getChildren() { return m_children; }

	Transformf boneparent() const { return m_boneparent; }
	Transformf boneworld() const { return m_boneworld; }
	Transformf boneinstance() const { return m_boneinstance; }
	Transformf nodeinstance() const { return boneworld() * m_boneinstance.inverse(); }
	Transformf orgBoneparent() const { return m_orgBoneparent; }
	Transformf orgBoneworld() const { return m_orgBoneworld; }

	// protected:
private:
	std::vector<Bone3f *> m_children;
	Bone3f *m_parent = nullptr;
	Transformf m_boneparent;
	Transformf m_boneworld;
	Transformf m_boneinstance;
	Transformf m_orgBoneparent;
	Transformf m_orgBoneworld;
};
}  // namespace spu
