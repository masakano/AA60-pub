//
// WavefrontObj :
//
#pragma once
#include <gsys/node.h>

namespace spu::gs_node {
namespace wavefront {
class Loader;
}  // namespace wavefront

class WavefrontObj : public GsNode {
public:
	explicit WavefrontObj(const char *name = nullptr) : GsNode(name) {}
	explicit WavefrontObj(const Attrs &attrs) : WavefrontObj() { init(attrs); }
	~WavefrontObj() override;

	void replacePainter(GsPainter *painter) override;
	void init(const Attrs &attrs) override;

protected:
	Attrs m_attrs;
	std::vector<Mesh> m_meshes;
	bool doSync(bool is_nonblock) override;

private:
	wavefront::Loader *m_loader = nullptr;
	void setMaterials(GsDrawcall &drawcall, const std::filesystem::path &path, const Attrs &attrs);
	void lazySetPainter();
};

}  // namespace spu::gs_node
