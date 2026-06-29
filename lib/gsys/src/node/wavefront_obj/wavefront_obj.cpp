//
// Loader :
//
#include "parser.h"

#include <gsys/node/wavefront_obj.h>
#include <gsys/util/lambert_to_pbr.h>
#include <gsys/painter.h>
#include <smath/color_chart.h>
#include <thread>

namespace spu::gs_node::wavefront {
class Loader {
public:
	enum {
		e_no_cache = 0,
		e_use_cache,
		e_use_cache_in_current,
	};

	Loader(const Attrs &attrs, std::vector<Mesh> &meshes);
	~Loader();

	void start();
	bool isRun() const { return m_isRun; }
	const std::filesystem::path &path() const { return m_path; }

private:
	std::vector<Mesh> &m_meshes;
	std::string m_header = GIT_REVISION;
	std::filesystem::path m_path;
	std::thread *m_thread = nullptr;
	std::vector<std::string> m_touches;

	bool m_isRun = false;
	int32_t m_cacheMode = 0;

	void readFromObjFile();
	void saveToCache();
	bool loadFromCache();

	size_t selfSerialize(uint8_t *heap, bool is_dry);
	size_t selfDeserialize(const uint8_t *heap);

	std::filesystem::path cachePath() const
	{
		switch (m_cacheMode) {
		case e_use_cache: return make_cache_path(m_path);
		case e_use_cache_in_current: return m_path.string() + ".cache";
		default: assert(0);
		}
		return {};
	}
};

Loader::Loader(const Attrs &attrs, std::vector<Mesh> &meshes) : m_meshes(meshes)
{
	m_cacheMode = attrs.get<int32_t>("use_cache", e_use_cache);
	m_path = attrs.get("path", "");
	aux_error(m_path.empty(), "no path\n");
	m_path = File::searchPath(m_path);
}

Loader::~Loader()
{
	if (m_thread) {
		m_thread->join();
		delete m_thread;
		m_thread = nullptr;
	}
}

void Loader::start()
{
	auto background = [&]() {
		if (m_cacheMode != e_no_cache) {
			if (!loadFromCache()) {
				readFromObjFile();
				saveToCache();
			}
		}
		else {
			readFromObjFile();
		}
		m_isRun = false;
	};
	m_isRun = true;
	assert(m_thread == nullptr);
	m_thread = new std::thread(background);
}

void Loader::readFromObjFile()
{
	Parser parser(m_meshes);
	parser.load(m_path);
	m_touches = parser.paths();
	aux_message(1, "'%s' : building complete\n", m_path.c_str());
}

void Loader::saveToCache()
{
	auto size = selfSerialize(nullptr, true);
	std::vector<uint8_t> heap(size);
	selfSerialize(heap.data(), false);
	File file(cachePath(), "wb");
	file.write(heap.data(), heap.size());
}

bool Loader::loadFromCache()
{
	auto cache_path = cachePath();

	if (!std::filesystem::exists(cache_path)) {
		aux_message(0, "'%s' : not found (rebuild)\n", cache_path.string().c_str());
		return false;
	}
	auto heap = read_from_file<std::vector<uint8_t>>(cache_path);
	selfDeserialize(heap.data());

	if (m_header != GIT_REVISION) {
		aux_message(0, "'%s' : GIT_REVISION mismatch (rebuild)\n", cache_path.string().c_str());
		return false;
	}

	auto mtime = File(cache_path).mtime();
	for (auto &touch_path: m_touches) {
		if (mtime < File(touch_path).mtime()) {
			aux_message(0, "'%s' : expired (rebuild)\n", touch_path.c_str());
			return false;
		}
	}

	aux_message(1, "load from cache (%s)\n", cache_path.string().c_str());
	return true;
}

size_t Loader::selfSerialize(uint8_t *heap, bool is_dry)
{
	auto *hp = heap;
	auto header = std::string(GIT_REVISION);

	hp += serialize(hp, is_dry, header);
	hp += serialize(hp, is_dry, m_touches);
	hp += serialize(hp, is_dry, m_meshes);
	return hp - heap;
}

size_t Loader::selfDeserialize(const uint8_t *heap)
{
	auto *hp = heap;
	hp += deserialize(hp, m_header);
	if (m_header == GIT_REVISION) {
		hp += deserialize(hp, m_touches);
		hp += deserialize(hp, m_meshes);
	}
	return hp - heap;
}
}  // namespace spu::gs_node::wavefront

namespace spu::gs_node {

WavefrontObj::~WavefrontObj() { delete m_loader; }

void WavefrontObj::init(const Attrs &attrs)
{
	m_attrs = attrs;
	m_attrs.preserve();
	GsNode::init(m_attrs);
}

void WavefrontObj::replacePainter(GsPainter *painter)
{
	if (painter) {
		assert(m_loader == nullptr);
		m_loader = new wavefront::Loader(m_attrs, m_meshes);
		m_loader->start();
	}
	GsNode::replacePainter(painter);
}

bool WavefrontObj::doSync(bool is_nonblock)
{
	if (getPainter() == nullptr) {
		aux_message(0, "sync() without painter (forgot replacePainter()?)\n");
	}
	if (m_loader == nullptr) {
		return GsNode::doSync(is_nonblock);
	}
	else if (m_loader->isRun() == false) {
		lazySetPainter();
		delete m_loader;
		m_loader = nullptr;
		m_meshes.clear();
		return GsNode::doSync(is_nonblock);
	}
	return true;
}

void WavefrontObj::setMaterials(GsDrawcall &drawcall, const std::filesystem::path &path, const Attrs &attrs)
{
	auto &ub_material = drawcall.ub_material;
	if (attrs.peek({"Kd", "Ka", "Ks", "Ns"})) {
		auto diffuse = srgb_to_linear(attrs.get("Kd", vec4f_t(1.0)));
		auto ambient = srgb_to_linear(attrs.get("Ka", vec4f_t(0.0)));
		auto specular = srgb_to_linear(attrs.get("Ks", vec4f_t(0.0)));
		auto specular_power = attrs.get("Ns", 0.0f);
		GsLambertToPBR ltb(diffuse, ambient, specular, specular_power);

		ub_material.albedo = Vec4f(diffuse, 1.0f);
		ub_material.metallic = ltb.metallic();
		ub_material.roughness = ltb.roughness();
		ub_material.ao = ltb.ao();
	}

	auto replaced_attrs = attrs;
	auto replace = [&](const hash32_t &official_key, const std::vector<hash32_t> &keys) {
		auto all_keys = keys;
		all_keys.push_back(official_key);
		for (auto &attr: replaced_attrs) {
			auto it = vector_find(all_keys, attr.key());
			if (it != end(all_keys)) {
				auto relative_path = std::string(attr);
				std::replace(begin(relative_path), end(relative_path), '\\', '/');
				auto full_path = path.parent_path().string() + '/' + relative_path;
				attr = Attr(official_key, full_path);
				attr.preserve();
			}
		}
	};

	replace("drawcall.texture.albedo.path", {"map_albeo", "Map_Kd", "map_Kd", "map_kd", "map_d"});
	replace("drawcall.texture.emission.path", {"map_emission"});
	replace("drawcall.texture.normal.path", {"map_normal", "norm"});
	replace("drawcall.texture.roughness.path", {"map_roughness"});
	replace("drawcall.texture.metallic.path", {"map_metallic"});
	replace("drawcall.texture.ao.path", {"map_ao"});
	replace("drawcall.texture.height.path", {"map_height", "map_bump", "map_Bump", "Map_bump", "Map_Bump"});

	// replaced_attrs.report("wavefront.replaced_attrs");
	drawcall.set(replaced_attrs.select("drawcall."));
}

void WavefrontObj::lazySetPainter()
{
	assert(m_loader);

	auto painter = getPainter();
	auto &path = m_loader->path();

	std::vector<Mesh::Vertex> soup_vertices;
	Mesh::postproc(m_attrs, m_meshes, soup_vertices);

	std::vector<int32_t> indices;

	// safety
	for (auto &vert: soup_vertices) {
		vert.n = normalize<Vec3f>(vert.n);
	}

	// index
	for (auto &mesh: m_meshes) {
		for (const auto &face: mesh.getFaces()) {
			for (const auto &index: face) {
				indices.push_back(index);
			}
		}
	}

	// report
	aux_message(1, "load '%s'\n", path.c_str());
	for (auto &mesh: m_meshes) {
		aux_message(
		        1, "%8ld %8d %-24s\n", mesh.getFaces().size(), mesh.getIndexCount(),
		        mesh.getAttrs().get("name", ""));
	}
	painter->send(soup_vertices, indices);
	painter->update();

	auto &drawcalls = painter->getDrawcalls();
	drawcalls.clear();
	drawcalls.reserve(m_meshes.size());

	auto first = 0;
	for (auto &mesh: m_meshes) {
		GsDrawcall drawcall;

		auto attrs = (mesh.getAttrs() + m_attrs).uniq();
		// attrs.trace(nullptr);  // stop trace

		setMaterials(drawcall, path, attrs);

		// for LOD
		auto count = mesh.getIndexCount();
		const char *mode_str = attrs.get("mode", "GL_TRIANGLES");

		drawcall.coms[0].mode = opengl_const(mode_str);
		drawcall.coms[0].target = GL_ELEMENT_ARRAY_BUFFER;
		drawcall.coms[0].first = first;
		drawcall.coms[0].count = count;

		auto name = attrs.getf<const char *>("name");
		if (name.hit) {
			drawcall.ub_material.hash = hash32_t(name.value).value();
		}

		drawcalls.push_back(drawcall);
		first += count;
	}

	// finish
	{
		setName(m_loader->path().stem().string());
		getARange() = painter->getRange();
	}
}
}  // namespace spu::gs_node
