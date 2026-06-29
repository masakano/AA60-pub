//
// App :
//
#include "base_app.h"
#include <ssys/random_generator.h>
namespace spu::prefixsum {

class App : public BaseApp {
public:
	enum { e_num_elements = 2048 };

	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;

protected:
	float m_input[e_num_elements * 4];
	float m_output[e_num_elements * 4];
	float m_reference[e_num_elements * 4];
	uint32_t m_shaderId;
	SpuArray m_array;
	void startup();
	void render() override;
	void prefixSum4(const float *input, float *output, int32_t elements);  // 16byte boundary
	void printResult(const float *src, const float *dst, const float *ref, int32_t n);
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	startup();
}

void App::startup()
{
	// for safety
	memset(m_input, 0, sizeof(m_input));
	memset(m_output, 0, sizeof(m_output));
	memset(m_reference, 0, sizeof(m_reference));
	RandomGenerator<float> frand;
	for (auto i = 0; i < e_num_elements; i++) {
		m_input[i * 4] = frand();
	}
	prefixSum4(m_input, m_reference, e_num_elements);

	// loadShaders();
	{
		Attrs shader_attrs = {
		        {"def_local_size", 1024},
		};
		m_shaderId = spu_inventory_new("shader", "prefixsum/prefixsum.us", shader_attrs);
		spu_shader_use(m_shaderId);

		Attrs array_attrs0 = {
		        {"shader_id", m_shaderId           },
		        {"a.block1",  0                    },
		        {"data",      m_input},
		        {"nelem",     sizeof(m_input)      },
		};
		Attrs array_attrs1 = {
		        {"a.block2", 0                     },
		        {"data",     m_output},
		        {"nelem",    sizeof(m_output)      },
		};
		m_array.init(array_attrs0);
		m_array.aux(array_attrs1, 1);
	}
}

void App::printResult(const float *src, const float *dst, const float *ref, int32_t n)
{
	spu_printf(0, "\n");
	spu_printf(0, "  %-26s|   %-26s|  %-26s \n", "input", "output sum", "reference sum");
	for (auto i = 0; i < n; i++) {
		spu_printf(
		        0, "%6.2f %6.2f %6.2f %6.2f | %6.2f %6.2f %6.2f %6.2f | %6.2f %6.2f %6.2f %6.2f\n",
		        src[i * 4 + 0], src[i * 4 + 1], src[i * 4 + 2], src[i * 4 + 3], dst[i * 4 + 0],
		        dst[i * 4 + 1], dst[i * 4 + 2], dst[i * 4 + 3], ref[i * 4 + 0], ref[i * 4 + 1],
		        ref[i * 4 + 2], ref[i * 4 + 3]);
	}
}

void App::render()
{
	spu_shader_use(m_shaderId);
	m_array.draw(0xffff, 1, 1, 1);
	auto *ptr0 = m_array.map<const float *>("r", 0);
	auto *ptr1 = m_array.map<const float *>("r", 1);
	assert(ptr0);
	assert(ptr1);
	printResult(ptr0, ptr1, m_reference, 28);
	m_array.unmap(0);
	m_array.unmap(1);
}

void App::prefixSum4(const float *input, float *output, int32_t elements)
{
	auto f = 0.0f;
	int32_t i;
	for (i = 0; i < elements; i++) {
		f += input[i * 4];
		output[i * 4] = f;
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("prefixsum");
}  // namespace spu::prefixsum
