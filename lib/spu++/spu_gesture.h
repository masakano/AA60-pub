//
// SpuGesture :
//
#pragma once

#include <smath/vec.h>
#include <spu/spu.h>

namespace spu {

class SpuGesture {
public:
	SpuGesture() = default;
	SpuGesture(SpuPad *pad) { init(pad); }

	void init(SpuPad *pad);
	void update();
	bool touched() const;
	void setUsage(int16_t code, const char *usage);

	SpuPad *native() const { return m_native; }
	const SpuPad &curr() const { return m_curr; }
	const SpuPad &prev() const { return m_prev; }

	SpuPad &curr() { return m_curr; }
	SpuPad &prev() { return m_prev; }

	int32_t wheel() const { return m_wheel; }
	const int16_t *anchorL() const { return m_anchorL; }
	const int16_t *anchorR() const { return m_anchorR; }

	bool moved() const
	{
		return m_curr.cursor[0] != m_prev.cursor[0] || m_curr.cursor[1] != m_prev.cursor[1] || m_wheel;
	}

	bool pressed() const { return m_curr.key != 0 && m_curr.key != m_prev.key; }  // tricky
	bool release() const { return m_curr.key == 0 && m_prev.key != 0; }

	bool press(int16_t code) const { return m_curr.code == code; }
	bool pressed(int16_t code) const
	{
		return m_curr.code == code && m_curr.sticky(code) != m_prev.sticky(code);
	}
	bool keyPress(int16_t key) const { return m_curr.key == key; }
	bool keyPressed(int16_t key) const { return m_curr.key == key && m_prev.key != key; }
	bool keyReleased(int16_t key) const { return m_curr.key != key && m_prev.key == key; }
	uint8_t sticky(int16_t code) const { return m_curr.sticky(code); }
	std::vector<std::string> usages();
	static const void *getGrab() { return ms_grab; }
	static void setGrab(const void *grab) { ms_grab = grab; }
	static const char **symbolmap();

private:
	SpuPad *m_native = nullptr;  // external pad
	SpuPad m_curr;
	SpuPad m_prev;

	int32_t m_wheel = 0;
	int16_t m_anchorL[2] = {0};
	int16_t m_anchorR[2] = {0};
	const char *m_usages[256] = {nullptr};
	inline static const void *ms_grab = nullptr;
};
}  // namespace spu
