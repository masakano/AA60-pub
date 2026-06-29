//
//$<<Header>>$
//

#pragma once

namespace spu::oglplus::imports {

template<typename Derived, typename ElementType> class BlendFileRangeTpl {
private:
	std::size_t m_current, m_count;

	const Derived &derived() const { return static_cast<const Derived &>(*this); }

protected:
	BlendFileRangeTpl(std::size_t count) : m_current(0), m_count(count) {}

public:
	std::size_t size() const { return m_count - m_current; }

	bool empty() const { return m_current == m_count; }

	void next()
	{
		assert(!empty());
		++m_current;
	}

	ElementType front() const
	{
		assert(!empty());
		return derived().get(m_current);
	}

	ElementType at(std::size_t position) const
	{
		assert(m_count - m_current > position);
		return derived().get(m_current + position);
	}
};

}  // namespace spu::oglplus::imports
