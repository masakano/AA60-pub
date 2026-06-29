//
// PMDFile :
//
#include "pmd.h"

namespace spu::gs_node::mmd {

void PMDFile::load()
{
	readHeader();
	readVertex();
	readFace();
	readMaterial();
	readBone();

	readIK();
	readMorph();
	readMorphDisplayList();
	readBoneDisplayList();

	auto size = file.size();
	if (file.tell() < size) {
		readExt();
	}

	if (file.tell() < size) {
		readToonTexture_name();
	}

	if (file.tell() < size) {
		readRigidBody();
	}

	if (file.tell() < size) {
		readConstraint();
	}
}

std::string PMDFile::readString(int32_t size)
{
	std::vector<char> buf(size + 1, 0);
	file.read(buf.data(), size);
	if (buf[0] != 0) {
		CharsetConverter conv("UTF-8", "SHIFT-JIS");
		auto dst = conv.get(buf.data(), size);
		return std::string(dst.data(), dst.size());
	}
	return "";
}

void PMDFile::readHeader()
{
	header.magic[0] = read<char>();
	header.magic[1] = read<char>();
	header.magic[2] = read<char>();
	header.version = read<float>();
	header.setName(readString(20));
	header.j_comment = readString(256);

	if (strncmp(header.magic, "Pmd", 3) != 0) {
		static_assert("PMD Header Error.", "");
	}

	if (header.version != 1.0F) {
		static_assert("PMD Version Error.", "");
	}
}

void PMDFile::readVertex()
{
	auto vertex_count = read<uint32_t>();

	vertices.resize(vertex_count);
	for (auto& vertex: vertices) {
		vertex.position = read<vec3f_t>();
		vertex.normal = read<vec3f_t>();
		vertex.texcoord = read<vec2f_t>();

		vertex.bone.i[0] = read<int16_t>();
		vertex.bone.i[1] = read<int16_t>();
		vertex.bone.i[2] = 0;
		vertex.bone.i[3] = 0;

		vertex.weight.f[0] = float(read<uint8_t>()) / 100.0f;
		vertex.weight.f[1] = 1.0f - vertex.weight.f[0];
		vertex.weight.f[2] = 0;
		vertex.weight.f[3] = 0;

		auto edge_mag = read<uint8_t>();
	}
}

void PMDFile::readFace()
{
	auto faceCount = read<uint32_t>();
	faces.resize(faceCount / 3);
	for (auto& face: faces) {
		face.vertices[0] = read<uint16_t>();
		face.vertices[1] = read<uint16_t>();
		face.vertices[2] = read<uint16_t>();
	}
}

void PMDFile::readMaterial()
{
	auto materialCount = read<uint32_t>();
	materials.resize(materialCount);
	for (auto& material: materials) {
		material.diffuse = read<vec3f_t>();
		material.diffuse.a = read<float>();
		material.specular_power = read<float>();
		material.specular = read<vec3f_t>();
		material.ambient = read<vec3f_t>();
		material.toon_index = read<uint8_t>();
		material.edge_flag = read<uint8_t>();
		material.count = read<uint32_t>();
		material.texture_name = readString(20);
	}
}

void PMDFile::readBone()
{
	auto boneCount = read<uint16_t>();
	bones.resize(boneCount);
	for (auto& bone: bones) {
		bone = new MMDBone();
	}
	for (auto& bone: bones) {
		bone->setName(readString(20));
		if (bone->name() == "センター") {
			center_bone = bone;
		}

		auto parent_index = read<int16_t>();
		auto parent = parent_index >= 0 ? bones[parent_index] : base_bone;
		bone->setParent(parent);

		auto tail_index = read<int16_t>();  // ignore
		auto type = read<uint8_t>();        // ignore
		auto ik_parent = read<int16_t>();   // ignore
		bone->setOrgBoneworld(Transformf(read<vec3f_t>()));

		if (bone->name() == "左ひざ" || bone->name() == "右ひざ") {
			auto lower_limit = Vec3f(-pi(), 0, 0);
			auto upper_limit = Vec3f(-radians(1.0), 0, 0);  // ad-hoc
			bone->setLimitAngle(true, lower_limit, upper_limit);
		}
	}
}

void PMDFile::readIK()
{
	auto ik_count = read<uint16_t>();

	ik_bones.resize(ik_count);
	for (auto& ik_bone: ik_bones) {
		auto target_index = read<int16_t>();
		assert(target_index >= 0);
		ik_bone = bones[target_index];

		auto effector_index = read<int16_t>();
		assert(effector_index >= 0);

		ik_bone->m_effectorBone = bones[effector_index];
		auto link_count = read<uint8_t>();

		ik_bone->m_iterationCount = read<int16_t>();
		ik_bone->m_rotateLimit = pi() * read<float>();

		for (auto i = 0; i < link_count; i++) {
			auto link_index = read<uint16_t>();
			if ((link_index != 0) && bones[link_index] != center_bone) {
				auto bone = bones[link_index];
				ik_bone->m_ikBones.push_back(bone);
			}
		}
	}
}

void PMDFile::readMorph()
{
	struct MorphVertex {
		int32_t index;
		Vec3f position;
	};
	std::vector<std::vector<MorphVertex>> morph_vertices_list;

	auto morph_count = read<uint16_t>();

	morphs.resize(morph_count);
	for (auto& mmd_morph: morphs) {
		auto morph = new MMDMorph();
		mmd_morph = morph;

		morph->setName(readString(20));
		auto morph_vertex_count = read<uint32_t>();

		std::vector<MorphVertex> vertices(morph_vertex_count);

		auto morph_type = read<uint8_t>();
		for (auto& vertex: vertices) {
			vertex.index = read<uint32_t>();
			vertex.position = read<vec3f_t>();
		}
		morph_vertices_list.push_back(vertices);
	}

	auto base_factors = morphs[0]->getSpuFactors();
	for (auto& morph: morphs) {
		auto morph_index = &morph - &morphs[0];
		auto base_morph = morphs[0];
		auto& vertices = morph_vertices_list[morph_index];
		for (auto& vertex: vertices) {
			MMDMorph::SpuFactor factor = {
			        .index = morph_index == 0 ? vertex.index :
			                                    base_morph->getSpuFactors()[vertex.index].index,
			        .type = 1,  // position only
			        .spu_index = int32_t(morph_targets.size()),
			};
			morph->getSpuFactors().push_back(factor);
			morph_targets.emplace_back(vertex.position);
		}
	}
}

void PMDFile::readMorphDisplayList()
{
	auto display_list_count = read<uint8_t>();
	for (auto i = 0; i < display_list_count; i++) {
		auto display_list = read<uint16_t>();
	}
}

void PMDFile::readBoneDisplayList()
{
	bone_display_list_count = read<uint8_t>() + 1;
	for (auto i = 0u; i < bone_display_list_count; i++) {
		if (i != 0) {
			auto name = readString(50);
		}
	}

	auto display_count = read<uint32_t>();
	for (auto i = 0u; i < display_count; i++) {
		auto bone_index = read<uint16_t>();
		auto frame_index = read<uint8_t>();
	}
}

void PMDFile::readExt()
{
	auto has_e_name = read<uint8_t>();

	if (has_e_name != 0) {
		header.setEnglishName(readString(20));
		header.e_comment = readString(256);

		// Bone_name
		for (auto& bone: bones) {
			bone->setEnglishName(readString(20));
		}

		auto morph_count = morphs.size();
		for (auto blend_shape_index = 1; blend_shape_index < morph_count; blend_shape_index++) {
			auto& morph = morphs[blend_shape_index];
			morph->setEnglishName(readString(20));
		}

		for (auto display_index = 1; display_index < bone_display_list_count; display_index++) {
			auto e_name = readString(50);
		}
	}
}

void PMDFile::readToonTexture_name()
{
	for (auto& toonmap_name: toonmap_names) {
		toonmap_name = readString(100);
	}
}

void PMDFile::readRigidBody()
{
	auto rigid_body_count = read<uint32_t>();
	rigid_body_descs.resize(rigid_body_count);
	for (auto& desc: rigid_body_descs) {
		readString(20);  // j_name

		desc.index = read<int16_t>();
		desc.group_index = read<uint8_t>();
		desc.group_mask = read<uint16_t>();
		desc.shape_type = read<uint8_t>();
		desc.shape_size.x = read<float>();
		desc.shape_size.y = read<float>();
		desc.shape_size.z = read<float>();

		Vec3f position = read<vec3f_t>();
		Vec3f orientation = read<vec3f_t>();
		desc.transform = Transformf(position, Quatf::from_eulerZYX(orientation));

		desc.mass = read<float>();
		desc.linear_dumping = read<float>();
		desc.angular_dumping = read<float>();
		desc.restitution = read<float>();
		desc.friction = read<float>();

		auto mtype = read<uint8_t>();
		desc.mix_rate = mtype == 0 ? 0.0 : mtype == 1 ? 1.0 : 0.5;

		if (desc.mix_rate > 0) {                           // not kinematic
			desc.shape_size = desc.shape_size * 0.67;  // need parameterize
		}
	}
}

void PMDFile::readConstraint()
{
	auto constraint_count = read<uint32_t>();

	constraint_descs.resize(constraint_count);

	for (auto& desc: constraint_descs) {
		readString(20);  // j_name

		desc.indexA = read<int32_t>();
		desc.indexB = read<int32_t>();

		Vec3f position = read<vec3f_t>();
		Vec3f orientation = read<vec3f_t>();
		desc.transform = Transformf(position, Quatf::from_eulerZYX(orientation));

		desc.linear_lower_limit = read<vec3f_t>();
		desc.linear_upper_limit = read<vec3f_t>();
		desc.angular_lower_limit = read<vec3f_t>();
		desc.angular_upper_limit = read<vec3f_t>();
		desc.translate_stiffness = read<vec3f_t>();
		desc.rotate_stiffness = read<vec3f_t>();
	}
}
}  // namespace spu::gs_node::mmd
