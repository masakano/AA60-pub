//
// UdpSocket :
//
#include <ssys/udp_socket.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace spu {

UdpSocket::errhandler_t UdpSocket::get_errhandler() { return ms_errhandler; }
void UdpSocket::set_errhandler(errhandler_t func) { ms_errhandler = func; }

void UdpSocket::init(int32_t port)
{
	assert(m_sock == 0);
	m_sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (port) {
		struct sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;

		auto ret = ::bind(m_sock, (struct sockaddr *)&addr, sizeof(addr));
		aux_perror(ret, "bind");
	}
}

void UdpSocket::close()
{
	if (m_sock) {
		::close(m_sock);
		m_sock = 0;
	}
}

ssize_t UdpSocket::sendto(const struct sockaddr_in &addr, const void *buf, size_t len)
{
	assert(m_sock);
	return ::sendto(m_sock, buf, len, 0, (const struct sockaddr *)&addr, sizeof(addr));
}

ssize_t UdpSocket::recvfrom(struct sockaddr_in &addr, void *buf, size_t len)
{
	assert(m_sock);
	socklen_t addrlen = sizeof(addr);
	return ::recvfrom(m_sock, buf, len, 0, (struct sockaddr *)&addr, &addrlen);
}

void UdpSocket::extend(uint32_t send_extend_byte, uint32_t recv_extend_byte)
{
	assert(m_sock);
	if (send_extend_byte) {
		auto ret = setsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, &send_extend_byte, sizeof(int));
		aux_perror(ret, "setsockopt");
	}
	if (recv_extend_byte) {
		auto ret = setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, &recv_extend_byte, sizeof(int));
		aux_perror(ret, "setsockopt");
	}
}

void UdpSocket::timeout(uint32_t waitmsec)
{
	assert(m_sock);
	struct timeval tv;
	tv.tv_sec = std::max(1u, waitmsec / 1000);
	tv.tv_usec = 0;
	auto ret = setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&tv), sizeof(tv));
	aux_perror(ret, "setsockopt");
}

struct sockaddr_in UdpSocket::make_sockaddr_in(const AddrAndPort &addr_and_port)
{
	struct sockaddr_in sockaddr;

	const auto *addr = addr_and_port.addr.c_str();
	const auto port = addr_and_port.port;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	if (addr && *addr) {
		inet_pton(AF_INET, addr, &sockaddr.sin_addr.s_addr);
	}
	else {
		sockaddr.sin_addr.s_addr = INADDR_ANY;
	}
	return sockaddr;
}

UdpSocket::AddrAndPort UdpSocket::parse(const char *desc)
{
	auto list = extract_from_string(desc, ":");
	aux_error(list.size() != 2, "\"%s\" : bad address (must be 'IPv4:port'\n", desc);

	return AddrAndPort{
	        list[0],
	        std::stoi(list[1]),
	};
	// return addr_and_port;
}
}  // namespace spu
