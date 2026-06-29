//
// GsNode :
//
#pragma once

#include "painter.h"
#include "canvas.h"
#include <smath/substance.h>

namespace spu {

/// graphical object
class GsNode : public GsObject, public Substance {
public:
	static constexpr int32_t e_class_depth = GsObject::e_class_depth + 1;
	static constexpr hash32_t e_lod = "lod";
	static constexpr hash32_t e_flip = "flip";
	explicit GsNode(const char *name = nullptr);
	explicit GsNode(const Attrs &attrs) : GsNode() { init(attrs); }

	~GsNode();

	virtual void render(const std::vector<Mat4f> &nodeworlds = {Mat4f()});
	virtual std::vector<Vec3f> points(const std::vector<Mat4f> &nodeworlds = {Mat4f()});
	virtual std::vector<Vec3f> childPoints(const std::vector<Mat4f> &nodeworlds = {Mat4f()});

	virtual bool hasShader(const hash32_t &type) const;
	virtual void replacePainter(GsPainter *painter);
	virtual GsPainter *movePainter();
	virtual GsPainter *getPainter() { return m_painter; }
	virtual const GsPainter *getPainter() const { return m_painter; }

	virtual void addChildren(const std::vector<GsNode *> &children);
	virtual void replaceChildren(const std::vector<GsNode *> &children);
	virtual std::vector<GsNode *> moveChildren();
	virtual const std::vector<GsNode *> &getChildren() const { return m_children; }
	virtual GsNode *getParent() const { return m_parent; }

	void set(const Attrs &attrs) override;
	using GsObject::set;

	void dispose() override;
	void update() override;
	void report(const char *str) const override;
	void startInspector() override;
	void setProperty(const hash32_t &type, int32_t value) override;

protected:
	bool doSync(bool is_nonblock) override;
	virtual bool doBuild(const std::vector<Mat4f> &nodeworlds);
	virtual void doRender();
	virtual void doDebugRender();

private:
	std::vector<GsNode *> m_children;
	GsPainter *m_painter = nullptr;
	GsNode *m_parent = nullptr;
};
}  // namespace spu
