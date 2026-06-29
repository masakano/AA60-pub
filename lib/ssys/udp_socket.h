//
// UdpSocket :
//
#pragma once
#include "ssys.h"
#include <netinet/in.h>  // sockaddr_in

namespace spu {

class UdpSocket {
public:
	struct AddrAndPort {
		std::string addr;
		int32_t port;
	};

	typedef void (*errhandler_t)();
	static constexpr uint32_t c_maxsize = 65507;

	UdpSocket() = default;
	explicit UdpSocket(int32_t port) { init(port); }
	~UdpSocket() { close(); }

	void init(int32_t port);
	void close();

	ssize_t sendto(const struct sockaddr_in &addr, const void *buf, size_t len);
	ssize_t recvfrom(struct sockaddr_in &addr, void *buf, size_t len);

	void extend(uint32_t send_extend_byte, uint32_t recv_extend_byte);
	void timeout(uint32_t waitmsec);
	int32_t sock() const { return m_sock; }

	static struct sockaddr_in make_sockaddr_in(const AddrAndPort &addr_and_port);
	static AddrAndPort parse(const char *desc);
	static errhandler_t get_errhandler();
	static void set_errhandler(errhandler_t func);

private:
	int32_t m_sock = 0;
	inline static void (*ms_errhandler)() = nullptr;

	void aux_perror(bool cond, const char *str)
	{
		if (cond) {
			if (ms_errhandler) ms_errhandler();
			aux_error(true, "%s: %s\n", str, strerror(errno));
		}
	}
};
}  // namespace spu
