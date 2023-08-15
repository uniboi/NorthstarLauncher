#pragma once

#define CREATEINTERFACE_PROCNAME "CreateInterface"

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

CMemory Sys_GetFactoryPtr(const std::string& svModuleName, const std::string& svFactoryName);
