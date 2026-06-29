//
// PMDActor :
//
#pragma once

#include "internal.h"

namespace spu::gs_node::mmd {

class PMDActor : public MMDActor {
public:
	explicit PMDActor(const char *name = nullptr) : MMDActor(name) {}
	explicit PMDActor(const Attrs &attrs) : MMDActor(attrs) {}
	void replacePainter(GsPainter *painter) override;
};

struct PMDFile : public ActorFile {
	struct Header : public ::spu::MMDObject<int32_t> {
		char magic[3];
		float version;
		std::string j_comment;
		std::string e_comment;
	};

	struct Material {
		Vec3f diffuse;
		Vec3f specular;
		Vec3f ambient;
		float specular_power;
		uint32_t count;
		uint8_t toon_index;
		uint8_t edge_flag;
		std::filesystem::path texture_name;
	};

	Header header;

	std::vector<Material> materials;
	uint32_t bone_display_list_count = 0;
	enum { c_toonmap_name_count = 10 };
	std::filesystem::path toonmap_names[c_toonmap_name_count];

	PMDFile() = default;
	PMDFile(const std::filesystem::path &path) : ActorFile(path) {}
	void load() override;

private:
	std::string readString(int32_t size);
	void readHeader();
	void readVertex();
	void readFace();
	void readMaterial();
	void readBone();
	void readIK();
	void readMorph();
	void readMorphDisplayList();
	void readBoneDisplayList();
	void readExt();
	void readToonTexture_name();
	void readRigidBody();
	void readConstraint();
};
}  // namespace spu::gs_node::mmd
