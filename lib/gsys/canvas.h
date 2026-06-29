//
// GsCanvas :
//
#pragma once

#include "object.h"
#include <smath/geometry.h>
#include <smath/composition.h>
#include <spu++/spu++.h>
#include <gsys/shaders/default/ub_light.us>
#include <gsys/shaders/default/ub_connect.us>

namespace spu {

class GsCanvas : public SpuFrame, public GsObject, public Composition {
public:
	static constexpr int32_t e_class_depth = GsObject::e_class_depth + 1;

	UB_LIGHT ub_light;
	UB_CONNECT ub_connect;

	uint32_t u_color0 = 0;
	uint32_t u_color1 = 0;
	uint32_t u_color2 = 0;
	uint32_t u_color3 = 0;
	uint32_t u_color4 = 0;
	uint32_t u_color5 = 0;
	uint32_t u_color6 = 0;
	uint32_t u_color7 = 0;
	uint32_t u_color8 = 0;
	uint32_t u_color9 = 0;
	uint32_t u_depth = 0;
	uint32_t u_stencil = 0;
	uint32_t u_multisample = 0;

	explicit GsCanvas(const char *name = nullptr);
	explicit GsCanvas(const Attrs &attrs) : GsCanvas() { init(attrs); }
	~GsCanvas();

	virtual void render();
	virtual void begin();
	virtual void clear();
	virtual void end();

	virtual void takeover(const GsCanvas *canvas = nullptr);
	virtual void syncComposition();

	virtual SpuTexture &getBuffer(const hash32_t &slot = "color0") { return SpuFrame::getBuffer(slot); }
	virtual const SpuTexture &getBuffer(const hash32_t &slot = "color0") const
	{
		return SpuFrame::getBuffer(slot);
	}

	virtual SpuRenderstate &getRenderstate() { return m_renderstate; }
	virtual const SpuRenderstate &getRenderstate() const { return m_renderstate; }

	virtual SpuArray &getArray() { return m_array; }
	virtual const SpuArray &getArray() const { return m_array; }

	virtual SpuShader &getShader() { return m_shader; }
	virtual const SpuShader &getShader() const { return m_shader; }

	virtual hash32_t &getShaderType() { return m_shaderType; }
	virtual const hash32_t &getShaderType() const { return m_shaderType; }

	virtual Attrs &getShaderAttrs() { return m_shaderAttrs; }
	virtual const Attrs &getShaderAttrs() const { return m_shaderAttrs; }

	void dispose() override;
	void init(const Attrs &attrs) override;

	void set(const Attrs &attrs) override;
	using GsObject::set;

	void report(const char *str) const override;
	bool peek(int32_t x, int32_t y, const hash32_t &slot, void *value) const;
	bool isReal() const;
	bool isAlive() const { return getProperty("alive"); }

	static const std::vector<GsCanvas *> &getStack();
	static GsCanvas *getCurrent();
	static GsCanvas *getPrevious();
	static GsCanvas *getDefault();
	static GsCanvas *getReal();

protected:
	bool doSync(bool is_nonblock) override;

private:
	SpuRenderstate m_renderstate;
	SpuShader m_shader;
	SpuArray m_array;
	hash32_t m_shaderType = "radiance";
	Attrs m_shaderAttrs;

	static std::vector<GsCanvas *> &ms_stack()
	{
		static std::vector<GsCanvas *> v;
		return v;
	}
	inline static GsCanvas *ms_default = nullptr;

	void setLight(const Attrs &attrs);
	void setWorldview(const Attrs &attrs);
	void setViewscreen(const Attrs &attrs);

	bool isInStack();
	bool isInShadow();
	Attrs uniformAttrs() const;

	friend struct ClassCreator<GsCanvas>;
	static void startup(const Attrs &attrs);
	static void shutdown();
	static ClassCreator<GsCanvas> ms_classCreator;
};
}  // namespace spu
