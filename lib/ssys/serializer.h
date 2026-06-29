//
//
//
#pragma once
#include "ssys.h"

#define SPU_SERIALIZER_FRIENDS                                                                       \
	template<class T> friend size_t spu::serialize(uint8_t *heap, bool is_dry, const T &object); \
	template<class T> friend size_t spu::deserialize(const uint8_t *heap, T &object);

namespace spu {

template<class T> size_t serialize(uint8_t *heap, bool is_dry, const T &object)
{
	static_assert(std::is_trivially_copyable<T>::value, "not trivially copyable");
	static_assert(!std::is_pointer<T>::value, "pointer is not copyable");

	uint8_t *hp = heap;
	if (!is_dry) {
		memcpy(hp, &object, sizeof(object));
	}
	hp += sizeof(object);
	return hp - heap;
}

template<class T> size_t deserialize(const uint8_t *heap, T &object)
{
	static_assert(std::is_trivially_copyable<T>::value, "not trivially copyable");
	static_assert(!std::is_pointer<T>::value, "pointer is not copyable");
	memcpy(&object, heap, sizeof(object));
	return sizeof(object);
}

template<class T> void serialize_to_file(const std::filesystem::path &path, const T &object)
{
	auto size = serialize(nullptr, true, object);
	std::vector<uint8_t> heap(size);
	serialize(heap.data(), false, object);
	File file(path, "wb");
	file.write(heap.data(), heap.size());
}

template<class T> size_t deserialize_from_file(const std::filesystem::path &path, T &object)
{
	auto heap = read_from_file<std::vector<uint8_t>>(path);
	return deserialize(heap.data(), object);
}

template<class T0, class T1> size_t serialize(uint8_t *heap, bool is_dry, const std::pair<T0, T1> &object)
{
	auto *hp = heap;
	hp += serialize(hp, is_dry, object.first);
	hp += serialize(hp, is_dry, object.second);
	return hp - heap;
}

template<class T0, class T1> size_t deserialize(const uint8_t *heap, std::pair<T0, T1> &object)
{
	const auto *hp = heap;
	hp += deserialize(hp, object.first);
	hp += deserialize(hp, object.second);
	return hp - heap;
}

template<class T> size_t serialize(uint8_t *heap, bool is_dry, const std::vector<T> &objects)
{
	size_t n = objects.size();
	auto *hp = heap;
	hp += serialize(hp, is_dry, n);
	for (auto &object: objects) {
		hp += serialize(hp, is_dry, object);
	}
	return hp - heap;
}

template<class T> size_t deserialize(const uint8_t *heap, std::vector<T> &objects)
{
	size_t n;
	const auto *hp = heap;
	hp += deserialize(hp, n);

	objects.resize(n);
	for (auto &object: objects) {
		hp += deserialize(hp, object);
	}
	return hp - heap;
}

template<class T> size_t serialize(uint8_t *heap, bool is_dry, const std::vector<T> &objects, uint32_t first)
{
	auto *hp = heap;
	size_t count = objects.size() - first;
	hp += serialize(hp, is_dry, count);
	for (auto i = first; i < objects.size(); i++) {
		hp += serialize(hp, is_dry, objects[i]);
	}
	return hp - heap;
}

template<class T> size_t deserialize(const uint8_t *heap, std::vector<T> &objects, uint32_t first)
{
	const auto *hp = heap;
	size_t count;
	hp += deserialize(hp, count);
	objects.resize(first + count);
	for (auto i = first; i < objects.size(); i++) {
		hp += deserialize(hp, objects[i]);
	}
	return hp - heap;
}

template<class T0, class T1> size_t serialize(uint8_t *heap, bool is_dry, const std::map<T0, T1> &objects)
{
	size_t n = objects.size();
	auto *hp = heap;
	hp += serialize(hp, is_dry, n);

	for (auto &object: objects) {
		hp += serialize(hp, is_dry, object);
	}
	return hp - heap;
}

template<class T0, class T1> size_t deserialize(const uint8_t *heap, std::map<T0, T1> &objects)
{
	size_t n;
	const auto *hp = heap;
	hp += deserialize(hp, n);
	objects.clear();
	for (auto i = 0u; i < n; i++) {
		std::pair<T0, T1> object;
		hp += deserialize(hp, object);
		objects[object.first] = object.second;
	}
	return hp - heap;
}

template<> inline size_t serialize(uint8_t *heap, bool is_dry, const std::string &object)
{
	if (is_dry) {
		return object.length() + 1;
	}
	auto *hp = heap;
	const auto *str = object.c_str();
	while (*str) {
		*hp++ = *str++;
	}
	*hp++ = 0;
	return hp - heap;
}

template<> inline size_t deserialize(const uint8_t *heap, std::string &object)
{
	const auto *hp = heap;
	object.clear();
	while (*hp) {
		object += *hp++;
	}
	hp++;  // skip sentinel
	return hp - heap;
}

}  // namespace spu
