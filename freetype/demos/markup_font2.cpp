//
// MarkupFont2 :
//
#include "writer.h"
#include "app.h"

using namespace spu::gs_painter;

namespace spu {

class MarkupFont2 : public App {
public:
	int32_t width() const override { return 500; }
	int32_t height() const override { return 300; }

	FTWriter writer;

	void init() override
	{
		writer.init();

		writer.clear();
		writer.getPen() = {0, 300};
		writer.addText(FTWriter::e_underline, "The");
		writer.addText(FTWriter::e_normal, " Quick");
		writer.addText(FTWriter::e_big, " brown ");
		writer.addText(FTWriter::e_reverse, " fox \n");
		writer.addText(FTWriter::e_italic, "jumps over ");
		writer.addText(FTWriter::e_bold, "the lazy ");
		writer.addText(FTWriter::e_normal, "dog.\n");
		writer.addText(
		        FTWriter::e_small,
		        "Now is the time for all good men to come to the aid of the party.\n");
		writer.addText(FTWriter::e_italic, "Ég get etið gler án þess að meiða mig.\n");
		writer.flush(FTText::e_align_center);

		m_bgcolor0 = Vec4f(0.40, 0.40, 0.45, 1.00);
	}

	void doDisplay() override
	{
		writer.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		writer.render();
	}
};
App *createMarkupFont2() { return new MarkupFont2(); }
}  // namespace spu
