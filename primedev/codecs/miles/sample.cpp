#include "client/audio.h"

#include <random>

const char* pszAudioEventName = nullptr;

template <typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g)
{
	std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
	std::advance(start, dis(g));
	return start;
}

template <typename Iter>
Iter select_randomly(Iter start, Iter end)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	return select_randomly(start, end, gen);
}

bool ShouldPlayAudioEvent(const char* eventName, const std::shared_ptr<EventOverrideData>& data)
{
	std::string eventNameString = eventName;
	std::string eventNameStringBlacklistEntry = ("!" + eventNameString);

	for (const std::string& name : data->EventIds)
	{
		if (name == eventNameStringBlacklistEntry)
			return false; // event blacklisted

		if (name == "*")
		{
			// check for bad sounds I guess?
			// really feel like this should be an option but whatever
			if (!!strstr(eventName, "_amb_") || !!strstr(eventName, "_emit_") || !!strstr(eventName, "amb_"))
				return false; // would play static noise, I hate this
		}
	}

	return true; // good to go
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool (*o_LoadSampleMetadata)(void* sample, void* audioBuffer, unsigned int audioBufferLength, int audioType);
// Only: mileswin64.dll + 0xf66d is caled with audioType being variable
bool h_LoadSampleMetadata(void* sample, void* audioBuffer, unsigned int audioBufferLength, int audioType)
{
	// Raw source, used for voice data only
	if (audioType == 0)
		return o_LoadSampleMetadata(sample, audioBuffer, audioBufferLength, audioType);

	const char* eventName = pszAudioEventName;

	if (Cvar_ns_print_played_sounds->GetInt() > 0)
		DevMsg(eLog::AUDIO, "Playing event %s\n", eventName);

	auto iter = g_CustomAudioManager.m_loadedAudioOverrides.find(eventName);
	std::shared_ptr<EventOverrideData> overrideData;

	if (iter == g_CustomAudioManager.m_loadedAudioOverrides.end())
	{
		// override for that specific event not found, try wildcard
		iter = g_CustomAudioManager.m_loadedAudioOverrides.find("*");

		if (iter == g_CustomAudioManager.m_loadedAudioOverrides.end())
		{
			// not found

			// try regex
			for (const auto& item : g_CustomAudioManager.m_loadedAudioOverridesRegex)
				for (const auto& regexData : item.second->EventIdsRegex)
					if (std::regex_search(eventName, regexData.second))
						overrideData = item.second;

			if (!overrideData)
				// not found either
				return o_LoadSampleMetadata(sample, audioBuffer, audioBufferLength, audioType);
			else
			{
				// cache found pattern to improve performance
				g_CustomAudioManager.m_loadedAudioOverrides[eventName] = overrideData;
			}
		}
		else
			overrideData = iter->second;
	}
	else
		overrideData = iter->second;

	if (!ShouldPlayAudioEvent(eventName, overrideData))
		return o_LoadSampleMetadata(sample, audioBuffer, audioBufferLength, audioType);

	void* data = 0;
	unsigned int dataLength = 0;

	if (overrideData->Samples.size() == 0)
	{
		// 0 samples, turn off this particular event.

		// using a dummy empty wave file
		data = EMPTY_WAVE;
		dataLength = sizeof(EMPTY_WAVE);
	}
	else
	{
		std::pair<size_t, std::unique_ptr<uint8_t[]>>* dat = NULL;

		switch (overrideData->Strategy)
		{
		case AudioSelectionStrategy::RANDOM:
			dat = &*select_randomly(overrideData->Samples.begin(), overrideData->Samples.end());
			break;
		case AudioSelectionStrategy::SEQUENTIAL:
		default:
			dat = &overrideData->Samples[overrideData->CurrentIndex++];
			if (overrideData->CurrentIndex >= overrideData->Samples.size())
				overrideData->CurrentIndex = 0; // reset back to the first sample entry
			break;
		}

		if (!dat)
			Warning(eLog::AUDIO, "Could not get sample data from override struct for event %s! Shouldn't happen\n", eventName);
		else
		{
			data = dat->second.get();
			dataLength = dat->first;
		}
	}

	if (!data)
	{
		Warning(eLog::AUDIO, "Could not fetch override sample data for event {}! Using original data instead.\n", eventName);
		return o_LoadSampleMetadata(sample, audioBuffer, audioBufferLength, audioType);
	}

	audioBuffer = data;
	audioBufferLength = dataLength;

	// most important change: set the sample class buffer so that the correct audio plays
	*(void**)((uintptr_t)sample + 0xE8) = audioBuffer;
	*(unsigned int*)((uintptr_t)sample + 0xF0) = audioBufferLength;

	// 64 - Auto-detect sample type
	bool res = o_LoadSampleMetadata(sample, audioBuffer, audioBufferLength, 64);
	if (!res)
		Error(eLog::AUDIO, NO_ERROR, "LoadSampleMetadata failed! The game will crash :(\n");

	return res;
}

// calls LoadSampleMetadata ( only caller where audiotype isnt 0 )
bool (*o_SetSource)(void* sample, void* audioBuffer, unsigned int audioBufferLength, int audioType);

bool h_SetSource(void* sample, void* audioBuffer, unsigned int audioBufferLength, int audioType)
{
	return o_SetSource(sample, audioBuffer, audioBufferLength, audioType);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
// Calls SetSource, looks like a queue func or smth
void* (*o_sub_1800294C0)(void* a1, void* a2);

void* h_sub_1800294C0(void* a1, void* a2)
{
	pszAudioEventName = reinterpret_cast<const char*>((*((__int64*)a2 + 6)));
	return o_sub_1800294C0(a1, a2);
}

ON_DLL_LOAD_CLIENT("mileswin64.dll", MilesHook, (CModule module))
{
	o_LoadSampleMetadata = module.Offset(0xF110).RCast<bool (*)(void*, void*, unsigned int, int)>();
	HookAttach(&(PVOID&)o_LoadSampleMetadata, (PVOID)h_LoadSampleMetadata);

	o_SetSource = module.Offset(0xF600).RCast<bool (*)(void*, void*, unsigned int, int)>();
	HookAttach(&(PVOID&)o_SetSource, (PVOID)h_SetSource);

	o_sub_1800294C0 = module.Offset(0x294C0).RCast<void* (*)(void*, void*)>();
	HookAttach(&(PVOID&)o_sub_1800294C0, (PVOID)h_sub_1800294C0);
}
