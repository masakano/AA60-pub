//
// VectorView :
//
#pragma once
#include "ssys.h"

namespace spu {

template<class view_t, class source_vector_t = std::vector<uint8_t>> class VectorView {
public:
	using source_t = typename source_vector_t::value_type;

	template<class T> struct is_vector : std::false_type {};
	template<class T, class Alloc> struct is_vector<std::vector<T, Alloc>> : std::true_type {};

	template<class T> struct is_const_vector : std::false_type {};
	template<class T, class Alloc> struct is_const_vector<const std::vector<T, Alloc>> : std::true_type {};

	class iterator {
	public:
		iterator(source_t *ptr, size_t stride) : m_ptr(ptr), m_stride(stride) {}

		iterator &operator++()
		{
			m_ptr += m_stride;
			return *this;
		}
		iterator &operator--()
		{
			m_ptr -= m_stride;
			return *this;
		}
		iterator operator+(const size_t n) const
		{
			iterator it = *this;
			it.m_ptr += n * m_stride;
			return it;
		}

		size_t operator-(const iterator &it) const { return (m_ptr - it.m_ptr) / m_stride; }
		const view_t &operator*() const { return *reinterpret_cast<const view_t *>(m_ptr); }
		view_t &operator*() { return *reinterpret_cast<view_t *>(m_ptr); }
		const view_t *operator->() const { return reinterpret_cast<const view_t *>(m_ptr); }
		view_t *operator->() { return reinterpret_cast<view_t *>(m_ptr); }
		bool operator==(const iterator &i1) const { return m_ptr == i1.m_ptr; }
		bool operator==(const void *ptr) const { return m_ptr == ptr; }

	private:
		source_t *m_ptr;
		size_t m_stride;
	};

	class const_iterator {
	public:
		const_iterator(const source_t *ptr, size_t stride) : m_ptr(ptr), m_stride(stride) {}

		const_iterator &operator++()
		{
			m_ptr += m_stride;
			return *this;
		}
		const_iterator &operator--()
		{
			m_ptr -= m_stride;
			return *this;
		}
		const_iterator operator+(const size_t n) const
		{
			const_iterator it = *this;
			it.m_ptr += n * m_stride;
			return it;
		}

		size_t operator-(const const_iterator &it) const { return (m_ptr - it.m_ptr) / m_stride; }
		const view_t &operator*() const { return *reinterpret_cast<const view_t *>(m_ptr); }
		const view_t *operator->() const { return reinterpret_cast<const view_t *>(m_ptr); }
		bool operator==(const const_iterator &i1) const { return m_ptr == i1.m_ptr; }
		bool operator==(const void *ptr) const { return m_ptr == ptr; }

	private:
		const source_t *m_ptr;
		size_t m_stride;
	};

	VectorView() = default;

	explicit VectorView(source_vector_t *source, size_t stride = sizeof(view_t) / sizeof(source_t))
	        requires(is_vector<source_vector_t>::value || is_const_vector<source_vector_t>::value)
	{
		init(source, stride);
	}
	void init(source_vector_t *source, size_t stride = sizeof(view_t) / sizeof(source_t))
	{
		m_source = source;
		m_stride = stride;
	}
	operator std::vector<view_t>() const
	{
		std::vector<view_t> result;
		result.reserve(size());
		for (auto &v: *this) {
			result.push_back(v);
		}
		return result;
	}
	VectorView &operator=(const std::vector<view_t> &view)
	{
		clear();
		reserve(view.size());
		for (auto &v: view) {
			push_back(v);
		}
		return *this;
	}
	const view_t &operator[](size_t index) const
	{
		return *reinterpret_cast<const view_t *>(&(*m_source)[source_index(index)]);
	}
	view_t &operator[](size_t index)
	        requires(!std::is_const_v<source_vector_t>)
	{
		return *reinterpret_cast<view_t *>(&(*m_source)[source_index(index)]);
	}
	void assign(source_vector_t *source)
	        requires(!std::is_const_v<source_vector_t>)
	{
		m_source = source;
	}
	const view_t *data() const { return reinterpret_cast<const view_t *>(m_source->data()); }
	view_t *data()
	        requires(!std::is_const_v<source_vector_t>)
	{
		return reinterpret_cast<view_t *>(m_source->data());
	}
	const view_t &at(size_t index) const
	{
		return *reinterpret_cast<const view_t *>(&m_source->at(source_index(index)));
	}
	view_t &at(size_t index)
	        requires(!std::is_const_v<source_vector_t>)
	{
		return *reinterpret_cast<view_t *>(&m_source->at(source_index(index)));
	}
	bool empty() const { return !m_source || m_source->empty(); }
	size_t size() const { return (!m_source || m_stride == 0) ? 0 : m_source->size() / m_stride; }
	void clear()
	        requires(!std::is_const_v<source_vector_t>)
	{
		m_source->clear();
	}
	void resize(size_t size)
	        requires(!std::is_const_v<source_vector_t>)
	{
		m_source->resize(source_index(size));
	}
	void reserve(size_t size)
	        requires(!std::is_const_v<source_vector_t>)
	{
		m_source->reserve(source_index(size));
	}
	void push_back(const view_t &elem)
	        requires(!std::is_const_v<source_vector_t>)
	{
		auto base = m_source->size();
		m_source->resize(base + m_stride);
		memcpy(&(*m_source)[base], &elem, sizeof(view_t));
	}
	void append(const std::vector<view_t> &elems)
	        requires(!std::is_const_v<source_vector_t>)
	{
		auto base = m_source->size();
		m_source->resize(base + m_stride * elems.size());
		memcpy(&(*m_source)[base], elems.data(), elems.size() * sizeof(view_t));
	}
	const_iterator begin() const
	{
		if (!m_source) {
			return const_iterator(nullptr, m_stride);
		}
		auto *ptr = m_source->data();
		return const_iterator(ptr, m_stride);
	}
	const_iterator end() const
	{
		if (!m_source) {
			return const_iterator(nullptr, m_stride);
		}
		auto ptr = m_source->data() + size() * m_stride;
		return const_iterator(ptr, m_stride);
	}

	iterator begin()
	        requires(!std::is_const_v<source_vector_t>)
	{
		if (!m_source) {
			return iterator(nullptr, m_stride);
		}
		auto ptr = m_source->data();
		return iterator(ptr, m_stride);
	}
	iterator end()
	        requires(!std::is_const_v<source_vector_t>)
	{
		if (!m_source) {
			return iterator(nullptr, m_stride);
		}
		auto ptr = m_source->data() + size() * m_stride;
		return iterator(ptr, m_stride);
	}

	const source_vector_t &source() const { return *m_source; }
	source_vector_t &source() const
	        requires(!std::is_const_v<source_vector_t>)
	{
		return *m_source;
	}

private:
	source_vector_t *m_source = nullptr;
	size_t m_stride = 0;

	size_t source_index(size_t index) const { return index * m_stride; }
};
}  // namespace spu
