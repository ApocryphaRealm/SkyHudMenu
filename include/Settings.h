#pragma once

#include "utils/Logger.h"

#include <string>
#include <vector>

// This mod's OWN settings (SkyHudMenu.ini): diagnostics + the on-screen ghost preview prefs. The
// HUD settings it edits live in skyhud.txt (skyhud::Config), not here.
namespace settings
{
	void Init(const std::string& a_iniFileName);
	bool Save();
	void Reload();
	const std::string& GetIniPath();

	namespace debug
	{
		inline logger::level logLevel = logger::level::trace;  // ships at trace (uLogLevel=0)
	}

	namespace preview
	{
		inline bool show = true;  // master on/off for the whole ghost overlay (bShowPreview:Preview)

		// Per-HUD-element ghost preference: whether its ghost shows and its palette colour index.
		// Indexed in the same order as skyhud::Elements(); sized and defaulted on Init (each
		// element gets a different default colour so they are distinguishable out of the box).
		struct GhostPref
		{
			bool show = true;
			int  color = 0;
		};

		std::vector<GhostPref>& Ghosts();
	}
}
