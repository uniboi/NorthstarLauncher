#include <winsock.h>

// FIXME [Fifty]: wsock implements these as wrappers with extra logic, although this works we
//                should recreate it

int __stdcall PROXY_getsockopt(SOCKET s, int level, int optname, char* optval, int* optlen)
{
	return getsockopt(s, level, optname, optval, optlen);
}

int __stdcall PROXY_recv(SOCKET s, char* buf, int len, int flags)
{
	return recv(s, buf, len, flags);
}

int __stdcall PROXY_recvfrom(SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen)
{
	return recvfrom(s, buf, len, flags, from, fromlen);
}

int __stdcall PROXY_setsockopt(SOCKET s, int level, int optname, const char* optval, int optlen)
{
	return setsockopt(s, level, optname, optval, optlen);
}
