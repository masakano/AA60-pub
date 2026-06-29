//
// SharedMemory :
//
#pragma once
#include "ssys.h"
#include <fcntl.h>
#include <sys/mman.h>

namespace spu {

class SharedMemory {
public:
	SharedMemory() = default;
	SharedMemory(const char *shm_name, int32_t size) { init(shm_name, size); }
	virtual ~SharedMemory() { shutdown(); }

	void init(const char *shm_name, int32_t size)
	{
		m_name = shm_name;
		m_size = size;
		m_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
		aux_perror(m_fd == -1, "shm_open failed");
		aux_perror(ftruncate(m_fd, m_size) == -1, "ftruncate init");

		m_ptr = mmap(nullptr, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
		aux_perror(m_ptr == MAP_FAILED, "mmap init");
	}

	void realloc(uint32_t size)
	{
		std::vector<uint8_t> buffer(m_size);
		memcpy(buffer.data(), m_ptr, m_size);

		aux_perror(munmap(m_ptr, m_size) == -1, "munmap");

		m_size = size;
		aux_perror(ftruncate(m_fd, m_size) == -1, "ftruncate resize");

		m_ptr = mmap(nullptr, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
		aux_perror(m_ptr == MAP_FAILED, "mmap resized");

		memcpy(m_ptr, buffer.data(), m_size);
	}

	void shutdown()
	{
		if (m_ptr) {
			munmap(m_ptr, m_size);
			m_ptr = nullptr;
		}
		if (m_fd) {
			close(m_fd);
			m_fd = 0;
		}
	}
	void unlink()
	{
		if (!m_name.empty()) {
			shm_unlink(m_name.c_str());
		}
	}
	void *ptr() const { return m_ptr; }
	const std::string &name() const { return m_name; }
	uint32_t size() const { return m_size; }

protected:
	std::string m_name;
	int32_t m_fd = 0;
	void *m_ptr = nullptr;
	uint32_t m_size = 0;

	void aux_perror(bool cond, const char *str)
	{
		if (cond) {
			aux_error(true, "%s: %s\n", str, strerror(errno));
		}
	}
};
}  // namespace spu
