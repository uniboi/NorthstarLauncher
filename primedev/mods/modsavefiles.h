#pragma once

#include <sstream>
#include <fstream>
#include "mods/modmanager.h"
#include "tier0/taskscheduler.h"

inline int MAX_FOLDER_SIZE = 52428800; // 50MB (50 * 1024 * 1024)
inline fs::path g_svSavePath;

int GetMaxSaveFolderSize();
bool ContainsInvalidChars(std::string str);
bool IsPathSafe(const std::string param, fs::path dir);

uintmax_t GetSizeOfFolder(fs::path dir);
uintmax_t GetSizeOfFolderContentsMinusFile(fs::path dir, std::string file);

class SaveFileManager
{
  public:
	// Saves a file asynchronously.
	void SaveFileAsync(fs::path file, std::string contents)
	{
		auto mutex = std::ref(fileMutex);
		std::thread writeThread(
			[mutex, file, contents]()
			{
				try
				{
					mutex.get().lock();

					fs::path dir = file.parent_path();
					// this actually allows mods to go over the limit, but not by much
					// the limit is to prevent mods from taking gigabytes of space,
					// we don't need to be particularly strict.
					if (GetSizeOfFolderContentsMinusFile(dir, file.filename().string()) + contents.length() > MAX_FOLDER_SIZE)
					{
						// tbh, you're either trying to fill the hard drive or use so much data, you SHOULD be congratulated.
						Error(eLog::MODSYS, NO_ERROR, "Mod spamming save requests? Folder limit bypassed despite previous checks. Not saving.\n");
						mutex.get().unlock();
						return;
					}

					std::ofstream fileStr(file);
					if (fileStr.fail())
					{
						mutex.get().unlock();
						return;
					}

					fileStr.write(contents.c_str(), contents.length());
					fileStr.close();

					mutex.get().unlock();
					// side-note: this causes a leak?
					// when a file is added to the map, it's never removed.
					// no idea how to fix this - because we have no way to check if there are other threads waiting to use this file(?)
					// tried to use m.try_lock(), but it's unreliable, it seems.
				}
				catch (std::exception ex)
				{
					Error(eLog::MODSYS, NO_ERROR, "SAVE FAILED!\n");
					mutex.get().unlock();
					Error(eLog::MODSYS, NO_ERROR, "%s\n", ex.what());
				}
			});

		writeThread.detach();
	}

	// Loads a file asynchronously.
	int LoadFileAsync(fs::path file, ScriptContext nContext)
	{
		int handle = ++m_iLastRequestHandle;
		auto mutex = std::ref(fileMutex);
		std::thread readThread(
			[mutex, file, handle, nContext]()
			{
				try
				{
					mutex.get().lock();

					std::ifstream fileStr(file);
					if (fileStr.fail())
					{
						Error(eLog::MODSYS, NO_ERROR, "A file was supposed to be loaded but we can't access it?!\n");

						g_pTaskScheduler->AddTask(
							[handle, nContext]()
							{
								CSquirrelVM* pVM = nullptr;
								if (nContext == ScriptContext::SERVER)
									pVM = g_pServerVM;
								else if (nContext == ScriptContext::CLIENT)
									pVM = g_pClientVM;
								else if (nContext == ScriptContext::UI)
									pVM = g_pUIVM;

								if (pVM && pVM->GetVM())
								{
									ScriptContext nContext = (ScriptContext)pVM->vmContext;
									HSQUIRRELVM hVM = pVM->GetVM();
									const char* pszFuncName = "NSHandleLoadResult";

									SQObject oFunction {};
									int nResult = sq_getfunction(hVM, pszFuncName, &oFunction, 0);
									if (nResult != 0)
									{
										Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", pszFuncName);
										return;
									}

									// Push
									sq_pushobject(hVM, &oFunction);
									sq_pushroottable(hVM);

									sq_pushinteger(hVM, handle);
									sq_pushbool(hVM, false);
									sq_pushstring(hVM, "", -1);

									(void)sq_call(hVM, 4, false, false);
								}
							});
						//g_pSquirrel<context>->AsyncCall("NSHandleLoadResult", handle, false, "");
						mutex.get().unlock();
						return;
					}

					std::stringstream stringStream;
					stringStream << fileStr.rdbuf();

					std::string svContents = stringStream.str();
					g_pTaskScheduler->AddTask(
						[handle, svContents, nContext]()
						{
							CSquirrelVM* pVM = nullptr;
							if (nContext == ScriptContext::SERVER)
								pVM = g_pServerVM;
							else if (nContext == ScriptContext::CLIENT)
								pVM = g_pClientVM;
							else if (nContext == ScriptContext::UI)
								pVM = g_pUIVM;

							if (pVM && pVM->GetVM())
							{
								ScriptContext nContext = (ScriptContext)pVM->vmContext;
								HSQUIRRELVM hVM = pVM->GetVM();
								const char* pszFuncName = "NSHandleLoadResult";

								SQObject oFunction {};
								int nResult = sq_getfunction(hVM, pszFuncName, &oFunction, 0);
								if (nResult != 0)
								{
									Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", pszFuncName);
									return;
								}

								// Push
								sq_pushobject(hVM, &oFunction);
								sq_pushroottable(hVM);

								sq_pushinteger(hVM, handle);
								sq_pushbool(hVM, true);
								sq_pushstring(hVM, svContents.c_str(), -1);

								(void)sq_call(hVM, 4, false, false);
							}
						});
					//g_pSquirrel<context>->AsyncCall("NSHandleLoadResult", handle, true, stringStream.str());

					fileStr.close();
					mutex.get().unlock();
					// side-note: this causes a leak?
					// when a file is added to the map, it's never removed.
					// no idea how to fix this - because we have no way to check if there are other threads waiting to use this file(?)
					// tried to use m.try_lock(), but it's unreliable, it seems.
				}
				catch (std::exception ex)
				{
					Error(eLog::MODSYS, NO_ERROR, "LOAD FAILED!\n");
					g_pTaskScheduler->AddTask(
						[handle, nContext]()
						{
							CSquirrelVM* pVM = nullptr;
							if (nContext == ScriptContext::SERVER)
								pVM = g_pServerVM;
							else if (nContext == ScriptContext::CLIENT)
								pVM = g_pClientVM;
							else if (nContext == ScriptContext::UI)
								pVM = g_pUIVM;

							if (pVM && pVM->GetVM())
							{
								ScriptContext nContext = (ScriptContext)pVM->vmContext;
								HSQUIRRELVM hVM = pVM->GetVM();
								const char* pszFuncName = "NSHandleLoadResult";

								SQObject oFunction {};
								int nResult = sq_getfunction(hVM, pszFuncName, &oFunction, 0);
								if (nResult != 0)
								{
									Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", pszFuncName);
									return;
								}

								// Push
								sq_pushobject(hVM, &oFunction);
								sq_pushroottable(hVM);

								sq_pushinteger(hVM, handle);
								sq_pushbool(hVM, false);
								sq_pushstring(hVM, "", -1);

								(void)sq_call(hVM, 4, false, false);
							}
						});
					//g_pSquirrel<context>->AsyncCall("NSHandleLoadResult", handle, false, "");
					mutex.get().unlock();
					Error(eLog::MODSYS, NO_ERROR, "%s\n", ex.what());
				}
			});

		readThread.detach();
		return handle;
	}

	// Deletes a file asynchronously.
	void DeleteFileAsync(fs::path file)
	{
		// P.S. I don't like how we have to async delete calls but we do.
		auto m = std::ref(fileMutex);
		std::thread deleteThread(
			[m, file]()
			{
				try
				{
					m.get().lock();

					fs::remove(file);

					m.get().unlock();
					// side-note: this causes a leak?
					// when a file is added to the map, it's never removed.
					// no idea how to fix this - because we have no way to check if there are other threads waiting to use this file(?)
					// tried to use m.try_lock(), but it's unreliable, it seems.
				}
				catch (std::exception ex)
				{
					Error(eLog::MODSYS, NO_ERROR, "DELETE FAILED!\n");
					m.get().unlock();
					Error(eLog::MODSYS, NO_ERROR, "%s\n", ex.what());
				}
			});

		deleteThread.detach();
	}
	// Future proofed in that if we ever get multi-threaded SSDs this code will take advantage of them.
	std::mutex fileMutex;

  private:
	int m_iLastRequestHandle = 0;
};

inline SaveFileManager* g_pSaveFileManager;
