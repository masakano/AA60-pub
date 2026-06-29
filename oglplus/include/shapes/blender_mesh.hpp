//
//$<<Header>>$
//

#pragma once

#include <blender/blender.hpp>
#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class BlenderMesh : public Shape {
public:
	struct LoadingOptions {
		const char *scene_name{nullptr};
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

		LoadingOptions &tangents(bool load = true)
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

private:
	std::vector<float> m_pos_data;
	std::vector<float> m_nml_data;
	std::vector<float> m_tgt_data;
	std::vector<float> m_btg_data;
	std::vector<float> m_uvc_data;
	std::vector<GLshort> m_mtl_data;
	std::vector<uint32_t> m_idx_data;
	std::vector<uint32_t> m_mesh_offsets;
	std::vector<uint32_t> m_mesh_n_elems;

	imports::BlendFileFlatStructBlockData find_scene(
	        const LoadingOptions & /*opts*/, imports::BlendFile &blend_file,
	        imports::BlendFileStructGlobBlock &glob_block)
	{
		return blend_file[glob_block.m_curscene];
	}

	void load_mesh(
	        const LoadingOptions &opts, imports::BlendFile &blend_file,
	        imports::BlendFileFlatStructBlockData &object_mesh_data, const Mat4f &mesh_matrix,
	        uint32_t &index_offset);

	template<class iter_t>
	void load_object(
	        const LoadingOptions &opts, iter_t names_begin, iter_t names_end,
	        imports::BlendFile &blend_file, imports::BlendFileFlatStructBlockData &object_data,
	        imports::BlendFilePointer object_data_ptr, uint32_t &index_offset);

	template<class iter_t>
	void load_meshes(
	        const LoadingOptions &opts, iter_t names_begin, iter_t names_end,
	        imports::BlendFile &blend_file);

	template<class iter_t>
	void call_load_meshes(
	        imports::BlendFile &blend_file, const char *scene_name, iter_t names_begin, iter_t names_end,
	        LoadingOptions opts);

public:
	BlenderMesh(imports::BlendFile &blend_file)
	{
		call_load_meshes(
		        blend_file, nullptr, static_cast<const char **>(nullptr),
		        static_cast<const char **>(nullptr), LoadingOptions());
	}
	template<class list_t>
	BlenderMesh(imports::BlendFile &blend_file, const list_t &names, LoadingOptions opts = LoadingOptions())
	{
		call_load_meshes(blend_file, nullptr, names.begin(), names.end(), opts);
	}

	uint32_t faceWinding() const { return GL_CCW; }

	using VertexAttribFunc = uint32_t (BlenderMesh::*)(std::vector<float> &) const;

	uint32_t positions(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.end(), m_pos_data.begin(), m_pos_data.end());
		return 3;
	}

	uint32_t normals(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.end(), m_nml_data.begin(), m_nml_data.end());
		return 3;
	}

	uint32_t tangents(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.end(), m_tgt_data.begin(), m_tgt_data.end());
		return 3;
	}

	uint32_t bitangents(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.end(), m_btg_data.begin(), m_btg_data.end());
		return 3;
	}

	uint32_t texCoordinates(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.end(), m_uvc_data.begin(), m_uvc_data.end());
		return 2;
	}

	uint32_t materialNumbers(std::vector<float> &dest) const
	{
		dest.clear();
		dest.insert(dest.end(), m_mtl_data.begin(), m_mtl_data.end());
		return 1;
	}

	Sphere3f getBoundingSphere() const;

	void boundingSphere(Sphere3f &bounding_sphere) const { bounding_sphere = getBoundingSphere(); }

	using IndexArray = std::vector<uint32_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const { return m_idx_data; }

	uint32_t restartIndex() const { return 0; }  // tricky

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const;
};

}  // namespace spu::oglplus::shapes
#include <shapes/blender_mesh.ipp>
