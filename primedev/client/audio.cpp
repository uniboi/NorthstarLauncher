#include "audio.h"

#include "codecs/miles/core.h"

#include <fstream>
#include <iostream>
#include <sstream>

CustomAudioManager g_CustomAudioManager;

EventOverrideData::EventOverrideData()
{
	Warning(eLog::AUDIO, "Initialised struct EventOverrideData without any data!\n");
	LoadedSuccessfully = false;
}

EventOverrideData::EventOverrideData(const std::string& data, const fs::path& path)
{
	if (data.length() <= 0)
	{
		Error(eLog::AUDIO, NO_ERROR, "Failed reading audio override file %s: file is empty\n", path.string().c_str());
		return;
	}

	fs::path fsSamplesFolder = path;
	fsSamplesFolder = fsSamplesFolder.replace_extension();

	if (!FileExists(fsSamplesFolder))
	{
		Error(eLog::AUDIO, NO_ERROR,
			  "Failed reading audio override file %s: samples folder doesn't exist; should be named the same as the definition file without "
			  "JSON extension.\n",
			  path.string().c_str());
		return;
	}

	nlohmann::json jsData;
	try
	{
		jsData = nlohmann::json::parse(data);
	}
	catch (const std::exception& ex)
	{
		Error(eLog::AUDIO, NO_ERROR, "Failed reading audio override file %s: %s\n", path.string().c_str(), ex.what());
	}

	// fail if it's not a json obj (could be an array, string, etc)
	if (!jsData.is_object())
	{
		Error(eLog::AUDIO, NO_ERROR, "Failed reading audio override file %s: file is not a JSON object\n", path.string().c_str());
		return;
	}

	// fail if no event ids given
	if (!jsData.contains("EventId"))
	{
		Error(eLog::AUDIO, NO_ERROR, "Failed reading audio override file %s: JSON object does not have the EventId property\n", path.string().c_str());
		return;
	}

	// array of event ids
	if (jsData["EventId"].is_array())
	{
		for (auto& eventId : jsData["EventId"])
		{
			if (!eventId.is_string())
			{
				Error(eLog::AUDIO, NO_ERROR, "Failed reading audio override file %s: EventId array has a value of invalid type, all must be strings\n", path.string().c_str());
				return;
			}

			EventIds.push_back(eventId.get<std::string>());
		}
	}
	// singular event id
	else if (jsData["EventId"].is_string())
	{
		EventIds.push_back(jsData["EventId"].get<std::string>());
	}
	// incorrect type
	else
	{
		Error(eLog::AUDIO, NO_ERROR, "Failed reading audio override file %s: EventId property is of invalid type (must be a string or an array of strings)\n", path.string().c_str());
		return;
	}

	if (jsData.contains("EventIdRegex"))
	{
		// array of event id regex
		if (jsData["EventIdRegex"].is_array())
		{
			for (auto& eventId : jsData["EventIdRegex"])
			{
				if (!eventId.is_string())
				{
					Error(eLog::AUDIO, NO_ERROR, "Failed reading audio override file %s: EventIdRegex array has a value of invalid type, all must be strings\n", path.string().c_str());
					return;
				}

				const std::string& regex = eventId.get<std::string>();

				try
				{
					EventIdsRegex.push_back({regex, std::regex(regex)});
				}
				catch (...)
				{
					Error(eLog::AUDIO, NO_ERROR, "Malformed regex \"%s\" in audio override file %s\n", regex.c_str(), path.string().c_str());
					return;
				}
			}
		}
		// singular event id regex
		else if (jsData["EventIdRegex"].is_string())
		{
			const std::string& regex = jsData["EventIdRegex"].get<std::string>();
			try
			{
				EventIdsRegex.push_back({regex, std::regex(regex)});
			}
			catch (...)
			{
				Error(eLog::AUDIO, NO_ERROR, "Malformed regex \"%s\" in audio override file %s\n", regex.c_str(), path.string().c_str());
				return;
			}
		}
		// incorrect type
		else
		{
			Error(eLog::AUDIO, NO_ERROR,
				  "Failed reading audio override file %s: EventIdRegex property is of invalid type (must be a string or an array of "
				  "strings)\n",
				  path.string().c_str());
			return;
		}
	}

	if (jsData.contains("AudioSelectionStrategy"))
	{
		if (!jsData["AudioSelectionStrategy"].is_string())
		{
			Error(eLog::AUDIO, NO_ERROR, "Failed reading audio override file %s: AudioSelectionStrategy property must be a string\n", path.string().c_str());
			return;
		}

		std::string strategy = jsData["AudioSelectionStrategy"].get<std::string>();

		if (strategy == "sequential")
		{
			Strategy = AudioSelectionStrategy::SEQUENTIAL;
		}
		else if (strategy == "random")
		{
			Strategy = AudioSelectionStrategy::RANDOM;
		}
		else
		{
			Error(eLog::AUDIO, NO_ERROR, "Failed reading audio override file %s: AudioSelectionStrategy string must be either \"sequential\" or \"random\"\n", path.string().c_str());
			return;
		}
	}

	// load samples
	for (fs::directory_entry file : fs::recursive_directory_iterator(fsSamplesFolder))
	{
		if (file.is_regular_file() && file.path().extension().string() == ".wav")
		{
			std::string pathString = file.path().string();

			// Open the file.
			std::ifstream wavStream(pathString, std::ios::binary);

			if (wavStream.fail())
			{
				Error(eLog::AUDIO, NO_ERROR, "Failed reading audio sample %s\n", file.path().string().c_str());
				continue;
			}

			// Get file size.
			wavStream.seekg(0, std::ios::end);
			size_t fileSize = wavStream.tellg();
			wavStream.close();

			// Allocate enough memory for the file.
			// blank out the memory for now, then read it later
			uint8_t* data = new uint8_t[fileSize];
			memcpy(data, EMPTY_WAVE, sizeof(EMPTY_WAVE));
			Samples.push_back({fileSize, std::unique_ptr<uint8_t[]>(data)});

			// thread off the file read
			// should we spawn one thread per read? or should there be a cap to the number of reads at once?
			std::thread readThread(
				[pathString, fileSize, data]
				{
					std::shared_lock lock(g_CustomAudioManager.m_loadingMutex);
					std::ifstream wavStream(pathString, std::ios::binary);

					// would be weird if this got hit, since it would've worked previously
					if (wavStream.fail())
					{
						Error(eLog::AUDIO, NO_ERROR, "Failed async read of audio sample %s\n", pathString.c_str());
						return;
					}

					// read from after the header first to preserve the empty header, then read the header last
					wavStream.seekg(0, std::ios::beg);
					wavStream.read(reinterpret_cast<char*>(data), fileSize);
					wavStream.close();

					DevMsg(eLog::AUDIO, "Finished async read of audio sample %s\n", pathString.c_str());
				});

			readThread.detach();
		}
	}

	if (Samples.size() == 0)
		Warning(eLog::AUDIO, "Audio override %s has no valid samples! Sounds will not play for this event.\n", path.string().c_str());

	DevMsg(eLog::AUDIO, "Loaded audio override file %s\n", path.string().c_str());

	LoadedSuccessfully = true;
}

bool CustomAudioManager::TryLoadAudioOverride(const fs::path& defPath)
{
	if (IsDedicatedServer())
		return true; // silently fail

	std::ifstream jsonStream(defPath);
	std::stringstream jsonStringStream;

	// fail if no audio json
	if (jsonStream.fail())
	{
		Warning(eLog::AUDIO, "Unable to read audio override from file %s\n", defPath.string().c_str());
		return false;
	}

	while (jsonStream.peek() != EOF)
		jsonStringStream << (char)jsonStream.get();

	jsonStream.close();

	std::shared_ptr<EventOverrideData> data = std::make_shared<EventOverrideData>(jsonStringStream.str(), defPath);

	if (!data->LoadedSuccessfully)
		return false; // no logging, the constructor has probably already logged

	for (const std::string& eventId : data->EventIds)
	{
		DevMsg(eLog::AUDIO, "Registering sound event %s\n", eventId.c_str());
		m_loadedAudioOverrides.insert({eventId, data});
	}

	for (const auto& eventIdRegexData : data->EventIdsRegex)
	{
		DevMsg(eLog::AUDIO, "Registering sound event regex %s\n", eventIdRegexData.first.c_str());
		m_loadedAudioOverridesRegex.insert({eventIdRegexData.first, data});
	}

	return true;
}

void CustomAudioManager::ClearAudioOverrides()
{
	if (IsDedicatedServer())
		return;

	if (m_loadedAudioOverrides.size() > 0 || m_loadedAudioOverridesRegex.size() > 0)
	{
		// stop all miles sounds beforehand
		// miles_stop_all

		MilesStopAll();

		// this is cancer but it works
		Sleep(50);
	}

	// slightly (very) bad
	// wait for all audio reads to complete so we don't kill preexisting audio buffers as we're writing to them
	std::unique_lock lock(m_loadingMutex);

	m_loadedAudioOverrides.clear();
	m_loadedAudioOverridesRegex.clear();
}
