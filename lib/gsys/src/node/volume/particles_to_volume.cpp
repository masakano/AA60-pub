//
// ParticlesToVolume :
//
#include "particles_to_volume.h"

namespace spu::gs_node::volume {

ParticlesToVolume::~ParticlesToVolume()
{
	if (u_uint_density_texture) {
		spu_texture_delete(u_uint_density_texture);
	}
}

ParticlesToVolume::ParticlesToVolume(uint32_t density_texture)
{
	u_density_texture = density_texture;

	uint32_t width, height, depth;
	spu_texture_get(u_density_texture, "width", &width);
	spu_texture_get(u_density_texture, "height", &height);
	spu_texture_get(u_density_texture, "depth", &depth);

	// u_uint_density_texture
	{
		Attrs texture_attrs = {
		        {"target",      GL_TEXTURE_3D},
                        {"iformat",     GL_R32UI     },
                        {"width",       width        },
		        {"height",      height       },
                        {"depth",       depth        },
                        {"max_level",   0            },
		        {"auto_mipmap", 0            },
		};
		u_uint_density_texture = spu_texture_new(texture_attrs);
	}

	// fill array
	{
		auto &shader = m_fillArray.getShader();
		const auto *shader_path = "compute/volume/particles_to_volume.us";

		Attrs shader_attrs = {
		        {"def_local_size", def_local_size},
		        {"def_uint_scale", 8192          },
		};
		shader.init(shader_path, shader_attrs);

		Attrs unif_attrs = {
		        {"u_uint_density_texture", &u_uint_density_texture},
		        {"u_particle_count",       &u_particle_count      },
		        //{"u_worldvolume",          &u_worldvolume         },
		        {"u_radius",               &u_radius              },
		};
		shader.addUniforms(unif_attrs);

		auto access = GL_READ_WRITE;
		auto layered = GL_TRUE;

		Attrs shader_set_attrs = {
		        {"u_uint_density_texture.access",  &access },
		        {"u_uint_density_texture.layered", &layered},
		};
		shader.set(shader_set_attrs);

		Attrs array_attrs = {
		        {"shader_id",    shader.id()},
		        {"a.a_position", 4          },
		};
		m_fillArray.init(array_attrs);
	}

	// copy array
	{
		auto &shader = m_copyArray.getShader();
		const auto *shader_path = "compute/volume/uint_volume_to_float_volume.us";

		Attrs shader_attrs = {
		        {"def_local_size", def_copy_local_size},
		        {"def_uint_scale", 8192               },
		};
		shader.init(shader_path, shader_attrs);

		Attrs unif_attrs = {
		        {"u_uint_density_texture", &u_uint_density_texture},
		        {"u_density_texture",      &u_density_texture     },
		};
		shader.addUniforms(unif_attrs);

		auto read_write_access = GL_READ_WRITE;
		auto write_access = GL_WRITE_ONLY;
		auto layered = GL_TRUE;

		Attrs shader_set_attrs = {
		        {"u_uint_density_texture.access",  &read_write_access},
		        {"u_uint_density_texture.layered", &layered          },
		        {"u_density_texture.access",       &write_access     },
		        {"u_density_texture.layered",      &layered          },
		};
		shader.set(shader_set_attrs);

		m_copyArray.getDim().x = (width + def_copy_local_size - 1) / def_copy_local_size;
		m_copyArray.getDim().y = (height + def_copy_local_size - 1) / def_copy_local_size;
		m_copyArray.getDim().z = (depth + def_copy_local_size - 1) / def_copy_local_size;
	}
}

void ParticlesToVolume::draw(const std::vector<Vec3f> &particles)
{
	const uint32_t izero = 0;
	u_particle_count = particles.size();
	spu_texture_send(u_uint_density_texture, &izero, GL_R32UI, nullptr, nullptr, true);
	m_fillArray.send(particles.data(), particles.size());
	m_fillArray.getDim().x = (u_particle_count + def_local_size - 1) / def_local_size;
	m_fillArray.compute();
	spu_graphics_memory_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_copyArray.compute();
}

}  // namespace spu::gs_node::volume
