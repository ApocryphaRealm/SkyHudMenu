#include "HudReload.h"

#include "utils/Logger.h"

#include <RE/Skyrim.h>

namespace hudreload
{
	bool Request()
	{
		auto* ui = RE::UI::GetSingleton();
		auto* queue = RE::UIMessageQueue::GetSingleton();
		if (!ui || !queue) {
			logger::error("HUD reload: UI/UIMessageQueue singleton missing");
			return false;
		}
		if (!ui->IsMenuOpen(RE::HUDMenu::MENU_NAME)) {
			logger::warn("HUD reload: HUDMenu is not open; nothing to reload right now");
			return false;
		}
		// Recreate the HUD menu: hide, then show. Recreating the movie re-runs SkyHUD's SWF init,
		// which is what re-reads skyhud.txt. Logged so the in-game test can confirm the effect.
		queue->AddMessage(RE::HUDMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
		queue->AddMessage(RE::HUDMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
		logger::info("HUD reload: dispatched kHide+kShow to HUDMenu");
		return true;
	}
}
