//
// VMDFile :
//
#include "vmd_file.h"

namespace spu::gs_node::mmd {

void VMDFile::load()
{
	readHeader();
	readBone();

	auto size = file.size();

	if (file.tell() < size) {
		readMorph();
	}
	if (file.tell() < size) {
		readCamera();
	}
	if (file.tell() < size) {
		readLight();
	}
	if (file.tell() < size) {
		readShadow();
	}
	if (file.tell() < size) {
		readIk();
	}
}

std::string VMDFile::readString(int32_t size)
{
	std::vector<char> buf(size + 1, 0);
	file.read(buf.data(), size);
	if (buf[0] != 0) {
		CharsetConverter conv("UTF-8", "SHIFT-JIS");
		auto dst = conv.get(buf.data(), size);
		return {dst.data(), dst.size()};
	}
	return "";
}

void VMDFile::readHeader()
{
	header.magic = readString(30);
	header.name = readString(20);

	if (header.magic != "Vocaloid Motion Data 0002" && header.magic != "Vocaloid Motion Data") {
		assert(0);
	}
}

void VMDFile::readBone()
{
	auto bone_count = read<uint32_t>();
	bones.resize(bone_count);
	for (auto &bone: bones) {
		bone.name = readString(15);
		bone.frame = read<uint32_t>();
		bone.translate = read<vec3f_t>();
		bone.quaternion = read<Quatf>();
		bone.interpolation = read<std::array<u_char, 64>>();
	}
}

void VMDFile::readMorph()
{
	auto morph_count = read<uint32_t>();

	morphs.resize(morph_count);
	for (auto &morph: morphs) {
		morph.name = readString(15);
		morph.frame = read<uint32_t>();
		morph.weight = read<float>();
	}
}

void VMDFile::readCamera()
{
	auto camera_count = read<uint32_t>();

	cameras.resize(camera_count);
	for (auto &camera: cameras) {
		camera.frame = read<uint32_t>();
		camera.distance = read<float>();
		camera.interest = read<vec3f_t>();
		camera.rotate = read<vec3f_t>();
		camera.interpolation = read<std::array<u_char, 24>>();
		camera.viewAngle = read<uint32_t>();
		camera.isPerspective = read<uint8_t>();
	}
}

void VMDFile::readLight()
{
	auto light_count = read<uint32_t>();

	lights.resize(light_count);
	for (auto &light: lights) {
		light.frame = read<uint32_t>();
		light.color = read<vec3f_t>();
		light.position = read<vec3f_t>();
	}
}

void VMDFile::readShadow()
{
	auto shadow_count = read<uint32_t>();

	shadows.resize(shadow_count);
	for (auto &shadow: shadows) {
		shadow.frame = read<uint32_t>();
		shadow.shadowType = read<uint8_t>();
		shadow.distance = read<float>();
	}
}

void VMDFile::readIk()
{
	auto ik_count = read<uint32_t>();
	for (auto i = 0u; i < ik_count; i++) {
		auto ik_frame = read<uint32_t>();
		auto ik_show = read<uint8_t>();
		auto ik_info_count = read<uint32_t>();

		for (auto j = 0u; j < ik_info_count; j++) {
			VMDFile::IkInfo ik_info;
			ik_info.frame = ik_frame;
			ik_info.show = ik_show;
			ik_info.name = readString(20);
			ik_info.enable = read<uint8_t>();
			ik_infos.push_back(ik_info);
		}
	}
}

namespace {
const std::string &loadKeyValue(
        const gs_node::mmd::VMDFile::Bone &fbone, std::pair<float, Transformf> *key_value)
{
	key_value->first = fbone.frame;
	key_value->second.t = fbone.translate;
	key_value->second.q = fbone.quaternion;

	// get(64);  // interporation table (skip) // interpolate here

	return fbone.name;
}

const std::string &loadKeyValue(const gs_node::mmd::VMDFile::Morph &fmorph, std::pair<float, float> *key_value)
{
	key_value->first = fmorph.frame;
	key_value->second = fmorph.weight;
	return fmorph.name;
}

template<class file_item_t, class motion_t>
void loadMotion(std::vector<file_item_t> &file_items, std::vector<motion_t> &motions)
{
	std::map<std::string, std::vector<typename motion_t::key_value_t>> key_value_list;
	for (auto &file_item: file_items) {
		typename motion_t::key_value_t key_value;
		const std::string &name = loadKeyValue(file_item, &key_value);
		key_value_list[name].push_back(key_value);
	}

	for (auto &pair: key_value_list) {
		auto &name = pair.first;
		auto &key_values = pair.second;

		auto compar = [&](const motion_t &motion) { return motion.name() == name; };
		auto it = find_if(motions.begin(), motions.end(), compar);
		aux_error(it != motions.end(), "duplicated name %s\n", name.c_str());

		motions.emplace_back(motion_t(name, key_values));
	}
}
}  // namespace

MMDAnimation *createFromVMDFile(const std::filesystem::path &path)
{
	gs_node::mmd::VMDFile desc(path);
	desc.load();

	std::string name;
	std::vector<MMDMotion<Transformf>> bone_motion_pool;
	std::vector<MMDMotion<float>> morph_motion_pool;

	name = desc.header.name;
	loadMotion(desc.bones, bone_motion_pool);
	loadMotion(desc.morphs, morph_motion_pool);

	auto *animation = new MMDAnimation();
	animation->init(name, bone_motion_pool, morph_motion_pool);
	return animation;
}

}  // namespace spu::gs_node::mmd
