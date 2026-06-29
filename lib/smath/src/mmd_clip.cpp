//
// MMDClips :
//
#include <smath/mmd.h>

namespace spu {

MMDClips MMDClip::split(float master_frame) const
{
	auto split_frame = master_frame + frame_offset;
	auto split_weight = getWeight(master_frame);
	auto split_transform = getTransform(master_frame);

	if (frame_range.p0 < split_frame && split_frame < frame_range.p1) {
		std::vector<MMDClip> clips = {*this, *this};
		clips[0].frame_range.p1 = clips[1].frame_range.p0 = split_frame;
		clips[0].weight_range.p1 = clips[1].weight_range.p0 = split_weight;
		clips[0].transforms[1] = clips[1].transforms[0] = split_transform;
		return MMDClips(clips);
	}

	return MMDClips({*this});
}

float MMDClip::getWeight(float master_frame) const
{
	auto frame = master_frame + frame_offset;
	if (frame_range.p0 <= frame && frame <= frame_range.p1) {
		auto rate = (frame - frame_range.p0) / (frame_range.p1 - frame_range.p0);
		return lerp(weight_range.p0, weight_range.p1, rate);
	}
	return 0.0;
}

Transformf MMDClip::getTransform(float master_frame) const
{
	auto frame = master_frame + frame_offset;
	if (frame_range.p0 <= frame && frame <= frame_range.p1) {
		float rate = (frame - frame_range.p0) / (frame_range.p1 - frame_range.p0);
		auto transform = lerp(transforms[0], transforms[1], rate);
		{
			if (std::isnan(transform.t.x) || transform.t.x < -1000.0f || transform.t.x > 1000.0f) {
				transform.report("clip.getTransform");
				assert(0);
			}
		}
		return transform;
	}
	return Transformf();
}

MMDClips::MMDClips(const std::vector<MMDClip> &list) { copy(list); }

MMDClips::MMDClips(std::initializer_list<MMDClip> list) { copy(list); }

MMDClips operator*(const Transformf &transform, const MMDClips &src_clips)
{
	auto clips = src_clips.clips();
	for (auto &clip: clips) {
		clip.transforms[0] = transform * clip.transforms[0];
		clip.transforms[1] = transform * clip.transforms[1];
	}
	return MMDClips(clips);
}

MMDClips operator+(const MMDClips &clips0, const MMDClips &clips1)
{
	auto result = clips0;
	result.cat(clips1.clips());
	return result;
}

MMDClips MMDClips::split(float master_frame, Transformf *split_trans) const
{
	MMDClips result;
	MMDBlend<Transformf> blend;
	for (auto &clip: clips()) {
		auto split_clips = clip.split(master_frame);
		result.cat(split_clips.clips());
		blend.add(split_clips.at(0).weight_range.p1, split_clips.at(0).transforms[1]);
	}
	if (split_trans) {
		blend.blend();
		*split_trans = blend.value();
	}
	return result;
}

MMDClips MMDClips::pack() const
{
	MMDClips result;
	for (const auto &clip: clips()) {
		if (clip.frame_range.p0 < clip.frame_range.p1
		    && (clip.weight_range.p0 > 0 || clip.weight_range.p1 > 0)) {
			result.push_back(clip);
		}
	}
	return result;
}

void MMDClips::partition(const Range1f master_range, MMDClips &inside, MMDClips &outside) const
{
	assert(master_range.p1 > master_range.p0);

	for (const auto &clip: clips()) {  // copy
		float frames[2] = {
		        master_range.p0 + clip.frame_offset,
		        master_range.p1 + clip.frame_offset,
		};
		if (frames[0] <= clip.frame_range.p0 && clip.frame_range.p1 <= frames[1]) {
			inside.push_back(clip);
		}
		else {
			outside.push_back(clip);
		}
	}
}

MMDClips MMDClips::trim(const Range1f &master_range) const
{
	MMDClips inside;
	MMDClips outside;

	auto split = this->split(master_range.p0).split(master_range.p1);
	split.partition(master_range, inside, outside);
	return inside;
}

MMDClips MMDClips::adjust(
        const MMDClips &clips, float master_frame, MMDClips *actor, const std::string &bone_name)
{
	const std::vector<std::string> bone_names = {bone_name};

	const Range1f master_range = {
	        master_frame,
	        master_frame + 1,
	};

	actor->copy(clips.clips());
	auto center0 = actor->rehearsal(bone_names, master_range)[0][0];
	actor->copy(this->clips());

	auto center1 = actor->rehearsal(bone_names, master_range)[0][0];
	auto pos0 = center0.t;
	auto pos1 = center1.t;

	return Transformf(pos0 - pos1) * (*this);
}

MMDClips MMDClips::correlate(
        const MMDClips &clips, float master_frame, float cross_window, MMDClips *actor,
        const std::vector<std::string> &bone_names, float search_window)
{
	const auto c_huge = huge();
	auto master_begin = master_frame - cross_window;
	auto master_end = master_frame + cross_window;

	auto begin = master_begin - search_window;
	auto end = master_end + search_window;
	auto range = master_end - master_begin;

	auto frame_range = Range1f(begin, end);

	actor->copy(clips.clips());
	auto transforms_list0 = actor->rehearsal(bone_names, frame_range);

	actor->copy(this->clips());
	auto transforms_list1 = actor->rehearsal(bone_names, frame_range);

	auto min_error = c_huge;
	auto min_delay = 0;
	for (auto delay = -search_window; delay < search_window; delay++) {
		auto error = 0.0f;
		for (auto i = 1u; i < bone_names.size(); i++) {
			auto &centers0 = transforms_list0[0];
			auto &centers1 = transforms_list1[0];
			auto &transforms0 = transforms_list0[i];
			auto &transforms1 = transforms_list1[i];
			for (auto frame = search_window; frame < search_window + range; frame++) {
				auto t0 = transforms0[frame].t - centers0[frame].t;
				auto t1 = transforms1[frame + delay].t - centers1[frame + delay].t;
				error += length(t0 - t1);
			}
		}
		if (error < min_error) {
			min_error = error;
			min_delay = delay;
		}
	}
	auto correlate_clips = this->clips();
	for (auto &clip: correlate_clips) {
		clip.frame_offset += min_delay;
		clip.frame_range.p0 += min_delay;
	}
	return MMDClips(correlate_clips);
}

MMDClips MMDClips::mix(const MMDClips &clips, float master_frame, float window) const
{
	auto master_begin = master_frame - window;
	auto master_end = master_frame + window;

	auto split_clips0 = this->split(master_begin).split(master_end).pack().clips();
	auto split_clips1 = clips.split(master_begin).split(master_end).pack().clips();
	auto range = master_end - master_begin;

	for (auto &clip: split_clips0) {
		auto begin = master_begin + clip.frame_offset;
		clip.weight_range.p0
		        *= std::clamp(lerp(1.0, 0.0, (clip.frame_range.p0 - begin) / range), 0.0, 1.0);
		clip.weight_range.p1
		        *= std::clamp(lerp(1.0, 0.0, (clip.frame_range.p1 - begin) / range), 0.0, 1.0);
	}
	for (auto &clip: split_clips1) {
		float begin = master_begin + clip.frame_offset;
		clip.weight_range.p0
		        *= std::clamp(lerp(0.0, 1.0, (clip.frame_range.p0 - begin) / range), 0.0, 1.0);
		clip.weight_range.p1
		        *= std::clamp(lerp(0.0, 1.0, (clip.frame_range.p1 - begin) / range), 0.0, 1.0);
	}
	vector_cat(split_clips0, split_clips1);
	auto result = MMDClips(split_clips0);
	return result.pack();
}

void MMDClips::report(const char *s) const
{
	if (s) {
		aux_printf("%s:\n", s);
	}

	aux_printf(
	        "\t%3s %3s %8s %8s %8s %8s %8s %5s %5s %5s %5s %5s %5s\n", "No", "id", "offset", "beginA",
	        "endA", "begin", "end", "w0", "w1", "q0", "q1", "z0", "z1");

	for (const auto &clip: clips()) {
		auto i = &clip - &clips().at(0);
		aux_printf(
		        "\t%3d %3d %8.1f %8.1f %8.1f %8.1f %8.1f %5.2f %5.2f %5.1f %5.1f %5.0f %5.0f\n", i,
		        clip.animation_id, clip.frame_offset, clip.frame_range.p0 - clip.frame_offset,
		        clip.frame_range.p1 - clip.frame_offset, clip.frame_range.p0, clip.frame_range.p1,
		        clip.weight_range.p0, clip.weight_range.p1, clip.transforms[0].q.w,
		        clip.transforms[1].q.w, clip.transforms[0].t.z, clip.transforms[1].t.z);
	}
}

}  // namespace spu
