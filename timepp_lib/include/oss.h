/*
    OS Service Layer with exception 
    Author: Tongjunhui
	Created:2007.3.17
	BUPT
*/

#pragma once

#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

// 封装socket通信，所有出错都做为异常抛出
// 可能产生的异常：iptvm_error
class ossocket
{
public:
	typedef SOCKET sock_t;
	ossocket();
	ossocket(int af, int type, int protocol);
	ossocket(sock_t s);
	ossocket(const ossocket& sock);
	ossocket& operator = (const ossocket& sock);
	bool operator == (const ossocket& sock) const;
	virtual ~ossocket();
	operator sock_t();

	void create(int af, int type, int protocol);
	bool valid() const;
	void getname(std::wstring * addr, int *port);
	void getpeername(std::wstring * addr, int *port);

	void listen(int backlog);
	void bind(const sockaddr_storage& ss);
	ossocket accept(sockaddr_storage * ss);
	void connect(const sockaddr_storage& ss);
	// 关闭写，用于TCP的顺序释放
	// 作用是向对端发送FIN信号，之后对端的读操作将返回0)
	void shutdown_write();
	void close();

	void setopt(int level, int optname, const void * val, socklen_t len);
	int  getopt(int level, int optname, void * val, socklen_t len);

	int send(const void * buf, size_t len);
	int sendto(const void * buf, size_t len, const struct sockaddr_storage& ss);
	// recv不保证能收到多少字节
	int recv(void * buf, size_t buf_len);
	int recvfrom(void * buf, size_t buf_len, sockaddr_storage * ss);
	// recvn保证收到足够的字节(n个, 注意buf缓冲区的长度要大于n)
	int recvn(void * buf, size_t buf_len, int n);
	int sendn(const void * buf, int len);

	static void read_select(const std::vector<ossocket>& insocks, std::vector<ossocket> * rsocks, struct timeval * timeout);
	static sockaddr_storage getaddr(int af, int type, int protocol, const wchar_t * node, int port, bool passive);
	static void getname(const sockaddr_storage& ss, std::wstring * addr, int * port);
private:
	sock_t m_socket;
};