#include "logging.h"
#include "loghooks.h"
#include "tier1/convar.h"
#include "tier1/cmd.h"
#include "mathlib/bitbuf.h"

enum class TextMsgPrintType_t
{
	HUD_PRINTNOTIFY = 1,
	HUD_PRINTCONSOLE,
	HUD_PRINTTALK,
	HUD_PRINTCENTER
};

class ICenterPrint
{
  public:
	virtual void ctor() = 0;
	virtual void Clear(void) = 0;
	virtual void ColorPrint(int r, int g, int b, int a, wchar_t* text) = 0;
	virtual void ColorPrint(int r, int g, int b, int a, char* text) = 0;
	virtual void Print(wchar_t* text) = 0;
	virtual void Print(char* text) = 0;
	virtual void SetTextColor(int r, int g, int b, int a) = 0;
};

enum class SpewType_t
{
	SPEW_MESSAGE = 0,

	SPEW_WARNING,
	SPEW_ASSERT,
	SPEW_ERROR,
	SPEW_LOG,

	SPEW_TYPE_COUNT
};
ICenterPrint* pInternalCenterPrint = NULL;

void (*o_TextMsg)(BFRead* msg);

void h_TextMsg(BFRead* msg)
{
	TextMsgPrintType_t msg_dest = (TextMsgPrintType_t)msg->ReadByte();

	char text[256];
	msg->ReadString(text, sizeof(text));

	if (!Cvar_cl_showtextmsg->GetBool())
		return;

	switch (msg_dest)
	{
	case TextMsgPrintType_t::HUD_PRINTCENTER:
		pInternalCenterPrint->Print(text);
		break;

	default:
		Warning(eLog::CLIENT, "Unimplemented TextMsg type %i! printing to console\n", msg_dest);
		[[fallthrough]];

	case TextMsgPrintType_t::HUD_PRINTCONSOLE:
		auto endpos = strlen(text);
		if (text[endpos - 1] == '\n')
			text[endpos - 1] = '\0'; // cut off repeated newline

		DevMsg(eLog::CLIENT, "%s\n", text);
		break;
	}
}

int (*o_fpritnf)(void* const stream, const char* const fmt, ...);

int h_fprintf(void* const stream, const char* const format, ...)
{
	NOTE_UNUSED(stream);

	va_list va;
	va_start(va, format);

	char buf[1024];
	int charsWritten = vsnprintf_s(buf, _TRUNCATE, format, va);

	if (charsWritten > 0)
	{
		if (buf[charsWritten - 1] == '\n')
			buf[charsWritten - 1] = '\0';
		DevMsg(eLog::ENGINE, "%s\n", buf);
	}

	va_end(va);
	return 0;
}

void (*o_ConCommand_echo)(const CCommand& arg);

void h_ConCommand_echo(const CCommand& arg)
{
	if (arg.ArgC() >= 2)
		DevMsg(eLog::ENGINE, "%s\n", arg.ArgS());
}

void (*o_CVEngineServer__Spew)(void* self, SpewType_t type, const char* fmt, va_list va);

void h_CVEngineServer__Spew(void* self, SpewType_t type, const char* format, va_list args)
{
	NOTE_UNUSED(self);
	if (!Cvar_spewlog_enable->GetBool())
		return;

	char formatted[2048] = {0};
	bool bShouldFormat = true;

	// because titanfall 2 is quite possibly the worst thing to yet exist, it sometimes gives invalid specifiers which will crash
	// ttf2sdk had a way to prevent them from crashing but it doesnt work in debug builds
	// so we use this instead
	for (int i = 0; format[i]; i++)
	{
		if (format[i] == '%')
		{
			switch (format[i + 1])
			{
			// this is fucking awful lol
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'f':
			case 'F':
			case 'g':
			case 'G':
			case 'a':
			case 'A':
			case 'c':
			case 's':
			case 'p':
			case 'n':
			case '%':
			case '-':
			case '+':
			case ' ':
			case '#':
			case '*':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				break;

			default:
			{
				bShouldFormat = false;
				break;
			}
			}
		}
	}

	if (bShouldFormat)
		vsnprintf(formatted, sizeof(formatted), format, args);
	else
		Warning(eLog::NS, "Failed to format spew message \"%s\"\n", format);

	auto endpos = strlen(formatted);
	if (formatted[endpos - 1] == '\n')
		formatted[endpos - 1] = '\0'; // cut off repeated newline

	switch (type)
	{
	case SpewType_t::SPEW_MESSAGE:
	case SpewType_t::SPEW_LOG:
		DevMsg(eLog::ENGINE, "%s\n", formatted);
		break;
	case SpewType_t::SPEW_WARNING:
		Warning(eLog::ENGINE, "%s\n", formatted);
		break;
	case SpewType_t::SPEW_ASSERT:
	case SpewType_t::SPEW_ERROR:
		Error(eLog::ENGINE, NO_ERROR, "%s\n", formatted);
		break;
	}
}

// used for printing the output of status
void (*o_Status_ConMsg)(const char* text, ...);

void h_Status_ConMsg(const char* text, ...)
{
	char formatted[2048];
	va_list list;

	va_start(list, text);
	vsprintf_s(formatted, text, list);
	va_end(list);

	size_t endpos = strlen(formatted);
	if (formatted[endpos - 1] == '\n')
		formatted[endpos - 1] = '\0'; // cut off repeated newline

	DevMsg(eLog::ENGINE, "%s\n", formatted);
}

bool (*o_CClientState__ProcessPrint)(void* self, uintptr_t msg);

bool h_CClientState__ProcessPrint(void* self, uintptr_t msg)
{
	NOTE_UNUSED(self);
	char* text = *(char**)(msg + 0x20);

	size_t endpos = strlen(text);
	if (text[endpos - 1] == '\n')
		text[endpos - 1] = '\0'; // cut off repeated newline

	DevMsg(eLog::ENGINE, "%s\n", text);
	return true;
}

ON_DLL_LOAD("engine.dll", EngineSpewFuncHooks, (CModule module))
{
	o_fpritnf = module.Offset(0x51B1F0).RCast<int (*)(void* const, const char* const, ...)>();
	HookAttach(&(PVOID&)o_fpritnf, (PVOID)h_fprintf);

	o_ConCommand_echo = module.Offset(0x123680).RCast<void (*)(const CCommand&)>();
	HookAttach(&(PVOID&)o_ConCommand_echo, (PVOID)h_ConCommand_echo);

	o_CVEngineServer__Spew = module.Offset(0x11CA80).RCast<void (*)(void*, SpewType_t, const char*, va_list)>();
	HookAttach(&(PVOID&)o_CVEngineServer__Spew, (PVOID)h_CVEngineServer__Spew);

	o_Status_ConMsg = module.Offset(0x15ABD0).RCast<void (*)(const char*, ...)>();
	HookAttach(&(PVOID&)o_Status_ConMsg, (PVOID)h_Status_ConMsg);

	o_CClientState__ProcessPrint = module.Offset(0x1A1530).RCast<bool (*)(void*, uintptr_t)>();
	HookAttach(&(PVOID&)o_CClientState__ProcessPrint, (PVOID)h_CClientState__ProcessPrint);
}

ON_DLL_LOAD_CLIENT("client.dll", ClientPrintHooks, (CModule module))
{
	o_TextMsg = module.Offset(0x198710).RCast<void (*)(BFRead*)>();
	HookAttach(&(PVOID&)o_TextMsg, (PVOID)h_TextMsg);

	pInternalCenterPrint = module.Offset(0x216E940).RCast<ICenterPrint*>();
}
