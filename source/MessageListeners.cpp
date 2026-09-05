#include "DevBenchTool.h"
#include "HudPreview.h"
#include "SkyHudState.h"
#include "UI.h"
#include "utils/Logger.h"

#include <SKSE/SKSE.h>

void SKSEMessageListener(SKSE::MessagingInterface::Message* a_msg)
{
	if (!a_msg) {
		return;
	}
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kPostPostLoad:
		UI::Register();
		preview::Register();
		DevBenchTool::Init();
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		// skyhud.txt is a loose Data file; it is readable by now. Load it so the page and the
		// DevBench tool have it in hand, and make the last DevBench registration attempt.
		state::LoadFromDisk();
		DevBenchTool::Init(/* a_lastAttempt = */ true);
		break;
	default:
		break;
	}
}
