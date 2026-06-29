//
// VMDFile :
//
#pragma once

#include "internal.h"

namespace spu::gs_node::mmd {

struct VMDFile : public MMDFile {
	struct Header {
		std::string magic;
		std::string name;
	};

	struct Bone {
		std::string name;
		uint32_t frame;
		vec3f_t translate;
		Quatf quaternion;
		std::array<u_char, 64> interpolation;
	};

	struct Morph {
		std::string name;
		uint32_t frame;
		float weight;
	};

	struct IkInfo {
		std::string name;
		uint32_t frame;
		uint8_t show;
		uint8_t enable;
	};

	struct Camera {
		uint32_t frame;
		float distance;
		vec3f_t interest;
		vec3f_t rotate;
		std::array<u_char, 24> interpolation;
		uint32_t viewAngle;
		uint8_t isPerspective;
	};

	struct Light {
		uint32_t frame;
		vec3f_t color;
		vec3f_t position;
	};

	struct Shadow {
		uint32_t frame;
		uint8_t shadowType;  // 0:Off 1:mode1 2:mode2
		float distance;
	};

	Header header;
	std::vector<Bone> bones;
	std::vector<Morph> morphs;
	std::vector<Camera> cameras;
	std::vector<Light> lights;
	std::vector<Shadow> shadows;
	std::vector<IkInfo> ik_infos;

	VMDFile(const std::filesystem::path &path) : MMDFile(path) {}
	void load() override;

private:
	std::string readString(int32_t size);
	void readHeader();
	void readBone();
	void readMorph();
	void readCamera();
	void readLight();
	void readShadow();
	void readIk();
};
}  // namespace spu::gs_node::mmd
