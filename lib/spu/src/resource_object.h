//
// ResourceObject :
//
#pragma once

#include "spu_object.h"

namespace spu::libspu::resource {
class Queue;
class ResourceObject {
public:
	ResourceObject(const char *path, const Attrs &attrs);

	void start();
	bool sync();

	uint32_t id() const { return m_id; }
	bool isSuccess() const { return m_isSuccess; }
	bool isBgRun() const { return m_isBgRun; }

	virtual int32_t get(const hash32_t &key, void *value) const = 0;
	static void dispose(ResourceObject *res);

protected:
	std::string m_path;
	Attrs m_attrs;
	uint32_t m_id = 0;

	bool m_isSuccess = true;
	bool m_reqBackground = true;
	virtual ~ResourceObject() = default;

private:
	bool m_isBgRun = false;
	bool m_reqDispose = false;

	friend class Queue;
	virtual void doBackground() = 0;
	virtual void doEpilogue() = 0;
};
}  // namespace spu::libspu::resource
