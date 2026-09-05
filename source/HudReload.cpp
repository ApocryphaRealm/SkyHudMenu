#include "HudReload.h"

#include "utils/Logger.h"

namespace hudreload
{
	namespace
	{
		int g_pending = 0;
	}

	int PendingSaves() { return g_pending; }

	bool Request()
	{
		++g_pending;
		logger::info("skyhud.txt saved ({} this session); SkyHUD reads it at game start, so the change shows after a restart", g_pending);
		return false;
	}
}
