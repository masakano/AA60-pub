//
// PMXFile :
//
#include "pmx.h"

namespace spu::gs_node::mmd {

void PMXFile::load()
{
	readHeader();
	readInfo();
	readVertex();
	readFace();
	readTexture();
	readMaterial();
	readBone();
	readMorph();
	readDisplayFrame();
	readRigidBody();
	readConstraint();

	if (file.tell() < file.size()) {
		readSoftbody();
	}
	ik_bones = bones;  // copy at last
}

std::string PMXFile::readString()
{
	auto size = read<uint32_t>();

	if (size > 0) {
		if (header.encode == 0) {
			std::vector<char> buf(size);
			file.read(buf.data(), size);
			CharsetConverter conv("UTF-8", "UTF-16LE");  // NOT SJIS!
			auto dst = conv.get(buf.data(), size);
			return std::string(dst.data(), dst.size());
		}
		if (header.encode == 1) {
			std::string u8str(size, '\0');
			file.read(u8str.data(), size);
			return u8str;
		}
	}
	return "";
}

int32_t PMXFile::readIndex(uint8_t indexSize)
{
	switch (indexSize) {
	case 1: {
		auto idx = read<uint8_t>();
		return idx != 0xFF ? idx : -1;
	}
	case 2: {
		auto idx = read<uint16_t>();
		return idx != 0xFFFF ? idx : -1;
	}
	case 4: {
		return read<int32_t>();
	}
	default: assert(0); return -1;
	}
}

void PMXFile::readHeader()
{
	header.magic = read<uint32_t>();
	header.version = read<float>();
	header.data_size = read<uint8_t>();
	header.encode = read<uint8_t>();
	header.add_uv_count = read<uint8_t>();
	header.vertex_index_size = read<uint8_t>();
	header.texture_index_size = read<uint8_t>();
	header.material_index_size = read<uint8_t>();
	header.bone_index_size = read<uint8_t>();
	header.morph_index_size = read<uint8_t>();
	header.rigid_body_index_size = read<uint8_t>();
}

void PMXFile::readInfo()
{
	info.setName(readString());
	info.setEnglishName(readString());
	info.j_comment = readString();
	info.e_comment = readString();
}

void PMXFile::readVertex()
{
	auto count = read<int32_t>();
	vertices.resize(count);
	for (auto& vertex: vertices) {
		vertex.position = read<vec3f_t>();
		vertex.normal = read<vec3f_t>();
		vertex.texcoord = read<vec2f_t>();

		for (uint8_t i = 0; i < header.add_uv_count; i++) {
			/*auto aux_texcoord = */ read<vec4f_t>();
		}

		// vertex.weight_type = read<uint8_t>();
		auto weight_type = read<uint8_t>();
		switch (weight_type) {
		case PMXWeightType::e_BDEF1: {
			vertex.bone.i[0] = readIndex(header.bone_index_size);
			vertex.bone.i[1] = 0;
			vertex.bone.i[2] = 0;
			vertex.bone.i[3] = 0;

			vertex.weight.f[0] = 1.0;
			vertex.weight.f[1] = 0.0;
			vertex.weight.f[2] = 0.0;
			vertex.weight.f[3] = 0.0;

			break;
		}
		case PMXWeightType::e_BDEF2: {
			vertex.bone.i[0] = readIndex(header.bone_index_size);
			vertex.bone.i[1] = readIndex(header.bone_index_size);
			vertex.bone.i[2] = 0;
			vertex.bone.i[3] = 0;

			vertex.weight.f[0] = read<float>();
			vertex.weight.f[1] = 1.0 - vertex.weight.f[0];
			vertex.weight.f[2] = 0.0;
			vertex.weight.f[3] = 0.0;

			break;
		}
		case PMXWeightType::e_BDEF4: {
			vertex.bone.i[0] = readIndex(header.bone_index_size);
			vertex.bone.i[1] = readIndex(header.bone_index_size);
			vertex.bone.i[2] = readIndex(header.bone_index_size);
			vertex.bone.i[3] = readIndex(header.bone_index_size);

			vertex.weight.f[0] = read<float>();
			vertex.weight.f[1] = read<float>();
			vertex.weight.f[2] = read<float>();
			vertex.weight.f[3] = read<float>();

			break;
		}
		case PMXWeightType::e_SDEF: {
			vertex.bone.i[0] = readIndex(header.bone_index_size);
			vertex.bone.i[1] = readIndex(header.bone_index_size);
			vertex.bone.i[2] = 0;
			vertex.bone.i[3] = 0;

			vertex.weight.f[0] = read<float>();
			vertex.weight.f[1] = 1.0 - vertex.weight.f[0];
			vertex.weight.f[2] = 0.0;
			vertex.weight.f[3] = 0.0;

			/*auto sdef_C = */ read<vec3f_t>();
			/*auto sdef_R0 = */ read<vec3f_t>();
			/*auto sdef_R1 = */ read<vec3f_t>();
			break;
		}
		case PMXWeightType::e_QDEF: {
			vertex.bone.i[0] = readIndex(header.bone_index_size);
			vertex.bone.i[1] = readIndex(header.bone_index_size);
			vertex.bone.i[2] = readIndex(header.bone_index_size);
			vertex.bone.i[3] = readIndex(header.bone_index_size);

			vertex.weight.f[0] = read<float>();
			vertex.weight.f[1] = read<float>();
			vertex.weight.f[2] = read<float>();
			vertex.weight.f[3] = read<float>();

			break;
		}
		default: assert(0);
		}
		auto edge_mag = read<float>();
	}
}

void PMXFile::readFace()
{
	auto count = read<int32_t>() / 3;

	faces.resize(count);

	switch (header.vertex_index_size) {
	case 1: {
		std::vector<uint8_t> vertices(count * 3);
		read(vertices.data(), vertices.size());
		for (auto faceIdx = 0; faceIdx < count; faceIdx++) {
			faces[faceIdx].vertices[0] = vertices[faceIdx * 3 + 0];
			faces[faceIdx].vertices[1] = vertices[faceIdx * 3 + 1];
			faces[faceIdx].vertices[2] = vertices[faceIdx * 3 + 2];
		}
		break;
	}
	case 2: {
		std::vector<uint16_t> vertices(count * 3);
		read(vertices.data(), vertices.size());
		for (auto faceIdx = 0; faceIdx < count; faceIdx++) {
			faces[faceIdx].vertices[0] = vertices[faceIdx * 3 + 0];
			faces[faceIdx].vertices[1] = vertices[faceIdx * 3 + 1];
			faces[faceIdx].vertices[2] = vertices[faceIdx * 3 + 2];
		}
		break;
	}
	case 4: {
		std::vector<uint32_t> vertices(count * 3);
		read(vertices.data(), vertices.size());
		for (auto faceIdx = 0; faceIdx < count; faceIdx++) {
			faces[faceIdx].vertices[0] = vertices[faceIdx * 3 + 0];
			faces[faceIdx].vertices[1] = vertices[faceIdx * 3 + 1];
			faces[faceIdx].vertices[2] = vertices[faceIdx * 3 + 2];
		}
		break;
	}
	default: assert(0);
	}
}

void PMXFile::readTexture()
{
	auto count = read<int32_t>();
	textures.resize(count);
	for (auto& texture: textures) {
		auto path_str = readString();
		std::replace(path_str.begin(), path_str.end(), '\\', '/');  // auto convert does not work
		texture = path_str;
	}
}

void PMXFile::readMaterial()
{
	auto count = read<int32_t>();

	materials.resize(count);

	for (auto& mat: materials) {
		mat.setName(readString());
		mat.setEnglishName(readString());

		mat.diffuse = read<vec4f_t>();
		mat.specular = read<vec3f_t>();
		mat.specular_power = read<float>();
		mat.ambient = read<vec3f_t>();
		mat.flags.bits = read<uint8_t>();
		mat.edgeColor = read<vec4f_t>();

		mat.edge_size = read<float>();
		mat.diffusemap_index = readIndex(header.texture_index_size);
		mat.spheremap_index = readIndex(header.texture_index_size);
		mat.sphere_mode = read<uint8_t>();
		mat.toon_mode = read<uint8_t>();

		if (mat.toon_mode == PMXMaterial::e_custom) {
			mat.toonmap_index = readIndex(header.texture_index_size);
		}
		else if (mat.toon_mode == PMXMaterial::e_common) {
			auto toon_index = read<uint8_t>();
			mat.toonmap_index = toon_index;
		}
		else {
			assert(0);
		}

		mat.memo = readString();
		mat.count = read<int32_t>();
	}
}

void PMXFile::readBone()
{
	auto count = read<int32_t>();

	bones.resize(count);
	for (auto& bone: bones) {
		bone = new MMDBone();
	}

	for (auto& bone: bones) {
		bone->setName(readString());
		bone->setEnglishName(readString());
		if (bone->name() == "センター") {
			center_bone = bone;
		}

		bone->setOrgBoneworld(Transformf(read<vec3f_t>()));

		auto parent_index = read<int16_t>();
		auto parent = parent_index >= 0 ? bones[parent_index] : base_bone;
		bone->setParent(parent);

		auto deform_depth = read<int32_t>();  // ignore
		PMXFile::Flags flags;
		flags.bits = read<uint16_t>();

		if (flags.target_show_mode == 0) {
			/*auto local_bone_position = */ read<vec3f_t>();  // ignore
		}
		else {
			/*auto link_bone_index = */ readIndex(header.bone_index_size);  // ignore
		}

		if (flags.grant_rotate || flags.grant_translate) {
			auto index = readIndex(header.bone_index_size);
			bone->m_grantorBone = bones[index];
			bone->m_grantWeight = read<float>();
			bone->m_grantRotate = flags.grant_rotate;
			bone->m_grantTranslate = flags.grant_translate;
		}

		if (flags.fixed_axis) {
			/*auto fixed_axis = */ read<vec3f_t>();  // ignore
		}

		if (flags.local_axis) {
			/*auto local_axis_x = */ read<vec3f_t>();  // ignore
			/*auto local_axis_z = */ read<vec3f_t>();  // ignore
		}

		if (flags.deform_outer_parent) {
			/*auto key_value = */ read<int32_t>();  // ignore
		}

		if (flags.invk) {
			auto index = readIndex(header.bone_index_size);
			bone->m_effectorBone = bones[index];
			bone->m_iterationCount = read<int32_t>();
			bone->m_rotateLimit = read<float>();

			auto link_count = read<int32_t>();
			bone->m_ikBones.resize(link_count);
			for (auto& ik_bone: bone->m_ikBones) {
				auto ik_bone_index = readIndex(header.bone_index_size);
				ik_bone = bones[ik_bone_index];

				auto is_limit = read<uint8_t>();
				if (is_limit) {
					Vec3f lower_limit_radian = read<vec3f_t>();
					Vec3f upper_limit_radian = read<vec3f_t>();
					ik_bone->setLimitAngle(true, lower_limit_radian, upper_limit_radian);
				}
				else {
					ik_bone->setLimitAngle(false);
				}
			}
		}
	}
}

void PMXFile::readMorph()
{
	auto count = read<int32_t>();
	morphs.resize(count);

	for (auto& mmd_morph: morphs) {
		auto morph = new PMXMorph();
		mmd_morph = morph;

		morph->setName(readString());
		morph->setEnglishName(readString());
		morph->m_controlPanel = read<uint8_t>();
		morph->m_type = read<uint8_t>();

		auto factor_count = read<int32_t>();

		if (morph->m_type == PMXMorph::e_position) {
			morph->m_spuFactors.resize(factor_count);
			for (auto& factor: morph->m_spuFactors) {
				auto index = readIndex(header.vertex_index_size);
				auto position = read<vec3f_t>();
				factor.type = morph->m_type;
				factor.index = index;
				factor.spu_index = morph_targets.size();
				morph_targets.push_back(position);
			}
		}
		else if (
		        morph->m_type == PMXMorph::e_uv || morph->m_type == PMXMorph::e_addUv1
		        || morph->m_type == PMXMorph::e_addUv2 || morph->m_type == PMXMorph::e_addUv3
		        || morph->m_type == PMXMorph::e_addUv4) {
			morph->m_spuFactors.resize(factor_count);
			for (auto& factor: morph->m_spuFactors) {
				auto index = readIndex(header.vertex_index_size);
				auto uv = read<vec4f_t>();
				factor.type = morph->m_type;
				factor.index = index;
				factor.spu_index = morph_targets.size();
				morph_targets.push_back(uv);
			}
		}
		else if (morph->m_type == PMXMorph::e_bone) {
			morph->m_boneFactors.resize(factor_count);
			for (auto& factor: morph->m_boneFactors) {
				factor.index = readIndex(header.bone_index_size);
				factor.transform.t = read<vec3f_t>();
				factor.transform.q = read<Quatf>();
			}
		}
		else if (morph->m_type == PMXMorph::e_material) {
			morph->m_materialFactors.resize(factor_count);
			for (auto& factor: morph->m_materialFactors) {
				factor.index = readIndex(header.material_index_size);
				factor.type = read<uint8_t>();
				factor.diffuse = read<vec4f_t>();
				factor.specular = read<vec3f_t>();
				factor.specular_power = read<float>();
				factor.ambient = read<vec3f_t>();
				factor.edge_color = read<vec4f_t>();
				factor.edge_width = read<float>();
				factor.diffusemap_factor = read<vec4f_t>();
				factor.spheremap_factor = read<vec4f_t>();
				factor.toonmap_factor = read<vec4f_t>();
			}
		}
		else if (morph->m_type == PMXMorph::e_group) {
			morph->m_groupFactors.resize(factor_count);
			for (auto& factor: morph->m_groupFactors) {
				factor.index = readIndex(header.morph_index_size);
				factor.weight = read<float>();
			}
		}
		else if (morph->m_type == PMXMorph::e_flip) {
			morph->m_flipFactors.resize(factor_count);
			for (auto& factor: morph->m_flipFactors) {
				factor.index = readIndex(header.morph_index_size);
				factor.weight = read<float>();
			}
		}
		else if (morph->m_type == PMXMorph::e_impluse) {
			morph->m_impulseFactors.resize(factor_count);
			for (auto& factor: morph->m_impulseFactors) {
				factor.index = readIndex(header.rigid_body_index_size);
				factor.localFlag = read<uint8_t>();
				factor.translateVelocity = read<vec3f_t>();
				factor.rotateTorque = read<vec3f_t>();
			}
		}
		else {
			aux_error(true, "Unsupported Morph Type:%d", morph->m_type);
		}
	}
}

void PMXFile::readDisplayFrame()
{
	// skip all
	enum TargetType {
		e_bone_index,
		e_morph_index,
	};

	auto display_frame_count = read<int32_t>();
	for (auto i = 0; i < display_frame_count; i++) {
		auto j_name = readString();
		auto e_name = readString();
		auto flag = read<uint8_t>();

		auto target_count = read<int32_t>();
		for (auto j = 0; j < target_count; j++) {
			auto type = read<uint8_t>();
			if (type == e_bone_index) {
				auto index = readIndex(header.bone_index_size);
			}
			else if (type == e_morph_index) {
				auto index = readIndex(header.morph_index_size);
			}
			else {
				assert(0);
			}
		}
	}
}

void PMXFile::readRigidBody()
{
	auto count = read<int32_t>();
	rigid_body_descs.resize(count);

	for (auto& desc: rigid_body_descs) {
		readString();  // j_name
		readString();  // e_name

		desc.index = readIndex(header.bone_index_size);
		desc.group_index = read<uint8_t>();
		desc.group_mask = read<uint16_t>();
		desc.shape_type = read<uint8_t>();
		desc.shape_size = read<vec3f_t>();

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

		// PATCH
		if (desc.mix_rate > 0) {                          // not kinematic
			desc.shape_size = desc.shape_size * 0.5;  // need parameterize
		}
	}
}

void PMXFile::readConstraint()
{
	auto count = read<int32_t>();
	constraint_descs.resize(count);

	for (auto& desc: constraint_descs) {
		readString();  // j_name
		readString();  // e_name

		desc.spring_type = read<uint8_t>();
		desc.indexA = readIndex(header.rigid_body_index_size);
		desc.indexB = readIndex(header.rigid_body_index_size);

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

void PMXFile::readSoftbody()
{
	auto softbody_count = read<int32_t>();

	for (auto i = 0; i < softbody_count; i++) {
		auto j_name = readString();
		auto e_name = readString();

		auto type = read<uint8_t>();
		auto material_index = readIndex(header.material_index_size);
		auto group = read<uint8_t>();
		auto collision_group = read<uint16_t>();
		auto flag = read<uint8_t>();
		auto blink_length = read<int32_t>();
		auto cluster_count = read<int32_t>();
		auto total_mass = read<float>();
		auto collision_margin = read<float>();
		auto aero_model = read<int32_t>();
		auto VCF = read<float>();
		auto DP = read<float>();
		auto DG = read<float>();
		auto LF = read<float>();
		auto PR = read<float>();
		auto VC = read<float>();
		auto DF = read<float>();
		auto MT = read<float>();
		auto CHR = read<float>();
		auto KHR = read<float>();
		auto SHR = read<float>();
		auto AHR = read<float>();
		auto SRHR_CL = read<float>();
		auto SKHR_CL = read<float>();
		auto SSHR_CL = read<float>();
		auto SR_SPLT_CL = read<float>();
		auto SK_SPLT_CL = read<float>();
		auto SS_SPLT_CL = read<float>();
		auto V_IT = read<int32_t>();
		auto P_IT = read<int32_t>();
		auto D_IT = read<int32_t>();
		auto C_IT = read<int32_t>();
		auto LST = read<float>();
		auto AST = read<float>();
		auto VST = read<float>();

		auto anchor_rigid_body_count = read<int32_t>();
		for (auto j = 0; j < anchor_rigid_body_count; j++) {
			auto rigid_body_index = readIndex(header.rigid_body_index_size);
			auto vertex_index = readIndex(header.vertex_index_size);
			auto near_mode = read<uint8_t>();
		}

		auto pin_vertex_index_count = read<int32_t>();
		for (auto j = 0; j < pin_vertex_index_count; j++) {
			auto pv = readIndex(header.vertex_index_size);
		}
	}
}
}  // namespace spu::gs_node::mmd
