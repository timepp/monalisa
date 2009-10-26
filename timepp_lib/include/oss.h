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

// ��װsocketͨ�ţ����г�����Ϊ�쳣�׳�
// ���ܲ������쳣��iptvm_error
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
	// �ر�д������TCP��˳���ͷ�
	// ��������Զ˷���FIN�źţ�֮��Զ˵Ķ�����������0)
	void shutdown_write();
	void close();

	void setopt(int level, int optname, const void * val, socklen_t len);
	int  getopt(int level, int optname, void * val, socklen_t len);

	int send(const void * buf, size_t len);
	int sendto(const void * buf, size_t len, const struct sockaddr_storage& ss);
	// recv����֤���յ������ֽ�
	int recv(void * buf, size_t buf_len);
	int recvfrom(void * buf, size_t buf_len, sockaddr_storage * ss);
	// recvn��֤�յ��㹻���ֽ�(n��, ע��buf�������ĳ���Ҫ����n)
	int recvn(void * buf, size_t buf_len, int n);
	int sendn(const void * buf, int len);

	static void read_select(const std::vector<ossocket>& insocks, std::vector<ossocket> * rsocks, struct timeval * timeout);
	static sockaddr_storage getaddr(int af, int type, int protocol, const wchar_t * node, int port, bool passive);
	static void getname(const sockaddr_storage& ss, std::wstring * addr, int * port);
private:
	sock_t m_socket;
};