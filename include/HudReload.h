#pragma once

#include <cstdint>

// SkyHUD reads skyhud.txt once, when its HUD movie initialises at game start, and there is no
// documented way to make it read again. Measured 2026-09-05: hiding and showing the HUD menu does
// not recreate its movie (the menu is flagged always-open), a save load does not either, and
// forcing the engine to rebuild the HUD menu (clearing that flag, hide, show) did not move a
// widget and crashed the game on the second rebuild inside the quest-marker code. So this mod
// does not try: a saved change takes effect the next time the game starts, and the page and the
// on-screen markers say so.
namespace hudreload
{
	// Records that a change is waiting for the next game start. Always returns false: nothing is
	// reloaded live.
	bool Request();

	// How many saves this session are waiting for a restart.
	int PendingSaves();
}
