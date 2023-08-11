#include "config/profile.h"
#include <string>

std::string g_svProfileDir;

std::string GetNorthstarPrefix()
{
	return g_svProfileDir;
}
