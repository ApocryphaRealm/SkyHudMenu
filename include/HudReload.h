#pragma once

// Making SkyHUD re-read skyhud.txt at runtime. SkyHUD reads its config only when the HUD movie
// initialises, so after writing skyhud.txt this asks the HUD to reload. There is no documented
// SkyHUD reload API, so this uses the engine's own path (recreate the HUD menu) and logs the
// outcome - it is the one part of this mod that must be proven in game.
namespace hudreload
{
	// Returns true if the reload request was dispatched (not a guarantee SkyHUD re-read).
	bool Request();
}
