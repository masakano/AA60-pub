//
// MMDObject :
//
#pragma once

#include "curve.h"
#include "quatf.h"
#include "range.h"

namespace spu {

template<class value_t> class MMDObject {
public:
	virtual ~MMDObject() = default;

	MMDObject() = default;
	MMDObject(const std::string &name) : m_jName(name) {}

	virtual value_t animate([[maybe_unused]] const value_t &value) { return value_t(0); }

	const std::string &name() const { return !m_jName.empty() ? m_jName : m_eName; }
	void setName(const std::string &name) { m_jName = name; }
	void setEnglishName(const std::string &name) { m_eName = name; }

private:
	std::string m_jName;
	std::string m_eName;
};

template<class value_t> class MMDMotion {
public:
	using key_value_t = std::pair<float, value_t>;

	MMDMotion() = default;

	MMDMotion(const std::string &name, std::vector<key_value_t> &key_values)
	{
		m_name = name;
		m_curve.init(key_values);
	}

	value_t animate(float frame)
	{
		m_value = m_curve(frame);
		return m_target->animate(m_value);
	}

	void setTarget(MMDObject<value_t> *target) { m_target = target; }
	const MMDObject<value_t> *target() const { return m_target; }

	const value_t &value() const { return m_value; }
	const std::string &name() const { return m_name; }

	Range1f range() const { return m_curve.range(); }

private:
	std::string m_name;
	value_t m_value = value_t(0);
	MMDObject<value_t> *m_target = nullptr;
	LinearCurve<float, value_t> m_curve;  // linear (1st order)
};

template<class value_t> class MMDBlend {
public:
	void clear()
	{
		m_pairs.clear();
		m_result = {0.0f, value_t(0)};
	}
	void add(float weight, const value_t &value)
	{
		m_pairs.emplace_back(weight, value);
		m_result.first += weight;
	}
	void blend()
	{
		m_result = {0.0f, value_t(0)};
		for (auto &pair: m_pairs) {
			auto &weight = pair.first;
			auto &value = pair.second;
			auto rate = weight == 0 ? 0 : weight / (m_result.first += weight);
			m_result.second = lerp(m_result.second, value, rate);
		}
	}
	float weight() const { return m_result.first; }
	const value_t &value() const { return m_result.second; }

private:
	std::vector<std::pair<float, value_t>> m_pairs;
	std::pair<float, value_t> m_result;
};

template<class value_t> class MMDBlendMotion {
public:
	template<class motion_t> MMDBlendMotion(const std::vector<motion_t *> &targets)
	{
		for (auto target: targets) {
			m_blends[target] = MMDBlend<value_t>();
		}
	}

	void clear()
	{
		for (auto &pair: m_blends) {
			pair.second.clear();
		}
	}

	template<class motion_t> void add(float weight, const std::vector<motion_t> &motions)
	{
		for (auto &motion: motions) {
			m_blends.at(motion.target()).add(weight, motion.value());
		}
	}

	template<class motion_t> void animate(const std::vector<motion_t *> &targets)
	{
		for (auto &target: targets) {
			auto &blend = m_blends.at(target);
			if (blend.weight() > 0) {
				blend.blend();
				target->animate(blend.value());
			}
		}
	}

private:
	std::map<const MMDObject<value_t> *, MMDBlend<value_t>> m_blends;
};

class MMDAnimation {
public:
	MMDAnimation() = default;

	void init(
	        const std::string &name, const std::vector<MMDMotion<Transformf>> bone_motion_pool,
	        const std::vector<MMDMotion<float>> morph_motion_pool);

	virtual ~MMDAnimation() = default;
	Transformf rehearsal(float frame, const std::string &name) const;

	void animate(float frame);
	void enable(bool is_bone, bool is_morph)
	{
		m_isBone = is_bone;
		m_isMorph = is_morph;
	}

	template<class bone_t, class morph_t>
	void bind(std::vector<bone_t *> &bones, std::vector<morph_t *> &morphs)
	{
		bindEach(bones, m_boneMotions, m_boneMotionPool);
		bindEach(morphs, m_morphMotions, m_morphMotionPool);
	}
	const Range1f &frameRange() const { return m_frameRange; }
	const std::vector<MMDMotion<Transformf>> &boneMotions() const { return m_boneMotions; }
	const std::vector<MMDMotion<float>> &morphMotions() const { return m_morphMotions; }

protected:
	std::string m_name;

	std::vector<MMDMotion<Transformf>> m_boneMotionPool;
	std::vector<MMDMotion<Transformf>> m_boneMotions;
	std::vector<MMDMotion<float>> m_morphMotionPool;
	std::vector<MMDMotion<float>> m_morphMotions;

	Range1f m_frameRange;
	bool m_isBone = true;
	bool m_isMorph = true;

	mutable MMDMotion<Transformf> m_cachedBoneMotion;

	template<class target_t, class motion_t>
	void bindEach(
	        std::vector<target_t *> &targets, std::vector<motion_t> &motions,
	        const std::vector<motion_t> &motion_pool) const
	{
		motions.clear();
		for (auto motion: motion_pool) {  // copy
			for (auto &target: targets) {
				auto &name = target->name();
				auto short_name = name.substr(0, name.find("（"));

				if (short_name == motion.name()) {
					motion.setTarget(target);
					motions.push_back(motion);
				}
			}
		}
	}
};

class MMDClips;

struct MMDClip {
	int32_t animation_id = 0;
	float frame_offset = 0;

	Range1f frame_range = {0, 9999};
	Range1f weight_range = {1, 1};

	std::array<Transformf, 2> transforms;

	MMDClips split(float master_frame) const;
	float getWeight(float master_frame) const;
	Transformf getTransform(float master_frame) const;
};

class MMDClips {
public:
	virtual ~MMDClips() = default;
	MMDClips() = default;
	MMDClips(std::initializer_list<MMDClip> list);

	explicit MMDClips(const std::vector<MMDClip> &list);
	explicit MMDClips(const Vec3f &position)
	{
		MMDClip clip;
		clip.transforms[0].t = clip.transforms[1].t = position;
		m_clips.emplace_back(clip);
	}

	MMDClips pack() const;
	MMDClips split(float master_frame, Transformf *split_trans = nullptr) const;
	MMDClips mix(const MMDClips &clips, float master_frame, float window) const;
	MMDClips trim(const Range1f &master_range) const;
	MMDClips adjust(const MMDClips &clips, float master_frame, MMDClips *actor, const std::string &name);
	MMDClips correlate(
	        const MMDClips &clips, float master_frame, float cross_window, MMDClips *actor,
	        const std::vector<std::string> &names, float search_window);
	void partition(Range1f master_range, MMDClips &inside, MMDClips &outside) const;
	void report(const char *s = nullptr) const;

	Range1f fitRange(const std::vector<MMDAnimation *> &animations) const
	{
		Range1f frame_range;
		frame_range.invalidate();
		for (const auto &clip: m_clips) {
			auto *animation = animations.at(clip.animation_id);
			frame_range.expand(animation->frameRange() - clip.frame_offset);
		}
		return frame_range;
	}

	Range1f frameRange() const
	{
		Range1f frame_range;
		frame_range.invalidate();
		for (const auto &clip: m_clips) {
			frame_range.expand(clip.frame_range - clip.frame_offset);
		}
		return frame_range;
	}

	friend MMDClips operator*(const Transformf &transform, const MMDClips &clips);
	friend MMDClips operator+(const MMDClips &clips0, const MMDClips &clips1);

	const std::vector<MMDClip> &clips() const { return m_clips; }
	MMDClip at(int32_t index) { return m_clips.at(index); }

	void push_back(const MMDClip &clip) { m_clips.push_back(clip); }
	void copy(const std::vector<MMDClip> &clips) { m_clips = clips; }
	void cat(const std::vector<MMDClip> &clips) { vector_cat(m_clips, clips); }

	virtual std::vector<std::vector<Transformf>> rehearsal(
	        [[maybe_unused]] const std::vector<std::string> &bone_names,
	        [[maybe_unused]] const Range1f &master_range)
	{
		assert(0);
		return {};
	}

private:
	std::vector<MMDClip> m_clips;
};
}  // namespace spu
