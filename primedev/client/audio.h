#pragma once

#include <vector>
#include <filesystem>
#include <regex>
#include <shared_mutex>

// Empty stereo 48000 WAVE file
inline unsigned char EMPTY_WAVE[45] = {0x52, 0x49, 0x46, 0x46, 0x25, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02,
								0x00, 0x44, 0xAC, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00, 0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x74, 0x00, 0x00, 0x00, 0x00};

enum class AudioSelectionStrategy
{
	INVALID = -1,
	SEQUENTIAL,
	RANDOM
};

class EventOverrideData
{
  public:
	EventOverrideData(const std::string&, const fs::path&);
	EventOverrideData();

  public:
	bool LoadedSuccessfully = false;

	std::vector<std::string> EventIds = {};
	std::vector<std::pair<std::string, std::regex>> EventIdsRegex = {};

	std::vector<std::pair<size_t, std::unique_ptr<uint8_t[]>>> Samples = {};

	AudioSelectionStrategy Strategy = AudioSelectionStrategy::SEQUENTIAL;
	size_t CurrentIndex = 0;

	bool EnableOnLoopedSounds = false;
};

class CustomAudioManager
{
  public:
	bool TryLoadAudioOverride(const fs::path&);
	void ClearAudioOverrides();

	std::shared_mutex m_loadingMutex;
	std::unordered_map<std::string, std::shared_ptr<EventOverrideData>> m_loadedAudioOverrides = {};
	std::unordered_map<std::string, std::shared_ptr<EventOverrideData>> m_loadedAudioOverridesRegex = {};
};

extern CustomAudioManager g_CustomAudioManager;
