//
// GsPage :
//
#pragma once

#include <ssys/object_registry.h>
#include <gsys/canvas.h>
#include <gsys/node/icamera.h>

namespace spu {
class SpuBackoffice;
}

namespace spu::gs_canvas {

class GsPage : public GsCanvas {
public:
	static constexpr hash32_t e_show_tweakbar = "show_tweakbar";

	class IDecorator {
	public:
		virtual ~IDecorator() = default;
		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void set(const Attrs &attrs) = 0;
	};
	class IPostproc {
	public:
		virtual ~IPostproc() = default;
		virtual void postproc(GsPage *page) = 0;
	};

	class ITweakbar {
	public:
		virtual ~ITweakbar() = default;
		virtual void draw(GsPage *page) = 0;
	};

	explicit GsPage(const char *name = nullptr) : GsCanvas(name) {}
	explicit GsPage(const Attrs &attrs) : GsPage() { init(attrs); }
	~GsPage();

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	using GsObject::set;
	void begin() override;
	void end() override;

	const SpuTexture &getBuffer(const hash32_t &slot = "color0") const override;
	SpuTexture &getBuffer(const hash32_t &slot = "color0") override;

	SpuBackoffice *getBackoffice() const { return m_backoffice; }
	SpuGesture *getGesture() const { return m_gesture; }
	gs_node::ICamera *getCamera() const { return m_camera; }
	IPostproc *getPostproc() const { return m_postproc; }

	void replaceGesture(SpuGesture *gesture) { reset(m_gesture, gesture); }
	void replaceCamera(gs_node::ICamera *camera) { reset(m_camera, camera); }
	void replacePostproc(IPostproc *postproc) { reset(m_postproc, postproc); }
	void replaceTweakbar(ITweakbar *tweakbar) { reset(m_tweakbar, tweakbar); }
#if 0
	[[deprecated("use replaceGesture()")]] void setGesture(SpuGesture *gesture) { replaceGesture(gesture); }
	[[deprecated("use replaceCamera()")]] void setCamera(gs_node::ICamera *camera)
	{
		replaceCamera(camera);
	}
	[[deprecated("use replacePostproc()")]] void setPostproc(IPostproc *postproc)
	{
		replacePostproc(postproc);
	}
	[[deprecated("use replaceTweakbar()")]] void setTweakbar(ITweakbar *tweakbar)
	{
		replaceTweakbar(tweakbar);
	}
#endif
	void setGlobalSrgb(bool is_srgb);

	IPostproc *movePostproc();
	ITweakbar *moveTweakbar();

protected:
	template<class T> void reset(T *&ptr, T *new_ptr)
	{
		delete ptr;
		ptr = new_ptr;
	}
	void addDecorator(IDecorator *decorator) { m_decorators.push_back(decorator); }
	void beginDecorators();
	void endDecorators();
	void disposeDecorators();
	void setDecorators(const Attrs &attrs);

private:
	void initMultisample(const Attrs &attrs);
	std::vector<IDecorator *> m_decorators;
	SpuGesture *m_gesture = nullptr;
	gs_node::ICamera *m_camera = nullptr;
	IPostproc *m_postproc = nullptr;
	ITweakbar *m_tweakbar = nullptr;
	int32_t m_multisample = 0;

	SpuBackoffice *m_backoffice = nullptr;
};
}  // namespace spu::gs_canvas
namespace spu {
extern template class ObjectRegistry<GsCanvas>;
}
