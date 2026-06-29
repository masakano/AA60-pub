#include <utility>

// #include <stdlib.h>  // free
using namespace std;
namespace spu::oglplus::shapes {

inline void BlenderMesh::load_mesh(
        const LoadingOptions &opts, imports::BlendFile &blend_file,
        imports::BlendFileFlatStructBlockData &object_mesh_data, const Mat4f &mesh_matrix,
        uint32_t &index_offset)
{
	auto n_verts = 0;
	auto n_add_verts = 0;

	imports::BlendFilePointer vertex_ptr = object_mesh_data.Field<void *>("mvert").get();
	if (vertex_ptr) {
		auto vertex_data = blend_file[vertex_ptr];
		n_verts = vertex_data.blockElementCount();

		auto vertex_co_field = vertex_data.Field<float>("co");
		auto vertex_no_field = vertex_data.Field<int16_t>("no");
		std::vector<float> ps(3 * n_verts);
		std::vector<float> ns(opts.load_normals ? 3 * n_verts : 0);
		std::vector<float> gs(opts.load_tangents ? 3 * n_verts : 0);
		std::vector<float> ts(opts.load_texcoords ? 2 * n_verts : 0, -1.0F);
		std::vector<GLshort> ms(opts.load_materials ? 1 * n_verts : 0, -1);
		for (auto v = 0; v != n_verts; ++v) {
			auto position
			        = Vec4f(vertex_co_field.get(v, 0), vertex_co_field.get(v, 1),
			                vertex_co_field.get(v, 2), 1.0F);

			auto newpos = mesh_matrix * position;
			ps[3 * v + 0] = newpos.x;
			ps[3 * v + 1] = newpos.z;
			ps[3 * v + 2] = -newpos.y;

			if (opts.load_normals) {
				auto normal = normalize(
				        Vec3f(vertex_no_field.get(v, 0), vertex_no_field.get(v, 1),
				              vertex_no_field.get(v, 2)));

				auto newnorm = mesh_matrix * normal;
				ns[3 * v + 0] = newnorm.x;
				ns[3 * v + 1] = newnorm.z;
				ns[3 * v + 2] = -newnorm.y;
			}
		}
		m_pos_data.insert(m_pos_data.end(), ps.begin(), ps.end());
		if (opts.load_normals) {
			m_nml_data.insert(m_nml_data.end(), ns.begin(), ns.end());
		}
		if (opts.load_tangents) {
			m_tgt_data.insert(m_tgt_data.end(), gs.begin(), gs.end());
		}
		if (opts.load_bitangents) {
			m_btg_data.insert(m_btg_data.end(), gs.begin(), gs.end());
		}
		if (opts.load_texcoords) {
			m_uvc_data.insert(m_uvc_data.end(), ts.begin(), ts.end());
		}
		if (opts.load_materials) {
			m_mtl_data.insert(m_mtl_data.end(), ms.begin(), ms.end());
		}
	}

	std::vector<float> aps;
	std::vector<float> ans;
	std::vector<float> ags;
	std::vector<float> abs;
	std::vector<float> ats;
	std::vector<GLshort> ams;
	std::vector<uint32_t> ais;

	auto face_ptr = object_mesh_data.Field<void *>("mface").get();
	auto tface_ptr = object_mesh_data.Field<void *>("mtface").get();
	if (opts.load_texcoords && face_ptr && !tface_ptr) {
		throw std::runtime_error("Unable to load UV coordinates.");
	}
	if (opts.load_tangents && face_ptr && !tface_ptr) {
		throw std::runtime_error("Unable to load tangent vectors.");
	}
	if ((opts.load_texcoords && face_ptr && tface_ptr) || (opts.load_tangents && face_ptr && tface_ptr)
	    || (opts.load_materials && face_ptr)) {
		auto face_data = blend_file[face_ptr];
		auto tface_data = blend_file[tface_ptr];
		auto n_faces = face_data.blockElementCount();

		if (opts.load_texcoords || opts.load_tangents) {
			assert(n_faces == tface_data.blockElementCount());
		}
		auto face_v1_field = face_data.Field<int>("v1");
		auto face_v2_field = face_data.Field<int>("v2");
		auto face_v3_field = face_data.Field<int>("v3");
		auto face_v4_field = face_data.Field<int>("v4");
		auto face_mat_nr_field = face_data.Field<int16_t>("mat_nr");

		std::vector<uint32_t> is(5 * n_faces);
		std::vector<bool> needs_vertex_copy(n_faces, false);

		auto ii = 0;
		for (uint32_t f = 0; f != n_faces; ++f) {
			int32_t fv[4]
			        = {face_v1_field.get(f, 0), face_v2_field.get(f, 0), face_v3_field.get(f, 0),
			           face_v4_field.get(f, 0)};

			float uv[8];
			if (opts.load_texcoords) {
				auto tface_uv_field = tface_data.Field<float>("uv");
				for (uint32_t i = 0; i != 8; ++i) {
					uv[i] = tface_uv_field.get(f, i);
				}
			}

			auto mat_nr = face_mat_nr_field.get(f, 0);

			uint32_t fi[4]
			        = {fv[0] + index_offset, fv[1] + index_offset, fv[2] + index_offset,
			           fv[3] + index_offset};

			auto f_verts = fv[3] != 0 ? 4 : 3;

			auto needs_vert_copy = false;
			for (auto i = 0; i != f_verts; ++i) {
				if (opts.load_texcoords) {
					for (auto j = 0; j != 2; ++j) {
						needs_vert_copy |= (m_uvc_data[fi[i] * 2 + j] >= 0.0F)
						                && (m_uvc_data[fi[i] * 2 + j] != uv[i * 2 + j]);
					}
				}
				if (opts.load_materials) {
					needs_vert_copy
					        |= (m_mtl_data[fi[i]] >= 0) && (m_mtl_data[fi[i]] != mat_nr);
				}
			}

			if (needs_vert_copy) {
				needs_vertex_copy[f] = true;
			}
			else {
				for (auto i = 0; i != f_verts; ++i) {
					if (opts.load_texcoords) {
						m_uvc_data[fi[i] * 2 + 0] = uv[i * 2 + 0];
						m_uvc_data[fi[i] * 2 + 1] = uv[i * 2 + 1];
					}
					if (opts.load_materials) {
						m_mtl_data[fi[i]] = mat_nr;
					}
					is[ii++] = fi[i];
				}
				if (opts.load_tangents || opts.load_bitangents) {
					for (auto i = 0; i != f_verts; ++i) {
						int32_t j[3] = {i, (i + 1) % f_verts, (i + 2) % f_verts};

						Vec3f p[3];
						Vec2f uvvec[3];
						for (auto k = 0; k != 3; ++k) {
							p[k]
							        = Vec3f(m_pos_data[fi[j[k]] * 3 + 0],
							                m_pos_data[fi[j[k]] * 3 + 1],
							                m_pos_data[fi[j[k]] * 3 + 2]);
							uvvec[k]
							        = Vec2f(m_uvc_data[fi[j[k]] * 2 + 0],
							                m_uvc_data[fi[j[k]] * 2 + 1]);
						}

						auto v0 = p[1] - p[0];
						auto v1 = p[2] - p[0];

						auto duv0 = uvvec[1] - uvvec[0];
						auto duv1 = uvvec[2] - uvvec[0];

						auto d = duv0.x * duv1.y - duv0.y * duv1.x;
						if (d != 0.0F) {
							d = 1.0F / d;
						}

						auto t = (duv1.y * v0 - duv0.y * v1) * d;
						auto nt = normalize(t);
						m_tgt_data[fi[i] * 3 + 0] = nt.x;
						m_tgt_data[fi[i] * 3 + 1] = nt.y;
						m_tgt_data[fi[i] * 3 + 2] = nt.z;

						auto b = (duv0.x * v1 - duv1.x * v0) * d;
						auto nb = normalize(b);
						m_btg_data[fi[i] * 3 + 0] = nb.x;
						m_btg_data[fi[i] * 3 + 1] = nb.y;
						m_btg_data[fi[i] * 3 + 2] = nb.z;
					}
				}
				is[ii++] = 0;
			}
		}
		is.resize(ii);
		m_idx_data.insert(m_idx_data.end(), is.begin(), is.end());

		for (auto f = 0u; f != n_faces; ++f) {
			int32_t fv[4]
			        = {face_v1_field.get(f, 0), face_v2_field.get(f, 0), face_v3_field.get(f, 0),
			           face_v4_field.get(f, 0)};

			float uv[8];
			if (opts.load_texcoords) {
				auto tface_uv_field = tface_data.Field<float>("uv");
				for (auto i = 0; i != 8; ++i) {
					uv[i] = tface_uv_field.get(f, i);
				}
			}

			auto mat_nr = face_mat_nr_field.get(f, 0);

			uint32_t fi[4]
			        = {fv[0] + index_offset, fv[1] + index_offset, fv[2] + index_offset,
			           fv[3] + index_offset};
			auto f_verts = fv[3] != 0 ? 4 : 3;

			if (needs_vertex_copy[f]) {
				for (auto i = 0; i != f_verts; ++i) {
					aps.push_back(m_pos_data[fi[i] * 3 + 0]);
					aps.push_back(m_pos_data[fi[i] * 3 + 1]);
					aps.push_back(m_pos_data[fi[i] * 3 + 2]);

					if (opts.load_normals) {
						ans.push_back(m_nml_data[fi[i] * 3 + 0]);
						ans.push_back(m_nml_data[fi[i] * 3 + 1]);
						ans.push_back(m_nml_data[fi[i] * 3 + 2]);
					}

					if (opts.load_tangents) {
						ags.push_back(m_tgt_data[fi[i] * 3 + 0]);
						ags.push_back(m_tgt_data[fi[i] * 3 + 1]);
						ags.push_back(m_tgt_data[fi[i] * 3 + 2]);
					}

					if (opts.load_bitangents) {
						abs.push_back(m_btg_data[fi[i] * 3 + 0]);
						abs.push_back(m_btg_data[fi[i] * 3 + 1]);
						abs.push_back(m_btg_data[fi[i] * 3 + 2]);
					}

					if (opts.load_texcoords) {
						ats.push_back(uv[i * 2 + 0]);
						ats.push_back(uv[i * 2 + 1]);
					}

					if (opts.load_materials) {
						ams.push_back(mat_nr);
					}

					ais.push_back(index_offset + n_verts + n_add_verts);
					++n_add_verts;
				}
				ais.push_back(0);
			}
		}
	}
	else if (face_ptr) {
		auto face_data = blend_file[face_ptr];
		auto n_faces = int32_t(face_data.blockElementCount());
		auto face_v1_field = face_data.Field<int>("v1");
		auto face_v2_field = face_data.Field<int>("v2");
		auto face_v3_field = face_data.Field<int>("v3");
		auto face_v4_field = face_data.Field<int>("v4");
		std::vector<uint32_t> is(5 * n_faces);
		auto ii = 0;
		for (auto f = 0; f != n_faces; ++f) {
			auto v1 = face_v1_field.get(f, 0);
			auto v2 = face_v2_field.get(f, 0);
			auto v3 = face_v3_field.get(f, 0);
			auto v4 = face_v4_field.get(f, 0);

			is[ii++] = v1 + index_offset;
			is[ii++] = v2 + index_offset;
			is[ii++] = v3 + index_offset;
			if (v4 != 0) {
				is[ii++] = v4 + index_offset;
			}
			is[ii++] = 0;  // primitive restart index
		}
		is.resize(ii);
		m_idx_data.insert(m_idx_data.end(), is.begin(), is.end());
	}

	auto poly_ptr = object_mesh_data.tryGet<void *>("mpoly", nullptr);
	auto loop_ptr = object_mesh_data.tryGet<void *>("mloop", nullptr);
	if (poly_ptr && loop_ptr) {
		auto poly_data = blend_file[poly_ptr];
		auto loop_data = blend_file[loop_ptr];
		auto n_polys = int32_t(poly_data.blockElementCount());
		auto poly_loopstart_field = poly_data.Field<int>("loopstart");
		auto poly_totloop_field = poly_data.Field<int>("totloop");
		auto loop_v_field = loop_data.Field<int>("v");

		std::vector<uint32_t> is;
		for (auto f = 0; f != n_polys; ++f) {
			auto ls = poly_loopstart_field.get(f, 0);
			auto tl = poly_totloop_field.get(f, 0);

			for (auto l = 0; l != tl; ++l) {
				auto v = loop_v_field.get(ls + l, 0);
				is.push_back(v + index_offset);
			}
			is.push_back(0);
		}
		m_idx_data.insert(m_idx_data.end(), is.begin(), is.end());
	}

	if (n_add_verts != 0u) {
		m_pos_data.insert(m_pos_data.end(), aps.begin(), aps.end());
		m_nml_data.insert(m_nml_data.end(), ans.begin(), ans.end());
		m_tgt_data.insert(m_tgt_data.end(), ags.begin(), ags.end());
		m_btg_data.insert(m_btg_data.end(), abs.begin(), abs.end());
		m_uvc_data.insert(m_uvc_data.end(), ats.begin(), ats.end());
		m_mtl_data.insert(m_mtl_data.end(), ams.begin(), ams.end());
		m_idx_data.insert(m_idx_data.end(), ais.begin(), ais.end());
	}
	index_offset += n_verts + n_add_verts;
}

template<class iter_t>
inline void BlenderMesh::load_object(
        const LoadingOptions &opts, iter_t names_begin, iter_t names_end, imports::BlendFile &blend_file,
        imports::BlendFileFlatStructBlockData &object_data, imports::BlendFilePointer object_data_ptr,
        uint32_t &index_offset)
{
	imports::BlendFileFlatStructBlockData object_data_data = blend_file[object_data_ptr];
	if (object_data_data.structureName() == "Mesh") {
		auto object_obmat_field = object_data.Field<float>("obmat");
		auto object_name_field = object_data.Field<std::string>("id.name");

		assert(m_mesh_offsets.size() == m_mesh_n_elems.size());

		auto mesh_idx = 0;  // suzu
		if (names_begin == names_end) {
			mesh_idx = m_mesh_offsets.size();
		}
		else {
			auto mi = 0;
			auto ni = names_begin;
			while (ni != names_end) {
				std::string tmp("OB");
				tmp.append(*ni);

				char *name
				        = strdup(object_name_field.get().c_str());  // walkaround for clang-tidy
				if (tmp == name) {
					mesh_idx = mi;
					break;
				}
				free(name);
				++mi;
				++ni;
			}
			if (ni == names_end) {
				return;
			}
		}

		if (int32_t(m_mesh_offsets.size()) < mesh_idx + 1) {
			m_mesh_offsets.resize(mesh_idx + 1);
			m_mesh_n_elems.resize(mesh_idx + 1);
		}
		Mat4f obmat = Mat4f(Vec4f(object_obmat_field.get(0, 0), object_obmat_field.get(0, 4),
		                          object_obmat_field.get(0, 8), object_obmat_field.get(0, 12)),

		                    Vec4f(object_obmat_field.get(0, 1), object_obmat_field.get(0, 5),
		                          object_obmat_field.get(0, 9), object_obmat_field.get(0, 13)),

		                    Vec4f(object_obmat_field.get(0, 2), object_obmat_field.get(0, 6),
		                          object_obmat_field.get(0, 10), object_obmat_field.get(0, 14)),

		                    Vec4f(object_obmat_field.get(0, 3), object_obmat_field.get(0, 7),
		                          object_obmat_field.get(0, 11), object_obmat_field.get(0, 15)))
		                      .transpose4();

		m_mesh_offsets[mesh_idx] = m_idx_data.size();

		load_mesh(opts, blend_file, object_data_data, obmat, index_offset);

		m_mesh_n_elems[mesh_idx] = m_idx_data.size() - m_mesh_offsets[mesh_idx];
	}
}
template<class iter_t>
inline void BlenderMesh::load_meshes(
        const LoadingOptions &opts, iter_t names_begin, iter_t names_end, imports::BlendFile &blend_file)
{
	m_idx_data.push_back(0);
	m_pos_data.push_back(0.0);
	m_pos_data.push_back(0.0);
	m_pos_data.push_back(0.0);
	if (opts.load_normals) {
		m_nml_data.push_back(0.0);
		m_nml_data.push_back(0.0);
		m_nml_data.push_back(0.0);
	}
	if (opts.load_tangents) {
		m_tgt_data.push_back(0.0);
		m_tgt_data.push_back(0.0);
		m_tgt_data.push_back(0.0);
	}
	if (opts.load_bitangents) {
		m_btg_data.push_back(0.0);
		m_btg_data.push_back(0.0);
		m_btg_data.push_back(0.0);
	}
	if (opts.load_texcoords) {
		m_uvc_data.push_back(0.0);
		m_uvc_data.push_back(0.0);
	}
	if (opts.load_materials) {
		m_mtl_data.push_back(0);
	}
	uint32_t index_offset = 1;
	imports::BlendFileStructGlobBlock glob_block = blend_file.structuredGlobalBlock();
	imports::BlendFileFlatStructBlockData scene_data = find_scene(opts, blend_file, glob_block);
	imports::BlendFilePointer object_link_ptr = scene_data.Field<void *>("base.first").get();
	while (object_link_ptr) {
		imports::BlendFileFlatStructBlockData object_link_data = blend_file[object_link_ptr];
		imports::BlendFilePointer object_ptr = object_link_data.Field<void *>("object").get();
		if (object_ptr) {
			imports::BlendFileFlatStructBlockData object_data = blend_file[object_ptr];
			imports::BlendFilePointer object_data_ptr = object_data.Field<void *>("data").get();
			if (object_data_ptr) {
				load_object(
				        opts, names_begin, names_end, blend_file, object_data, object_data_ptr,
				        index_offset);
			}
		}
		object_link_ptr = object_link_data.Field<void *>("next").get();
	}
	assert(m_pos_data.size() % 3 == 0);
	if (opts.load_normals) {
		assert(m_pos_data.size() / 3 == m_nml_data.size() / 3);
	}
	if (opts.load_texcoords) {
		assert(m_pos_data.size() / 3 == m_uvc_data.size() / 2);
	}
	if (opts.load_materials) {
		assert(m_pos_data.size() / 3 == m_mtl_data.size() / 1);
	}
}

template<class iter_t>
inline void BlenderMesh::call_load_meshes(
        imports::BlendFile &blend_file, const char *scene_name, iter_t names_begin, iter_t names_end,
        LoadingOptions opts)
{
	opts.scene_name = scene_name;
	opts.load_tangents |= opts.load_bitangents;
	opts.load_bitangents |= opts.load_tangents;
	opts.load_texcoords |= opts.load_tangents;

	load_meshes(opts, names_begin, names_end, blend_file);
}

inline Sphere3f BlenderMesh::getBoundingSphere() const
{
	auto min_x = m_pos_data[3];
	auto max_x = m_pos_data[3];
	auto min_y = m_pos_data[4];
	auto max_y = m_pos_data[4];
	auto min_z = m_pos_data[5];
	auto max_z = m_pos_data[5];

	for (int32_t v = 1, vn = m_pos_data.size() / 3; v != vn; ++v) {
		auto x = m_pos_data[v * 3 + 0];
		auto y = m_pos_data[v * 3 + 1];
		auto z = m_pos_data[v * 3 + 2];

		if (min_x > x) {
			min_x = x;
		}
		if (min_y > y) {
			min_y = y;
		}
		if (min_z > z) {
			min_z = z;
		}
		if (max_x < x) {
			max_x = x;
		}
		if (max_y < y) {
			max_y = y;
		}
		if (max_z < z) {
			max_z = z;
		}
	}

	auto c = Vec3f(((min_x + max_x) * 0.5F), ((min_y + max_y) * 0.5F), ((min_z + max_z) * 0.5F));

	return Sphere3f(c, distance(c, Vec3f(min_x, min_y, min_z)));
}

inline std::vector<spu::SpuCommand> BlenderMesh::instructions(BlenderMesh::DefaultTag /*unused*/) const
{
	assert(m_mesh_offsets.size() == m_mesh_n_elems.size());

	std::vector<spu::SpuCommand> instructions;
	auto im = 0;
	auto nm = int32_t(m_mesh_offsets.size());

	while (im != nm) {
		spu::SpuCommand com;
		com.target = GL_ELEMENT_ARRAY_BUFFER;
		com.mode = GL_TRIANGLE_FAN;
		com.first = m_mesh_offsets[im];
		com.count = m_mesh_n_elems[im];
		com.flags = uint32_t(im);

		instructions.push_back(com);
		++im;
	}
	return instructions;
}

}  // namespace spu::oglplus::shapes
