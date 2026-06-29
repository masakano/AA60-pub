//
// GsObject :
//
#include <ssys/ssys.h>
#include "environ.h"
#include <gsys/drawcall.h>
#include <gsys/util/lambert_to_pbr.h>
#include <filesystem>

namespace spu {

void GsDrawcall::startup(const Attrs &attrs)
{
	assert(ms_default == nullptr);
	assert(ms_initial == nullptr);
	ms_initial = new GsDrawcall();
	ms_default = new GsDrawcall();

	resetInitial();
	auto initial_attrs = (GsObject::getAttrs() + attrs).select("drawcall.");

	ms_initial->set(initial_attrs);
	resetDefault();
}

void GsDrawcall::shutdown()
{
	delete ms_initial;
	delete ms_default;

	ms_initial = nullptr;
	ms_default = nullptr;
}

void GsDrawcall::resetInitial()
{
	assert(ms_initial);
	ms_initial->albedomap = GsObject::defaultWhiteTexture();
	ms_initial->specularmap = GsObject::defaultBlackTexture();
	ms_initial->emissionmap = GsObject::defaultWhiteTexture();
	ms_initial->armmap = GsObject::defaultWhiteTexture();
	ms_initial->normalmap = GsObject::defaultBlackTexture();
	ms_initial->heightmap = GsObject::defaultBlackTexture();
	ms_initial->brdfmap = GsObject::defaultBrdfTexture();
	ms_initial->lightmap = GsObject::defaultLightTexture();
	ms_initial->irradmap = GsObject::defaultLightTexture();

	auto &ub_material = ms_initial->ub_material;
	memset(&ub_material, 0, sizeof(ub_material));

	ub_material = {
	        .albedo = {1, 1, 1, 1},
	        .emission = {0, 0, 0, 0},
	        .roughness = 0.5,
	        .metallic = 0.5,
	        .ao = 0.5,
	        .gamma = 1.0,
	        .refractvie_index = 1.0,
	        .absorption_coefficient = 1.0,
	        .min_alpha = 0.1,
	        .point_size = 0.1,
	        .map_scale = 1.0,
	        .height_scale = 1.0,
	        .hash = 0,
	        .debug_mode = 0,
	        .invisible = 0,
	        .point_coord = 0,
	        ._i = {0, 0},
	};
}

void GsDrawcall::resetDefault()
{
	assert(ms_initial);
	assert(ms_default);
	*ms_default = *ms_initial;
}

GsDrawcall::GsDrawcall() : coms(1)
{
	if (ms_default && this != ms_default) {
		*this = *ms_default;
		lightmap = GsObject::defaultLightTexture();
		irradmap = GsObject::defaultLightTexture();
	}
}

bool operator==(const GsDrawcall &d0, const GsDrawcall &d1)
{
	return (SpuRenderstate(d0) == SpuRenderstate(d1)
	        && memcmp(&d0.ub_material, &d1.ub_material, sizeof(d0.ub_material)) == 0
	        && d0.albedomap.id() == d1.albedomap.id() && d0.specularmap.id() == d1.specularmap.id()
	        && d0.emissionmap.id() == d1.emissionmap.id() && d0.armmap.id() == d1.armmap.id()
	        && d0.normalmap.id() == d1.normalmap.id() && d0.heightmap.id() == d1.heightmap.id()
	        && d0.brdfmap.id() == d1.brdfmap.id() && d0.lightmap.id() == d1.lightmap.id()
	        && d0.irradmap.id() == d1.irradmap.id());
}

Attrs GsDrawcall::completeTexturePath(const char *preface, const char *ext) const
{
	std::map<const char *, std::vector<const char *>> bodies = {
	        {"albedo.path",    {"albedo", "Albedo", "basecolor"}    },
	        {"specular.path",  {"specular", "Specular"}             },
	        {"roughness.path", {"roughness", "Roughness"}           },
	        {"metallic.path",  {"metallic", "Metallic", "metalness"}},
	        {"normal.path",    {"normal", "normal-dx", "Normal-dx"} },
	        {"height.path",    {"height", "Height"}                 },
	        {"ao.path",        {"ao", "AO"}                         },
	};

	Attrs attrs;
	for (auto &pair: bodies) {
		for (auto &body: pair.second) {
			auto texture_path = File::searchPath(std::string(preface) + body + ext, false);
			if (std::filesystem::exists(texture_path)) {
				Attr attr = {pair.first, texture_path.c_str()};
				attr.preserve();
				attrs.push_back(attr);
			}
		}
	}
	return attrs;
}

void GsDrawcall::setMaterial(const Attrs &attrs)
{
	attrs.apply("emission", ub_material.emission);
	attrs.apply("albedo", ub_material.albedo);
	attrs.apply("roughness", ub_material.roughness);
	attrs.apply("metallic", ub_material.metallic);
	attrs.apply("ao", ub_material.ao);
	attrs.apply("gamma", ub_material.gamma);
	attrs.apply("refractvie_index", ub_material.refractvie_index);
	attrs.apply("min_alpha", ub_material.min_alpha);
	attrs.apply("point_size", ub_material.point_size);
	attrs.apply("map_scale", ub_material.map_scale);
	attrs.apply("height_scale", ub_material.height_scale);
	attrs.apply("invisible", ub_material.invisible);
	if (ub_material.albedo.a < 1.0) {
		flags.blend = true;
	}
}

void GsDrawcall::setTextures(const Attrs &attrs)
{
	attrs.peek("nz", "use 'gray_scale' instead");
	auto iformat = attrs.get("iformat", 0);
	auto flip_str = attrs.get("flip", "");
	auto limit_size = attrs.get("limit_size", 0);
	auto is_mipmap = attrs.get("mipmap", 1);
	auto restarget = attrs.get("restarget", "texture");
	auto complete_path = attrs.get("complete_path", "");

	Attrs global_attrs;
	Attrs local_attrs = attrs;

	if (*complete_path) {
		local_attrs.prepend(completeTexturePath(complete_path));  // debug
	}
	if (is_mipmap) {
		global_attrs = {
		        {"restarget",   restarget              },
		        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
		        {"auto_mipmap", 1                      },
		};
	}
	else {
		global_attrs = {
		        {"restarget",   restarget},
		        {"auto_mipmap", 0        },
		        {"max_level",   0        },
		};
	}
	if (*flip_str) {
		global_attrs.emplace_back("flip", flip_str);
	}
	if (iformat) {
		global_attrs.emplace_back("iformat", iformat);
	}
	if (limit_size) {
		global_attrs.emplace_back("limit_size", limit_size);
	}

	attrs.subpeek({"bump."}, "use 'height.' instead\n");

	local_attrs = local_attrs.uniq();

	const char *path = nullptr;
	auto albedo_attrs = global_attrs + local_attrs.select("albedo.");

	if (*(path = albedo_attrs.pick("path", ""))) {
		auto clamp = albedo_attrs.get("clamp", 0.0f);
		auto alpha = albedo_attrs.get("alpha", 0.0f);
		auto is_srgb = albedo_attrs.get("srgb", 1);  // default on
		auto is_smooth_edge = albedo_attrs.get("smooth_edge", 0);
		auto is_gray_scale = albedo_attrs.get("gray_scale", 0);

		// alpha = 1.0;
		Attrs aux_attrs = {
		        {"srgb",        is_srgb       },
		        {"alpha",       alpha         },
		        {"clamp",       clamp         },
		        {"smooth_edge", is_smooth_edge},
		        {"gray_scale",  is_gray_scale },
		};
		albedo_attrs.prepend(aux_attrs);
		albedomap.init(path, albedo_attrs);
	}

	auto specular_attrs = global_attrs + local_attrs.select("specular.");
	if (*(path = specular_attrs.pick("path", ""))) {
		specularmap.init(path, specular_attrs);
	}

	auto emission_attrs = global_attrs + local_attrs.select("emission.");
	if (*(path = emission_attrs.pick("path", ""))) {
		emissionmap.init(path, emission_attrs);
	}

	auto set_armmap = [&](int32_t channel, const char *selector) {
		auto chan_attrs = global_attrs + local_attrs.select(selector);
		if (*(path = chan_attrs.pick("path", ""))) {
			auto texture_id = GsObject::isDefaultTexture(armmap.id()) ? -1 : armmap.id();
			Attrs aux_attrs = {
			        {"iformat",    GL_RGBA8  },
			        {"texture_id", texture_id},
			        {"channel",    channel   },
			};
			chan_attrs.prepend(aux_attrs);
			armmap.init(path, chan_attrs);
		}
	};
	set_armmap(0, "ao.");         // A
	set_armmap(1, "roughness.");  // R
	set_armmap(2, "metallic.");   // M

	auto normal_attrs = global_attrs + local_attrs.select("normal.");
	if (*(path = normal_attrs.pick("path", ""))) {
		normalmap.init(path, normal_attrs);
	}

	auto height_attrs = global_attrs + local_attrs.select("height.");
	if (*(path = height_attrs.pick("path", ""))) {
		auto is_gray_scale = height_attrs.get("gray_scale", 0);
		Attrs aux_attrs = {
		        {"gray_scale", is_gray_scale},
		        {"iformat",    GL_R8        },
		};
		height_attrs.prepend(aux_attrs);
		heightmap.init(path, height_attrs);
	}

	auto light_attrs = attrs.select("light.");  // no global_attrs
	if (*(path = light_attrs.pick("path", ""))) {
		bool use_cross = (std::string(path).find("cross_") != std::string::npos)
		              || (std::string(path).find("_cross") != std::string::npos);

		if (use_cross) {
			Attrs aux_attrs = {
			        {"target",      GL_TEXTURE_CUBE_MAP    },
			        {"iformat",     GL_RGBA16F             },
			        {"clamp",       64.0                   }, // need parameterize
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
			        {"auto_mipmap", 1                      },
			};
			light_attrs.prepend(aux_attrs);
			lightmap.init(path, light_attrs);
		}
		else {
			Attrs aux_attrs = {
			        {"target",      GL_TEXTURE_2D          },
			        {"iformat",     GL_RGBA16F             },
			        {"clamp",       64.0                   }, // need parameterize
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
			        {"wrap_s",      GL_REPEAT              },
			        {"wrap_t",      GL_CLAMP_TO_EDGE       },
			        {"auto_mipmpa", 1                      },
			};
			light_attrs.prepend(aux_attrs);
			lightmap.init(path, light_attrs);
		}
	}
}

void GsDrawcall::set(const Attrs &attrs)
{
	setMaterial(attrs.select("ub_material."));
	setTextures(attrs.select("texture."));
	SpuRenderstate::set(attrs);
}

void GsDrawcall::sort(std::vector<GsDrawcall> &drawcalls)
{
	auto add_drawcall = [](std::vector<GsDrawcall> &drawcalls, const GsDrawcall &drawcall) {
		auto it = vector_find(drawcalls, drawcall);
		if (it != std::end(drawcalls)) {
			vector_cat(it->coms, drawcall.coms);
		}
		else {
			drawcalls.push_back(drawcall);
		}
	};

	std::vector<GsDrawcall> opaque_drawcalls;
	std::vector<GsDrawcall> translucent_drawcalls;

	for (auto &drawcall: drawcalls) {
		if (drawcall.flags.blend && drawcall.ub_material.albedo.a < 1.0) {
			add_drawcall(translucent_drawcalls, drawcall);
		}
		else {
			add_drawcall(opaque_drawcalls, drawcall);
		}
	}
	drawcalls = vector_cat(opaque_drawcalls, translucent_drawcalls);
}

void GsDrawcall::report(const char *str) const
{
	// title
	if (str && *str) {
		aux_printf("%s: %s\n", str, ub_material.invisible ? "(non visible)" : "");
	}

	// command
	{
		aux_printf("    commands:\n");
		aux_printf(
		        "\t%4s %8s %8s %6s %6s %6s %s\n", "mode", "first", "count", "n_inst", "b_vert",
		        "b_inst", "target");

		for (auto &com: coms) {
			aux_printf(
			        "\t%04x %8d %8d %6d %6d %6d %s\n", com.mode, com.first, com.count,
			        com.instance_count, com.base_vertex, com.base_instance,
			        com.target ? opengl_const(com.target) : "auto");
		}
		aux_printf("\n");
	}
	// material
	{
		aux_printf("    ub_material:\n");
		prt_f4("albedo", ub_material.albedo);
		prt_f4("emission", ub_material.emission);
		prt_f("roughness", ub_material.roughness);
		prt_f("metallic", ub_material.metallic);
		prt_f("ao", ub_material.ao);
		prt_f("gamma", ub_material.gamma);
		aux_printf("\n");
	}

	// textures
	{
		auto prt_tex = [&](const char *name, const SpuTexture &texture) {
			const char *signature = nullptr;
			texture.get("signature", &signature);
			if (signature) {
				auto list = extract_from_string(signature, ":");
				aux_printf("\t%-30s : %08x : '%s'\n", name, texture.id(), list[0].c_str());
			}
			else {
				aux_printf("\t%-30s : %08x : -\n", name, texture.id());
			}
		};

		aux_printf("    textures:\n");
		prt_tex("albedomap", albedomap);
		prt_tex("specularmap", specularmap);
		prt_tex("emissionmap", emissionmap);
		prt_tex("armmap", armmap);
		prt_tex("normalmap", normalmap);
		prt_tex("heightmap", heightmap);
		prt_tex("brdfmap", brdfmap);
		prt_tex("lightmap", lightmap);
		prt_tex("irradmap", irradmap);
		aux_printf("\n");
	}
}
GsObject::ClassCreator<GsDrawcall> GsDrawcall::ms_classCreator;
}  // namespace spu
