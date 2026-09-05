#pragma once

// The "skyhud.config" DevBench driving tool (CLAUDE.md rule 31): read the loaded skyhud.txt,
// set a value, save+apply, and force a HUD reload - so the config->write->HUD-reload path can be
// exercised headlessly, which is exactly the part of this mod that has to be proven in game.
namespace DevBenchTool
{
	void Init(bool a_lastAttempt = false);
}
