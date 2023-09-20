#pragma once

constexpr int MAX_HULLS = 5;

#pragma pack(push, 1)
struct CAI_NodeLink
{
	short srcId;
	short destId;
	bool hulls[MAX_HULLS];
	char unk0;
	char unk1; // maps => unk0 on disk
	char unk2[5];
	int64_t flags;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CAI_NodeLinkDisk
{
	short srcId;
	short destId;
	char unk0;
	bool hulls[MAX_HULLS];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CAI_Node
{
	int index; // not present on disk
	float x;
	float y;
	float z;
	float hulls[MAX_HULLS];
	float yaw;

	int unk0; // always 2 in buildainfile, maps directly to unk0 in disk struct
	int unk1; // maps directly to unk1 in disk struct
	int unk2[MAX_HULLS]; // maps directly to unk2 in disk struct, despite being ints rather than shorts

	// view server.dll+393672 for context and death wish
	char unk3[MAX_HULLS]; // hell on earth, should map to unk3 on disk
	char pad[3]; // aligns next bytes
	float unk4[MAX_HULLS]; // i have no fucking clue, calculated using some kind of demon hell function float magic

	CAI_NodeLink** links;
	char unk5[16];
	int linkcount;
	int unk11; // bad name lmao
	short unk6; // should match up to unk4 on disk
	char unk7[16]; // padding until next bit
	short unk8; // should match up to unk5 on disk
	char unk9[8]; // padding until next bit
	char unk10[8]; // should match up to unk6 on disk
};
#pragma pack(pop)

// the way CAI_Nodes are represented in on-disk ain files
#pragma pack(push, 1)
struct CAI_NodeDisk
{
	float x;
	float y;
	float z;
	float yaw;
	float hulls[MAX_HULLS];

	char unk0;
	int unk1;
	short unk2[MAX_HULLS];
	char unk3[MAX_HULLS];
	short unk4;
	short unk5;
	char unk6[8];
}; // total size of 68 bytes
#pragma pack(pop)

#pragma pack(push, 1)
struct UnkNodeStruct0
{
	int index;
	char unk0;
	char unk1; // maps to unk1 on disk
	char pad0[2]; // padding to +8

	float x;
	float y;
	float z;

	char pad5[4];
	int* unk2; // maps to unk5 on disk;
	char pad1[16]; // pad to +48
	int unkcount0; // maps to unkcount0 on disk

	char pad2[4]; // pad to +56
	int* unk3;
	char pad3[16]; // pad to +80
	int unkcount1;

	char pad4[132];
	char unk5;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct UnkLinkStruct1
{
	short unk0;
	short unk1;
	int unk2;
	char unk3;
	char unk4;
	char unk5;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CAI_ScriptNode
{
	float x;
	float y;
	float z;
	uint64_t scriptdata;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CAI_Network
{
	// +0
	char unk0[8];
	// +8
	int linkcount; // this is uninitialised and never set on ain build, fun!
	// +12
	char unk1[124];
	// +136
	int zonecount;
	// +140
	char unk2[16];
	// +156
	int unk5; // unk8 on disk
	// +160
	char unk6[4];
	// +164
	int hintcount;
	// +168
	short hints[2000]; // these probably aren't actually hints, but there's 1 of them per hint so idk
	// +4168
	int scriptnodecount;
	// +4172
	CAI_ScriptNode scriptnodes[4000];
	// +84172
	int nodecount;
	// +84176
	CAI_Node** nodes;
};
#pragma pack(pop)
