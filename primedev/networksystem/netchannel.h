#pragma once

typedef enum
{
	NA_NULL = 0,
	NA_LOOPBACK,
	NA_IP,
} netadrtype_t;

#pragma pack(push, 1)
typedef struct netadr_s
{
	netadrtype_t type;
	unsigned char ip[16]; // IPv6
	// IPv4's 127.0.0.1 is [::ffff:127.0.0.1], that is:
	// 00 00 00 00 00 00 00 00    00 00 FF FF 7F 00 00 01
	unsigned short port;
} netadr_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct netpacket_s
{
	netadr_t adr; // sender address
	// int				source;		// received source
	char unk[10];
	double received_time;
	unsigned char* data; // pointer to raw packet data
	void* message; // easy bitbuf data access // 'inpacket.message' etc etc (pointer)
	char unk2[16];
	int size;

	// bf_read			message;	// easy bitbuf data access // 'inpacket.message' etc etc (pointer)
	// int				size;		// size in bytes
	// int				wiresize;   // size in bytes before decompression
	// bool			stream;		// was send as stream
	// struct netpacket_s* pNext;	// for internal use, should be NULL in public
} netpacket_t;
#pragma pack(pop)
