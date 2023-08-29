#pragma once
template <typename ReturnType, typename... Args>
ReturnType CallVFunc(int index, void* thisPtr, Args... args)
{
	return (*reinterpret_cast<ReturnType(__fastcall***)(void*, Args...)>(thisPtr))[index](thisPtr, args...);
}

template <typename ReturnType, typename... Args>
ReturnType CallVFunc_Self(int index, void* pVtable, void* pThis, Args... args)
{
	return (reinterpret_cast<ReturnType(__fastcall**)(void*, Args...)>(pVtable))[index](pThis, args...);
}

template <typename T, size_t index, typename... Args>
constexpr T CallVFunc_Alt(void* classBase, Args... args) noexcept
{
	return ((*(T(__thiscall***)(void*, Args...))(classBase))[index])(classBase, args...);
}

#define STR_HASH(s) (std::hash<std::string>()(s))
