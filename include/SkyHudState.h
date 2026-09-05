#pragma once

#include "SkyHudConfig.h"

// The single in-memory skyhud.txt the page edits. Loaded when the plugin starts and whenever the
// page is opened (so external edits are picked up), written and applied on Save.
namespace state
{
	skyhud::Config& Config();

	// (Re)load skyhud.txt from disk into the shared Config.
	bool LoadFromDisk();

	// Write the shared Config back to skyhud.txt and ask SkyHUD to re-read it. Returns whether
	// the write succeeded (the reload is best-effort and logged).
	bool WriteAndApply();
}
