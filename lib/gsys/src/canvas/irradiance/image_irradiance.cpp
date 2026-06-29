//
// ImageIrradiance :
//
#include "image_irradiance.h"
#include "analizer.h"

#include <gsys/canvas/lightmap.h>
#include <gsys/canvas/gauss.h>
#include <gsys/canvas/copy.h>
#include <gsys/drawcall.h>
#include <ssys/serializer.h>
#include <gsys/shaders/canvas/irradiance/ub_integrate_irradiance.us>

// #ifndef GIT_REVISION
// #define GIT_REVISION "irradiance"
// #endif

namespace spu::gs_canvas {
namespace {

uint32_t texture_target(uint32_t texture_id) { return texture_id >> 16; }

uint32_t texture_width(uint32_t texture_id)
{
	uint32_t width;
	spu_texture_get(texture_id, "width", &width);
	return width;
}

uint32_t texture_height(uint32_t texture_id)
{
	uint32_t height;
	spu_texture_get(texture_id, "height", &height);
	return height;
}

const char *texture_signature(uint32_t texture_id)
{
	const char *signature = nullptr;
	spu_inventory_get(texture_id, "signature", &signature);
	return signature;
}
}  // namespace

void ImageIrradiance::init(const Attrs &attrs)
{
	auto lightmap_attrs = attrs.select("lightmap.");
	if (!lightmap_attrs.empty()) {
		m_lightmap.init(lightmap_attrs);
	}
}

void ImageIrradiance::update()
{
	auto target_lightmap = lightmap();  // copy
	if (target_lightmap.sync(true) || target_lightmap.id() == m_cachedLightmapId) {
		return;
	}
	auto lightmap_signature = texture_signature(target_lightmap.id());
	if (lightmap_signature == nullptr) {
		return;
	}

	Irradiance::update();

	m_cachedLightmapId = target_lightmap.id();
	auto lightmap_target = texture_target(target_lightmap.id());
	auto depth = m_powers.size();
	auto target = depth == 1 ? GL_TEXTURE_2D : GL_TEXTURE_2D_ARRAY;

	Attrs texture_attrs = {
	        {"target",      target            },
	        {"iformat",     c_irradmap_iformat},
	        {"width",       m_width           },
	        {"height",      m_width / 2       },
	        {"depth",       depth             },
	        {"wrap_s",      GL_REPEAT         },
	        {"wrap_t",      GL_CLAMP_TO_EDGE  },
	        {"min_filter",  GL_LINEAR         },
	        {"mag_filter",  GL_LINEAR         },
	        {"auto_mipmap", 0                 },
	        {"max_level",   0                 },
	};
	m_irradmap.init(texture_attrs);
	GsDrawcall::getDefault()->irradmap = m_irradmap;

	auto cache_path = makeCachePath(lightmap_signature);
	if (load(cache_path)) {
		return;
	}

	if (lightmap_target == GL_TEXTURE_CUBE_MAP) {
		auto cubemap_to_sphere_canvas = cubemapToSphere();
		correctExposure(cubemap_to_sphere_canvas->getBuffer("color0").id(), 0);
		delete cubemap_to_sphere_canvas;
	}
	else {
		correctExposure(target_lightmap.id(), 0);
	}

	GsCanvas *gauss_canvas = nullptr;  // keep life

	if (lightmap_target == GL_TEXTURE_2D && texture_width(target_lightmap.id()) > m_width) {
		gauss_canvas = gaussianFilter(8.0, 1.0);
		target_lightmap = gauss_canvas->getBuffer("color0");
	}

	const auto *c_path = "canvas/irradiance/integrate_irradiance.us";
	GsCanvas integrate_canvas;

	auto viewport = Rectf(0, 0, m_width, m_width / 2);
	Attrs canvas_attrs = {
	        {"path",              c_path         },
	        {"viewport0",         viewport       },
	        {"color0.texture_id", m_irradmap.id()},
	};
	integrate_canvas.init(canvas_attrs);

	irradiance::UB ub = {
	        .lightmap_target = lightmap_target,
	        .power = 1.0f,
	        .step = c_irradmap_sample_step,
	};

	auto lightmap_id = target_lightmap.id();
	Attrs unif_attrs = {
	        {"ub",                &ub         },
	        {"u_lightmap_sphere", &lightmap_id},
	        {"u_lightmap_cube",   &lightmap_id},
	};
	integrate_canvas.getShader().addUniforms(unif_attrs);

	for (auto i = 0u; i < m_powers.size(); i++) {
		ub.power = m_powers[i];
		integrate_canvas.set("layer", i);
		integrate_canvas.begin();
		integrate_canvas.render();
		integrate_canvas.end();
	}
	integrate_canvas.getBuffer("color0").save("irrad%d.bmp", "color");
	save(cache_path);
	delete gauss_canvas;
}

void ImageIrradiance::set(const Attrs &attrs)
{
	auto powers = attrs.getf<float *>("powers");
	if (powers.hit) {
		m_powers.clear();
		for (auto i = 0; i < 16; i++) {
			if (powers.value[i] == 0.0f) break;
			m_powers.push_back(powers.value[i]);
		}
		aux_error(m_powers.size() >= 16, "too many powers. (forget terminating by zero?)\n");
	}
	auto width = attrs.getf<uint32_t>("width");
	if (width.hit) {
		m_width = width.value;
	}
	Irradiance::set(attrs);
}

GsCanvas *ImageIrradiance::cubemapToSphere()
{
	auto &target_lightmap = lightmap();
	auto width = texture_width(target_lightmap.id());
	assert(width == texture_height(target_lightmap.id()));

	auto viewport0 = Rectf(0, 0, width * 2, width);
	Attrs canvas_attrs = {
	        {"viewport0",                viewport0 },
	        {"color0.iformat",           GL_RGBA32F},
	        {"def_cubemap_to_spheremap", 1         },
	};
	auto *canvas = new gs_canvas::Copy(canvas_attrs);
	canvas->u_color0 = target_lightmap.id();
	canvas->begin();
	canvas->render();
	canvas->end();
	return canvas;
}

GsCanvas *ImageIrradiance::gaussianFilter(float variance, float footstep)
{
	auto &target_lightmap = lightmap();
	auto viewport = Rectf(0, 0, m_width, m_width / 2);

	Attrs canvas_attrs = {
	        {"viewport0",          viewport          },
                {"color0.iformat",     c_irradmap_iformat},
	        {"color0.wrap_s",      GL_REPEAT         },
                {"color0.wrap_t",      GL_CLAMP_TO_EDGE  },
	        {"color0.min_filter",  GL_LINEAR         },
                {"color0.mag_filter",  GL_LINEAR         },
	        {"color0.auto_mipmap", 0                 },
                {"color0.max_level",   0                 },
	};
	auto canvas = new gs_canvas::Gauss2D(canvas_attrs);

	canvas->u_color0 = target_lightmap.id();
	canvas->u_variance = variance;
	canvas->u_footstep = {footstep, 0, 0, footstep};
	canvas->begin();
	canvas->render();
	canvas->end();
	return canvas;
}

void ImageIrradiance::correctExposure(uint32_t lightmap, int32_t layer)
{
	auto width = texture_width(lightmap);
	auto height = texture_height(lightmap);
	auto target = texture_target(lightmap);

	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_2D_ARRAY) {
		aux_message(0, "unsupported target '%s'. (skip correction)", opengl_const(target));
		return;
	}

	int32_t loc[4] = {0, 0, layer, 0};
	uint32_t size[4] = {0, 0, 1, 0};

	std::vector<Vec3f> pixels(width * height);
	spu_texture_recv(lightmap, pixels.data(), GL_RGBA32F, loc, size);

	irradiance::Analizer analizer(pixels, width, height);

	ub_light.ambient = analizer.ambient();
	ub_light.sources[0].emission = analizer.emission();
	ub_light.sources[0].position = analizer.peakdir() * c_lightmap_distance;
	ub_light.sources[0].direction = analizer.peakdir();
	ub_light.sources[0].type = e_ub_light_parallel;
}

std::string ImageIrradiance::makeCachePath(const char *signature) const
{
	auto sigstr = std::string(signature);
	auto src_path = File::searchPath(sigstr.substr(0, sigstr.find(':')), false);
	aux_error(src_path.empty(), "invalid texture signature '%s'\n", signature);
	return make_cache_path(src_path.string() + ".irrad");
}

bool ImageIrradiance::load(const std::filesystem::path &path)
{
	if (!std::filesystem::exists(path)) {
		aux_message(0, "%s: not found (rebuild)\n", path.string().c_str());
		return false;
	}
	auto heap = read_from_file<std::vector<uint8_t>>(path);
	if (selfDeserialize(heap.data()) == 0) {
		aux_message(0, "%s: invalid (rebuild)\n", path.string().c_str());
		return false;
	}
	return true;
}

void ImageIrradiance::save(const std::filesystem::path &path) const
{
	aux_message(0, "save: %s\n", path.string().c_str());

	std::vector<uint8_t> heap;
	auto size = selfSerialize(nullptr, true);
	heap.resize(size);
	selfSerialize(heap.data(), false);

	File file(path, "wb");
	file.write(heap.data(), heap.size());
}

size_t ImageIrradiance::selfSerialize(uint8_t *heap, bool is_dry) const
{
	std::string header = GIT_REVISION;
	std::vector<Vec4f> pixels(m_width * m_width / 2 * m_powers.size());
	m_irradmap.recv(pixels.data(), GL_RGBA32F);

	auto *hp = heap;
	hp += serialize(hp, is_dry, header);
	hp += serialize(hp, is_dry, m_width);
	hp += serialize(hp, is_dry, ub_light);
	hp += serialize(hp, is_dry, m_powers);
	hp += serialize(hp, is_dry, pixels);
	return hp - heap;
}

size_t ImageIrradiance::selfDeserialize(uint8_t *heap)
{
	std::string header;
	std::vector<Vec4f> pixels;

	uint8_t *hp = heap;
	hp += deserialize(hp, header);
	if (header != GIT_REVISION) {
		return 0;
	}
	hp += deserialize(hp, m_width);
	hp += deserialize(hp, ub_light);
	hp += deserialize(hp, m_powers);
	hp += deserialize(hp, pixels);

	m_irradmap.send(pixels.data(), GL_RGBA32F);

	return hp - heap;
}

void ImageLightmap::init(const Attrs &attrs)
{
	m_irradiance = new ImageIrradiance(attrs.select("irradiance."));
	Lightmap::init(attrs);
}
bool ImageLightmap::doSync(bool is_nonblock)
{
	auto &lightmap = const_cast<spu::SpuTexture &>(irradiance()->lightmap());
	return lightmap.sync(is_nonblock);
}
}  // namespace spu::gs_canvas
