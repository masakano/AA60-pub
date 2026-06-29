//
// MMDFile :
//
#pragma once
#include <gsys/node/mmd.h>

namespace spu::gs_node::mmd {

struct MMDFile {
	MMDFile() = default;
	MMDFile(const std::filesystem::path &path) : file(path, "r") {}
	virtual ~MMDFile() = default;

protected:
	File file;
	template<class T> T read()
	{
		T value;
		file.read(&value, sizeof(T));
		return value;
	}
	template<class T> void read(T *values, size_t size) { file.read(values, size * sizeof(T)); }
	virtual void load() {}
};

struct ActorFile : public MMDFile {
	struct Vertex {
		Vec3f position;
		Vec3f normal;
		Vec2f texcoord;
		Vec4i bone;
		Vec4f weight;
	};

	struct Face {
		uint32_t vertices[3];
	};
	std::vector<Vertex> vertices;
	std::vector<Face> faces;

	MMDBone *base_bone = nullptr;
	MMDBone *center_bone = nullptr;
	std::vector<MMDBone *> bones;
	std::vector<MMDBone *> ik_bones;
	std::vector<GsBullet::RigidBodyDesc> rigid_body_descs;
	std::vector<GsBullet::ConstraintDesc> constraint_descs;

	std::vector<MMDMorph *> morphs;
	std::vector<Vec4f> morph_targets;

	ActorFile() = default;
	ActorFile(const std::filesystem::path &path) : MMDFile(path)
	{
		base_bone = new MMDBone();  // not owner
		base_bone->setName("トップ");
		load();
	}
};
}  // namespace spu::gs_node::mmd
