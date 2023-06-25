#include "../core/math/vector.h"

struct RecordedAnimationLayer
{
	int unknown_0;
	int sequenceIdx;
	int unknown_8;
	int unknown_12;
	int unknown_16;
	int unknown_20;
	int unknown_24;
	int unknown_28;
	int unknown_32;
};

struct RecordedAnimationFrame
{
	int unknown_0; /* DAT_180bfbe08 at offset 16 */
	int unknown_4;
	int unknown_8;
	int unknown_12;
	int unknown_16;
	int unknown_20;
	int unknown_24;
	int unknown_28;
	int sequenceIdx;
	int unknown_36;
	int unknown_40;
	int unknown_44;
	int layerIdx;
	int unknown_52;
	char gap_56[11];
	bool unknown_67;
};

struct UnknownAnimData
{
	float unknown_0;
	float unknown_1;
};

struct RecordedAnimation
{
	char* sequences_1[15]; /* no idea what they're used for */
	UnknownAnimData unknown_120[15];
	char* sequences[47];
	long long unknown_616[17]; /* unsure about type */
	Vector3 origin;
	Vector3 angles;
	RecordedAnimationFrame* frames; /* value is 0x31ce0 bytes large */
	RecordedAnimationLayer* layers; /* value is 0x1a5e0 bytes large */
	int frameCount; /* max 3k frames; everything after will get truncated */
	int layerCount; /* max 3k layers; everything after will get truncated */
	long long loadedOffset_maybe; /* Looks like the index of this anim in the loaded
									 anims (g_LoadedRecordedAnimations_maybe) */
	int userdataOffset_maybe;
	bool notRefcounted_maybe; /* +0x32e only increases if this is false */
	char unknown_813;
	short refcount_maybe; /* increases when an animation is loaded */
};