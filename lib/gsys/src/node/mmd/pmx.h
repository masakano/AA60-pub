//
// PMXMorph :
//
#pragma once

// #include <gsys/node/mmd.h>
#include "internal.h"

namespace spu::gs_node::mmd {
struct PMXFile;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#endif

class PMXMorph : public MMDMorph {
public:
	enum MorphType {
		e_group = 0,
		e_position,
		e_bone,
		e_uv,
		e_addUv1,
		e_addUv2,
		e_addUv3,
		e_addUv4,
		e_material,
		e_flip,
		e_impluse,
	};

	struct FlipFactor {
		int32_t index;
		float weight;
	};

	struct ImpulseFactor {
		int32_t index;
		uint8_t localFlag;  // 0:OFF 1:ON
		Vec3f translateVelocity;
		Vec3f rotateTorque;
	};

	struct BoneFactor {
		int32_t index;
		Transformf transform;
	};
	struct MaterialFactor {
		enum Type {
			e_mul,
			e_add,
		};
		int32_t index;
		uint8_t type;

		Vec4f diffuse;
		Vec3f ambient;
		Vec3f specular;
		Vec4f edge_color;
		Vec4f diffusemap_factor;
		Vec4f spheremap_factor;
		Vec4f toonmap_factor;
		float specular_power;
		float edge_width;
	};
	struct GroupFactor {
		int32_t index;
		float weight;
	};
	void update(MMDActor *pmx) override;

private:
	friend class PMXFile;
	uint8_t m_controlPanel;  // 1:眉(左下) 2:目(左上) 3:口(右上) 4:その他(右下)  | 0:システム予約
	uint8_t m_type;

	std::vector<FlipFactor> m_flipFactors;
	std::vector<ImpulseFactor> m_impulseFactors;
	std::vector<GroupFactor> m_groupFactors;
	std::vector<BoneFactor> m_boneFactors;
	std::vector<MaterialFactor> m_materialFactors;
};

class PMXActor : public MMDActor {
public:
	explicit PMXActor(const char *name = nullptr) : MMDActor(name) {}
	explicit PMXActor(const Attrs &attrs) : MMDActor(attrs) {}
	void replacePainter(GsPainter *painter) override;

private:
	std::vector<SpuTexture> m_textures;
};

enum PMXWeightType {
	e_BDEF1 = 0,
	e_BDEF2,
	e_BDEF4,
	e_SDEF,
	e_QDEF,
};

struct PMXMaterial : public ::spu::MMDObject<int32_t> {
	union Flags {
		struct {
			uint32_t both_face          : 1;
			uint32_t ground_shadow      : 1;
			uint32_t cast_self_shadow   : 1;
			uint32_t receive_self_shadow: 1;
			uint32_t draw_edge          : 1;
			uint32_t vertex_color       : 1;
			uint32_t draw_point         : 1;
			uint32_t invisible          : 1;  // additional
		};
		uint8_t bits;
	};

	enum ToonMode {
		e_custom = 0,
		e_common,
	};
	std::string memo;

	Flags flags;

	Vec4f diffuse = ezero<Vec4f>();
	Vec4f edgeColor = ezero<Vec4f>();
	Vec3f specular = ezero();
	Vec3f ambient = ezero();
	float specular_power = 0;
	float edge_size = 0;
	int32_t diffusemap_index = 0;
	int32_t spheremap_index = 0;
	int32_t toonmap_index = 0;
	int32_t count = 0;
	uint8_t toon_mode = 0;
	uint8_t sphere_mode = 0;
};

struct PMXFile : public ActorFile {
	struct Header {
		uint32_t magic;
		float version;
		uint8_t data_size;
		uint8_t encode;  // 0:UTF16 1:UTF8
		uint8_t add_uv_count;
		uint8_t vertex_index_size;
		uint8_t texture_index_size;
		uint8_t material_index_size;
		uint8_t bone_index_size;
		uint8_t morph_index_size;
		uint8_t rigid_body_index_size;
	};

	struct Info : public ::spu::MMDObject<int32_t> {
		std::string j_comment;
		std::string e_comment;
	};

	union Flags {
		struct {
			uint32_t target_show_mode    : 1;  // editor only
			uint32_t allow_rotate        : 1;  // editor only
			uint32_t allow_translate     : 1;  // editor only
			uint32_t visible             : 1;  // editor only
			uint32_t allow_control       : 1;  // editor only
			uint32_t invk                : 1;
			uint32_t pad0                : 1;
			uint32_t grant_local         : 1;
			uint32_t grant_rotate        : 1;
			uint32_t grant_translate     : 1;
			uint32_t fixed_axis          : 1;  // editor only
			uint32_t local_axis          : 1;  // editor only
			uint32_t deform_after_physics: 1;
			uint32_t deform_outer_parent : 1;
		};
		uint16_t bits = 0;
	};

	Header header;
	Info info;

	std::vector<std::filesystem::path> textures;
	std::vector<PMXMaterial> materials;

	PMXFile(const std::filesystem::path &path) : ActorFile(path) {}
	void load() override;

private:
	std::string readString();
	int32_t readIndex(uint8_t indexSize);
	void readHeader();
	void readInfo();
	void readVertex();
	void readFace();
	void readTexture();
	void readMaterial();
	void readBone();
	void readMorph();
	void readDisplayFrame();
	void readRigidBody();
	void readConstraint();
	void readSoftbody();
};
}  // namespace spu::gs_node::mmd

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
