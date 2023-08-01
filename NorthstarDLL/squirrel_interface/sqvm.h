#include "sqconfig.h"

struct HSquirrelVM
{
	struct VarArgs
	{
		VarArgs()
		{
			size = 0;
			base = 0;
		}
		unsigned short size;
		unsigned short base;
	};

	struct CallInfo
	{
		SQInstruction* _ip;
		SQObjectPtr* _literals;
		SQObjectPtr _closure;
		SQGenerator* _generator;
		SQInt32 _etraps;
		SQInt32 _prevstkbase;
		SQInt32 _prevtop;
		SQInt32 _target;
		SQInt32 _ncalls;
		SQBool _root;
		VarArgs _vargs;
	};

  SQInt32 uiRef;
  char gap_8[12];
  void *_toString;
  void *_roottable_pointer;
  void *pointer_28;
  CallInfo *ci;
  CallInfo *_callstack;
  SQInt32 _callsstacksize;
  SQInt32 _stackbase;
  SQObject *_stackOfCurrentFunction;
  SQSharedState *sharedState;
  void *pointer_58;
  void *pointer_60;
  SQInt32 _top;
  SQObject *_stack;
  char gap_78[8];
  SQObject *_vargvstack;
  char gap_88[8];
  SQObject temp_reg;
  char gapA0[8];
  void *pointer_A8;
  char gap_B0[8];
  SQObject _roottable_object;
  SQObject _lasterror;
  SQObject _errorHandler;
  SQUnsignedInteger field_E8;
  SQInt32 traps;
  char gap_F4[12];
  SQInt32 _nnativecalls;
  SQInt32 _suspended;
  SQInt32 _suspended_root;
  SQInt32 _callstacksize;
  SQInt32 _suspended_target;
  SQInt32 trapAmount;
  SQInt32 _suspend_varargs;
  SQInt32 unknown_field_11C;
  SQObject object_120;
};
