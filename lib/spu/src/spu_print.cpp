//
// Painter :
//
#include "spu_object.h"
#include "spu_print_painter.h"

namespace spu {
namespace libspu::spu_print {

class PrintObject : public Object {
public:
	inline static LocalPool ms_pool;
	Painter *m_painter = nullptr;

	explicit PrintObject(const Attrs &attrs)
	{
		m_painter = new Painter(attrs);
		m_handle.ui = ms_pool.alloc();
	}

	~PrintObject() override
	{
		ms_pool.free(m_handle.ui);
		delete m_painter;
	}

	void begin() { m_painter->begin(); }
	void puts(const char *str) { m_painter->puts(str); }
	void end() const { m_painter->end(); }
	void set(const Attrs &attrs) { m_painter->set(attrs); }
	inline static int32_t get(const hash32_t &key, void *value) { return Painter::get(key, value); }
};

class PrintManager : public Manager<PrintObject> {
public:
	explicit PrintManager(const char *name) : Manager<PrintObject>(name, 2) {}

	void startup() override { Painter::startup(); }
};
PrintManager s_objects("print");
}  // namespace libspu::spu_print

using namespace libspu;
using namespace libspu::spu_print;

uint32_t spu_print_new(const Attrs &attrs)
{
	SET();
	return s_objects.add(attrs).ui;
}

int32_t spu_print_get(uint32_t print_id, const hash32_t &key, void *value)
{
	GET_CHK(print_id, key);
	return s_objects.at(print_id)->get(key, value);
}

void spu_print_delete(uint32_t print_id)
{
	SET();
	s_objects.remove(print_id);
}

void spu_print_begin(uint32_t print_id)
{
	SET();
	s_objects.at(print_id)->begin();
}

void spu_print_end(uint32_t print_id)
{
	SET();
	s_objects.at(print_id)->end();
}

void spu_print_set(uint32_t print_id, const Attrs &attrs)
{
	SET();
	s_objects.at(print_id)->set(attrs);
}

int32_t spu_printf(uint32_t print_id, const char *fmt, ...)
{
	SET();

	va_list args;
	int32_t result;
	char buf[512];
	va_start(args, fmt);
	result = vsprintf(buf, fmt, args);
	va_end(args);
	s_objects.at(print_id)->puts(buf);
	return result;
}

}  // namespace spu
