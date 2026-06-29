//
// PacketBase :
//
#include "base_app.h"
namespace spu::packetbuffer {

class PacketBase {
public:
	virtual void execute() = 0;
	virtual ~PacketBase() = default;
};

class PacketStream : public std::vector<PacketBase *> {
public:
	void execute()
	{
		for (auto packet: *this) {
			packet->execute();
		}
	}
	void dispose()
	{
		for (auto &packet: *this) {
			delete packet;
		}
		clear();
	}
};

class ShaderUsePacket : public PacketBase {
public:
	ShaderUsePacket(const char *path, const Attrs &shader_attrs, const Attrs &unif_attrs)
	{
		m_shaderId = spu_inventory_new("shader", path, shader_attrs);
		std::vector<const char *> names;
		for (auto &attr: unif_attrs) {
			names.push_back(attr.key().c_str());
			m_ptrs.push_back(attr);
		}
		m_locs.resize(m_ptrs.size());
		spu_shader_loc(m_shaderId, names.data(), m_locs.data(), 0, 0, names.size());
	}
	void execute() override { spu_shader_use(m_shaderId, m_locs.data(), m_ptrs.data(), m_locs.size()); }

private:
	uint32_t m_shaderId;
	std::vector<int32_t> m_locs;
	std::vector<void *> m_ptrs;
};

class ArrayDrawPacket : public PacketBase {
public:
	ArrayDrawPacket(
	        uint32_t array_id, int32_t mode, int32_t first = 0, int32_t count = 0,
	        int32_t instance_count = 1, int32_t target = 0)
	        : m_arrayId(array_id), m_mode(mode), m_first(first), m_count(count),
	          m_instanceCount(instance_count), m_target(target)
	{
	}
	void execute() override
	{
		spu_array_draw(m_arrayId, m_mode, m_first, m_count, m_instanceCount, m_target);
	}

private:
	uint32_t m_arrayId;
	int32_t m_mode;
	int32_t m_first;
	int32_t m_count;
	int32_t m_instanceCount;
	int32_t m_target;
};

class App : public BaseApp {
public:
	static constexpr int32_t c_n_instance = 7 * 7;
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	~App() { m_stream.dispose(); }
	struct {
		Mat4f modelview;
		Mat4f worldview;
		Mat4f viewscreen;
	} constants[64];
	struct {
		Vec4f diffuse_albedo;
		Vec4f specular_albedo;  // w: power
	} materials[64];
	sb6::Object m_object;
	PacketStream m_stream;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs shader_attrs = {
		        {"preface", "#version 410 core\n"},
		};
		Attrs unif_attrs = {
		        {"UB_CONSTANT", &constants[0]},
		        {"UB_MATERIAL", &materials[0]},
		};
		m_stream.push_back(new ShaderUsePacket("blinnphong/blinnphong.us", shader_attrs, unif_attrs));
	}
	// object
	{
		m_object.load("torus.sbm");
		m_stream.push_back(
		        new ArrayDrawPacket(m_object.getArray().id(), GL_TRIANGLES, 0, 0, c_n_instance));
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cull_face = true;
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		// renderstate.use();
	}
}

void App::render()
{
	auto t = getSeconds().current();
	auto eye = Vec3f(0.0, 0.0, 20.0);
	sb6::Composition composition;
	composition.lookat(eye, ezero(), ey());
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	auto worldview = composition.worldview();
	auto viewscreen = composition.viewscreen();
	auto modelworld = c_unit.rot("XZY", -20.0, -180.0, -t * 14.5 * 8.0);
	auto n = 0;
	for (auto j = 0; j < 7; j++) {
		for (auto i = 0; i < 7; i++, n++) {
			constants[n].worldview = worldview;
			constants[n].viewscreen = viewscreen;
			constants[n].modelview = worldview
			                       * c_unit.trans({i * 2.75f - 8.25f, 6.75f - j * 2.25f, 0.0f})
			                       * modelworld;
			materials[n].diffuse_albedo = {0.5, 0.2, 0.7, 1.0};
			materials[n].specular_albedo = Vec3f(i / 9.0 + 1.0 / 9.0);
			materials[n].specular_albedo.w = powf(2.0, j + 2.0);
		}
	}
	assert(n == c_n_instance);
	m_stream.execute();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("packetbuffer");
}  // namespace spu::packetbuffer
