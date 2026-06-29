//
// Instance :
//
#include <gsys/canvas.h>
#include <gsys/decorator/instance.h>

namespace spu::gs_decorator {
// static_assert(std::is_trivially_copyable<UbComposition>::value, "not copyable");

namespace {
//  slot type            type    contents
// ------------------------------------------------------
//  e_slot_vertices      static  vertices
//  e_slot_transforms    static  transform matrices
//  e_slot_bones         static  bone indices and weights
//  e_slot_morphs        static  morph target
//  e_slot_morph_targets dynamic morph indices and rates
//  e_slot_bone_matrices dynamic bone matrices
enum {
	e_slot_vertices = 0,
	e_slot_transforms = 1,
	e_slot_bones = 2,
	e_slot_morphs = 3,
	e_slot_bone_matrices = 4,
	e_slot_morph_targets = 5,
};
}  // namespace

Instance::Instance(GsPainter *painter, const Attrs &attrs) : GsDecorator(painter, attrs)
{
	// auto &array = m_painter->getArray();
	auto array_id = attrs.get("array_id", 0);  // check ownership

	auto array_aux = [&](const Attrs &def_attrs, int32_t slot) {
		std::string slot_str = std::to_string(slot) + ".";
		m_painter->aux(def_attrs + attrs.select(slot_str.c_str()), slot);
	};

	def_use_animation = attrs.get("def_use_animation", def_use_animation);
	def_use_morphing = attrs.get("def_use_morphing", def_use_morphing);

	// uniforms
	{
		painter->addUniforms(uniforms());
	}

	// transforms
	{
		Attrs transform_attrs = {
		        {"divisor",                1 },
		        {"a.a_instance_nodeworld", 16},
		};
		array_aux(transform_attrs, e_slot_transforms);
	}

	// bone
	if (array_id == 0 && def_use_animation) {
		Attrs bone_attrs = {
		        {"format",     GL_INT  },
                        {"oformat",    GL_INT  },
                        {"a.a_bone",   4       },
		        {"format",     GL_FLOAT},
                        {"oformat",    GL_FLOAT},
                        {"a.a_weight", 4       },
		};
		array_aux(bone_attrs, e_slot_bones);
	}

	// bone buffer
	if (array_id == 0 && def_use_animation) {
		Attrs bone_matrix_attrs = {
		        {"a.a_bone_matrices", 16},
		};
		array_aux(bone_matrix_attrs, e_slot_bone_matrices);
	}

	// morph
	if (array_id == 0 && def_use_morphing) {
		Attrs morph_attrs = {
		        {"format",                  GL_INT  },
		        {"oformat",                 GL_INT  },
		        {"a.a_generic_morph_type",  1       },
		        {"a.a_generic_morph_index", 1       },
		        {"format",                  GL_FLOAT},
		        {"oformat",                 GL_FLOAT},
		        {"a.a_generic_morph_rate",  1       },
		};
		array_aux(morph_attrs, e_slot_morphs);
	}

	// morph buffer
	if (array_id == 0 && def_use_morphing) {
		Attrs morph_target_attrs = {
		        {"a.a_morph_targets", sizeof(Morph) / 4},
		};
		array_aux(morph_target_attrs, e_slot_morph_targets);
	}
}

Attrs Instance::uniforms() const
{
	Attrs unif_attrs = {
	        {"ub_composition", &ub_composition},
	};
	return unif_attrs;
}

void Instance::sendBones(const std::vector<Bone> &bones)
{
	if (def_use_animation) {
		m_painter->SpuArray::send(bones, e_slot_bones);
	}
	else {
		aux_message(0, "\"def_use_animation\" not set\n");
	}
}

void Instance::sendMorphs(const std::vector<Morph> &morphs)
{
	if (def_use_morphing) {
		m_painter->SpuArray::send(morphs, e_slot_morphs);
	}
	else {
		aux_message(0, "\"def_use_morphing\" not set\n");
	}
}

void Instance::sendMorphTargets(const std::vector<Vec4f> &morph_targets)
{
	if (def_use_morphing) {
		m_painter->SpuArray::send(morph_targets, e_slot_morph_targets);
	}
	else {
		aux_message(0, "\"def_use_morphing\" not set\n");
	}
}

void Instance::sendBoneMatrices(const std::vector<Mat4f> &bone_matrices)
{
	if (def_use_animation) {
		m_painter->SpuArray::send(bone_matrices, e_slot_bone_matrices);
	}
	else {
		aux_message(0, "'def_use_animation' not set\n");
	}
}

void Instance::doUse(uint32_t id)
{
	if (id == 0) {
		auto *current = GsCanvas::getCurrent();
		auto &uc = ub_composition;

		uc.worldview1 = uc.worldview;
		uc.viewworld1 = uc.viewworld;

		uc.worldview = current->worldview(0);
		uc.viewscreen = current->viewscreen();
		uc.screenfrag = current->screenfrag(0);
		uc.viewworld = current->viewworld(0);
		uc.screenview = current->viewscreen().inverse();
	}
	GsDecorator::doUse(id);
}
void Instance::doRender()
{
	m_painter->SpuArray::send(m_painter->instancePtr(), m_painter->instanceCount(), e_slot_transforms);
	GsDecorator::doRender();
}
}  // namespace spu::gs_decorator
