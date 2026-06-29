//
// Parser :
//
#pragma once

#include "metavert.h"
#include <stack>
#include <utility>

namespace spu::gs_node::wavefront {

class Parser {
public:
	Parser(std::vector<Mesh> &meshes) : m_meshes(meshes) {}

	void load(const std::filesystem::path &name);

	std::vector<std::string> paths() const
	{
		std::vector<std::string> str_paths;
		for (const auto &path: m_paths) {
			str_paths.push_back(path.string());
		}
		return str_paths;
	}

private:
	const std::map<std::string, std::string> c_blacklist = {
	        {"ambient",                "drawcall.ub_material.ao"                    },
	        {"albedo",                 "drawcall.ub_material.albedo"                },
	        {"emission",               "drawcall.ub_material.emission"              },
	        {"roughness",              "drawcall.ub_material.roughness"             },
	        {"metallic",               "drawcall.ub_material.metallic"              },
	        {"ao",                     "drawcall.ub_material.ao"                    },
	        {"refractive_index",       "drawcall.ub_material.refractive_index"      },
	        {"absorption_coefficient", "drawcall.ub_material.absorption_coefficient"},
	        {"min_alpha",              "drawcall.ub_material.min_alpha"             },
	        {"gamma",                  "drawcall.ub_material.min_alpha"             },
	        {"height_scale",           "drawcall.ub_material.height_scale"          },
	        {"point_size",             "drawcall.ub_material.point_size"            },
	        {"albedomap",              "drawcall.texture.albedo.path"               },
	        {"map_albedo",             "drawcall.texture.albedo.path"               },
	        {"emissionmap",            "drawcall.texture.emission.path"             },
	        {"map_emission",           "drawcall.texture.emission.path"             },
	        {"normalmap",              "drawcall.texture.normal.path"               },
	        {"map_normal",             "drawcall.texture.normal.path"               },
	        {"roughnessmap",           "drawcall.texture.roughness.path"            },
	        {"map_roughness",          "drawcall.texture.roughness.path"            },
	        {"metallicmap",            "drawcall.texture.metallic.path"             },
	        {"map_metallic",           "drawcall.texture.metallic.path"             },
	        {"aomap",                  "drawcall.texture.ao.path"                   },
	        {"map_ao",                 "drawcall.texture.ao.path"                   },
	        {"heitghmap",              "drawcall.texture.height.path"               },
	        {"map_height",             "drawcall.texture.height.path"               },
	        {"alpha",                  "drawcall.ub_material.albedo"                },
	};

	const std::vector<std::string> c_vertlist = {"v", "vt", "vn"};

	const std::vector<std::string> c_f4list = {
	        "drawcall.ub_material.albedo",
	};
	const std::vector<std::string> c_f3list = {
	        "Ka", "Kd", "Ks", "Ke", "Tf", "drawcall.ub_material.emission", "drawcall.ub_material.albedo",

	};
	const std::vector<std::string> c_f2list = {
	        "texcoord_scale",
	};
	const std::vector<std::string> c_f1list = {
	        "Ns",
	        "Ni",
	        "d",
	        "Tr",
	        "illum",

	        "drawcall.ub_material.roughness",
	        "drawcall.ub_material.metallic",
	        "drawcall.ub_material.ao",
	        "drawcall.ub_material.gamma",
	        "drawcall.ub_material.refractvie_index",
	        "drawcall.ub_material.absorption_coefficient",
	        "drawcall.ub_material.min_alpha",
	        "drawcall.ub_material.point_size",
	        "drawcall.ub_material.height_scale",

	        "drawcall.texture.clamp",
	        "drawcall.texture.albedo.alpha",
	        "drawcall.texture.albedo.srgb",
	        "drawcall.texture.albedo.gray_scale",
	        "drawcall.texture.height.gray_scale",
	        "drawcall.texture.height.height_scale",
	        //"alpha",
	};

	const std::vector<std::string> c_i1list = {
	        "remesh.grid",
	        "remesh.resample_normal",
	        "align_to_xz_plane",
	        "normalize_scale",
	        "all_flat",
	        "flip_face",
	        "flip_normal",
	        "singulate",
	        "all_flat",
	        "gen_texcoord",
	        "gen_normal",

	        "drawcall.ub_material.invisible",

	        "drawcall.texture.smooth_edge",
	        "drawcall.texture.max_level",
	        "drawcall.texture.limit_size",

	        "drawcall.flags.fill",
	        "drawcall.flags.blend",
	        "drawcall.flags.cull_face",
	        "drawcall.flags.ccw",
	};

	const std::vector<std::string> c_skiplist = {"s", "o"};

	struct Current {
		std::filesystem::path file;
		std::string line;
		Current(const std::filesystem::path &file) : file(file) {}
	};

	struct Group {
		std::string name;
		std::string usemtl;
		std::vector<std::pair<uint32_t, uint32_t>> ranges;

		Group(const std::string &name, int32_t first) : name(name), ranges(1)
		{
			ranges[0].first = first;
			ranges[0].second = 0;
		}
	};
	std::vector<Mesh> &m_meshes;  // reference

	std::array<char, 4096> m_linebuf;
	std::vector<Mesh::Face> m_sourpFaces;
	std::vector<Mesh::Vertex> m_soupVertices;

	std::stack<Current> m_current;
	std::vector<Group> m_groups;
	std::vector<std::vector<Metaindex>> m_objFaces;
	std::map<const std::string, Attrs> m_mtllibs;
	std::map<const std::string, std::vector<Vec3f>> m_objVertices;
	std::vector<std::filesystem::path> m_paths;
	std::unordered_set<std::filesystem::path> m_readMtllibs;

	Metavert *m_metavert = nullptr;  // pimpl
	Attrs m_attrs;                   // 1:1 to group

	void readMtllib();
	void readVertex();
	std::vector<const char *> fastExtract(const std::string &line);
};
}  // namespace spu::gs_node::wavefront
