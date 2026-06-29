//
//$<<Header>>$
//

#pragma once

#include <ssys/ssys.h>
#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class ObjMesh : public Shape {
public:  // MSVC
	struct LoadingOptions {
		bool load_normals;
		bool load_tangents;
		bool load_bitangents;
		bool load_texcoords;
		bool load_materials;

		LoadingOptions(bool load_all = true) { all(load_all); }

		LoadingOptions &all(bool load_all = true)
		{
			load_normals = load_all;
			load_tangents = load_all;
			load_bitangents = load_all;
			load_texcoords = load_all;
			load_materials = load_all;
			return *this;
		}

		LoadingOptions &nothing() { return all(false); }

		LoadingOptions &normals(bool load = true)
		{
			load_normals = load;
			return *this;
		}

		LoadingOptions &Tangents(bool load = true)
		{
			load_tangents = load;
			return *this;
		}

		LoadingOptions &bitangents(bool load = true)
		{
			load_bitangents = load;
			return *this;
		}

		LoadingOptions &texCoords(bool load = true)
		{
			load_texcoords = load;
			return *this;
		}

		LoadingOptions &materials(bool load = true)
		{
			load_materials = load;
			return *this;
		}
	};

	std::vector<double> m_pos_data;
	std::vector<double> m_nml_data;
	std::vector<double> m_tgt_data;
	std::vector<double> m_btg_data;
	std::vector<double> m_tex_data;
	std::vector<uint32_t> m_mtl_data;
	std::vector<std::string> m_mtl_names;

	struct VertIndices {
		uint32_t pos{0};
		uint32_t nml{0};
		uint32_t tex{0};
		uint32_t mtl{0};

		VertIndices()

		        = default;
	};

	std::vector<std::string> m_mesh_names;
	std::vector<uint32_t> m_mesh_offsets;
	std::vector<uint32_t> m_mesh_counts;

	bool load_index(
	        uint32_t &value, uint32_t n_verts, std::string::const_iterator &i,
	        std::string::const_iterator &e);

	bool load_indices(
	        VertIndices &indices, const VertIndices &counts, std::string::const_iterator &i,
	        std::string::const_iterator &e);

	template<class iter_t>
	void load_meshes(
	        // const LoadingOptions &opts, iter_t names_begin, iter_t names_end, std::istream &input);
	        const LoadingOptions &opts, iter_t names_begin, iter_t names_end, File &input);

	template<class iter_t>
	void call_load_meshes(
	        // std::istream &input, iter_t names_begin, iter_t names_end, LoadingOptions opts);
	        File &input, iter_t names_begin, iter_t names_end, LoadingOptions opts);

	// ObjMesh(std::istream &input, LoadingOptions opts = LoadingOptions())
	ObjMesh(File &input, LoadingOptions opts = LoadingOptions())
	{
		const char **p = nullptr;
		call_load_meshes(input, p, p, opts);
	}

	template<class list_t>
	void ObjeMesh(
	        // std::istream &input, const list_t &names,
	        File &input, const list_t &names, LoadingOptions opts = LoadingOptions())
	{
		call_load_meshes(input, names.begin(), names.end(), opts);
	}

	uint32_t faceWinding() const { return GL_CCW; }

	using VertexAttribFunc = uint32_t (ObjMesh::*)(std::vector<float> &) const;

	uint32_t positions(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.begin(), m_pos_data.begin(), m_pos_data.end());
		return 3;
	}

	uint32_t normals(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.begin(), m_nml_data.begin(), m_nml_data.end());
		return 3;
	}

	uint32_t tangents(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.begin(), m_tgt_data.begin(), m_tgt_data.end());
		return 3;
	}

	uint32_t bitangents(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.begin(), m_btg_data.begin(), m_btg_data.end());
		return 3;
	}

	uint32_t texCoordinates(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.begin(), m_tex_data.begin(), m_tex_data.end());
		return 3;
	}

	template<typename T> uint32_t materialNumbers(std::vector<T> &dest) const
	{
		dest.clear();
		dest.insert(dest.begin(), m_mtl_data.begin(), m_mtl_data.end());
		return 1;
	}

	const std::string &materialName(uint32_t mat_num) const { return m_mtl_names[mat_num]; }

	bool queryMeshIndex(const std::string &name, uint32_t &index) const;
	uint32_t getMeshIndex(const std::string &name) const;
	Sphere3f makeBoundingSphere() const;

	void boundingSphere(Sphere3f &bounding_sphere) const
	{
		bounding_sphere = Sphere3f(makeBoundingSphere());
	}

	void boundingSphere(Vec4f &center_and_radius) const
	{
		auto s = Sphere3f(makeBoundingSphere());
		center_and_radius = Vec4f(s.center.x, s.center.y, s.center.z, s.radius);
	}

	using IndexArray = std::vector<uint32_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const { return IndexArray(); }

	uint32_t restartIndex() const { return 0xfffffffe; }  // not -1 (represents no restart)

	std::vector<spu::SpuCommand> instructions(uint32_t primitive) const;

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const
	{
		return instructions(GL_TRIANGLES);
	}
};

}  // namespace spu::oglplus::shapes
#include <shapes/obj_mesh.ipp>
