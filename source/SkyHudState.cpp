#include "SkyHudState.h"

#include "HudReload.h"
#include "utils/Logger.h"

namespace state
{
	skyhud::Config& Config()
	{
		static skyhud::Config g_config;
		return g_config;
	}

	bool LoadFromDisk()
	{
		return Config().Load();
	}

	bool WriteAndApply()
	{
		if (!Config().Save()) {
			return false;
		}
		hudreload::Request();
		return true;
	}
}
