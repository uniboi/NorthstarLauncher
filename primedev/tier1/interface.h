#pragma once

#include "core/memory.h"

#define CREATEINTERFACE_PROCNAME "CreateInterface"

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

CMemoryAddress Sys_GetFactoryPtr(const std::string& svModuleName, const std::string& svFactoryName);
