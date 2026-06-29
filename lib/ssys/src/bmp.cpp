//
// File :
//
#include <ssys/ssys.h>

namespace spu {

class BmpFile : public File {
public:
	int32_t fsize;     // file size (14+40+pallete)
	int32_t reserve;   // reserved;
	int32_t offset;    // offset to image
	int32_t hsize;     // header size (Window BMP is 40)
	int32_t width;     // width  (aligned by 4byte)
	int32_t height;    // height (if minus, orign is left-upper corner)
	int16_t nplane;    // number of plane
	int16_t depth;     // bit per pixel (1, 4, 8, 24, 32)
	int32_t compress;  // compression code (0: uncompressed)
	int32_t size;      // image size
	int32_t res_x;     // resolution x (pixel/meter)
	int32_t res_y;     // resolution y (pixel/meter)
	int32_t nused;     // number of palette
	int32_t imprt;     // important color

	bool open(const std::filesystem::path &path, const std::string &mode)
	{
		return File::open(path, mode, false);  // no abort
	}

	template<class T> inline void put(T val) { write(&val, sizeof(T)); }
	template<class T> inline void get(T *val) { read(val, sizeof(T)); }

	void writeHeader(int32_t width, int32_t height, int32_t isrev)
	{
		std::array<char, 2> id = {'B', 'M'};

		fsize = size + offset;
		reserve = 0;
		offset = 14 + 40;
		hsize = 40;
		this->width = width;
		this->height = isrev != 0 ? height : -height;
		nplane = 1;
		depth = 8 * 3;
		compress = 0;
		size = width * height * 3;
		res_x = 3780;  // 96dpi, 1inch = 0.0254m,  96/0.0254 = 3780
		res_y = 3780;
		nused = 0;
		imprt = 0;

		write(id.data(), id.size());

		put(fsize);
		put(reserve);
		put(offset);
		put(hsize);
		put(width);
		put(height);
		put(nplane);
		put(depth);
		put(compress);
		put(size);
		put(res_x);
		put(res_y);
		put(nused);
		put(imprt);
	}

	bool readHeader(int32_t *new_width, int32_t *new_height, int32_t *new_depth)
	{
		std::array<char, 2> id;

		if (id.size() != read(id.data(), id.size(), false)) {
			return false;
		}
		get(&fsize);
		get(&reserve);
		get(&offset);
		get(&hsize);
		get(&width);
		get(&height);
		get(&nplane);
		get(&depth);
		get(&compress);
		get(&size);
		get(&res_x);
		get(&res_y);
		get(&nused);
		get(&imprt);

#ifdef MAINTENANCE
		aux_printf("readHeader:\n");
		aux_printf("   offset: %d\n", offset);
		aux_printf("   hsize:  %d\n", hsize);
		aux_printf("   width:  %d\n", width);
		aux_printf("   height: %d\n", height);
		aux_printf("   depth:  %d\n", depth);
		aux_printf("   size:   %d\n", size);
		aux_printf("   plane:  %d\n", nplane);
#endif

		std::array<char, 2> ref_id = {'B', 'M'};

		aux_error(id != ref_id, "not bmp file");
		aux_error(nplane != 1, "unsupported format (nplane=%d depth=%d", nplane, depth);

		*new_width = width;
		*new_height = height;
		*new_depth = depth;
		return true;
	}
};

void bmp_write(const char *name, uint8_t *buf, int32_t width, int32_t height, int32_t isrev)
{
	BmpFile file;

	aux_error(file.open(name, "wb") == false, "bmp_write: cannot open '%s'\n", name);
	file.writeHeader(width, height, isrev);
	file.write(buf, size_t(width * height * 3));
}

uint8_t *bmp_read(const char *name, int32_t *width, int32_t *height, int32_t *bpp)
{
	fflush(stdout);

	BmpFile file;

	if (!file.open(name, "rb")) {
		return nullptr;
	}

	if (!file.readHeader(width, height, bpp)) {
		return nullptr;
	}

	size_t size = (*width) * (*height) * (*bpp) / 8;
	auto *buf = new u_char[size];
	auto ret = file.read(buf, size, false);
	if (ret != size) {
		delete[] buf;
		return nullptr;
	}
	return buf;
}
}  // namespace spu
