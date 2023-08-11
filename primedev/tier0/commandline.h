#pragma once

#include "icommandline.h"

class CCommandLine : public ICommandLine
{
	// 
};

inline CCommandLine* (*CommandLine)();
