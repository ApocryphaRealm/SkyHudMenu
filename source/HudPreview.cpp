#include "HudPreview.h"

#include "HudElements.h"
#include "SKSEMenuFramework.h"
#include "Settings.h"
#include "SkyHudState.h"
#include "utils/Logger.h"

#include <string>

namespace preview
{
	namespace
	{
		constexpr float kStageW = 1280.0F;
		constexpr float kStageH = 720.0F;
		// Representative widget sizes in the HUD's 1280x720 stage space, so a ghost has the right
		// SHAPE (wide bar, small square, ...) even though we do not read the live SWF's exact
		// bounds (that needs the HUD-clip access of the standalone positioner). Name-matched;
		// a sensible default covers anything not listed.
		struct Size { float w, h; };
		Size SizeFor(const std::string& a_name)
		{
			if (a_name == "Health" || a_name == "Magicka" || a_name == "Stamina" || a_name == "Enemy health") { return { 200.0F, 14.0F }; }
			if (a_name == "Charge meters") { return { 120.0F, 12.0F }; }
			if (a_name == "Compass") { return { 480.0F, 36.0F }; }
			if (a_name == "Compass markers") { return { 20.0F, 28.0F }; }
			if (a_name == "Crosshair") { return { 32.0F, 32.0F }; }
			if (a_name == "Stealth meter") { return { 56.0F, 56.0F }; }
			if (a_name == "Subtitles") { return { 600.0F, 40.0F }; }
			if (a_name == "Ammo / arrow count") { return { 90.0F, 24.0F }; }
			if (a_name == "Activate prompt") { return { 220.0F, 40.0F }; }
			if (a_name == "Location info") { return { 320.0F, 30.0F }; }
			if (a_name == "Message info") { return { 420.0F, 28.0F }; }
			if (a_name == "Objective text") { return { 380.0F, 56.0F }; }
			if (a_name == "Level-up meter") { return { 120.0F, 120.0F }; }
			if (a_name == "Animated letters") { return { 300.0F, 48.0F }; }
			if (a_name == "Clock") { return { 96.0F, 24.0F }; }
			return { 140.0F, 40.0F };
		}

		struct Swatch { const char* name; int r, g, b; };
		constexpr Swatch kPalette[] = {
			{ "Gold", 255, 210, 64 },
			{ "Red", 235, 70, 70 },
			{ "Green", 100, 225, 110 },
			{ "Blue", 80, 140, 255 },
			{ "Cyan", 64, 210, 255 },
			{ "Magenta", 255, 100, 220 },
			{ "Orange", 255, 150, 50 },
			{ "White", 240, 240, 240 },
		};
		enum ColorIdx { kGold = 0, kRed, kGreen, kBlue };
		constexpr int kPaletteCount = static_cast<int>(sizeof(kPalette) / sizeof(kPalette[0]));

		const Swatch& SwatchAt(int a_i) { return kPalette[(a_i >= 0 && a_i < kPaletteCount) ? a_i : 0]; }


		float AsFloat(const std::string& a_sec, const std::string& a_key, float a_def)
		{
			const auto v = state::Config().Get(a_sec, a_key);
			if (!v || v->empty()) {
				return a_def;
			}
			try {
				return std::stof(*v);
			} catch (...) {
				return a_def;
			}
		}

		void Marker(ImGuiMCP::ImDrawList* a_dl, float a_sw, float a_sh, const char* a_name, float a_stageX, float a_stageY, float a_scale, int a_color)
		{
			const float cx = a_stageX / kStageW * a_sw;
			const float cy = a_stageY / kStageH * a_sh;
			const float sc = a_scale <= 0.0F ? 1.0F : a_scale;
			const Size  box = SizeFor(a_name);
			// Stage units scale onto the screen by the same factor the HUD movie does.
			const float hw = box.w * 0.5F * sc / kStageW * a_sw;
			const float hh = box.h * 0.5F * sc / kStageH * a_sh;
			const Swatch& sw2 = SwatchAt(a_color);
			const ImGuiMCP::ImU32 kFill = IM_COL32(sw2.r, sw2.g, sw2.b, 40);
			const ImGuiMCP::ImU32 kLine = IM_COL32(sw2.r, sw2.g, sw2.b, 220);
			constexpr ImGuiMCP::ImU32 kText = IM_COL32(255, 255, 255, 235);
			ImGuiMCP::ImDrawListManager::AddRectFilled(a_dl, ImGuiMCP::ImVec2{ cx - hw, cy - hh }, ImGuiMCP::ImVec2{ cx + hw, cy + hh }, kFill, 3.0F, 0);
			ImGuiMCP::ImDrawListManager::AddRect(a_dl, ImGuiMCP::ImVec2{ cx - hw, cy - hh }, ImGuiMCP::ImVec2{ cx + hw, cy + hh }, kLine, 3.0F, 0, 2.0F);
			ImGuiMCP::ImDrawListManager::AddLine(a_dl, ImGuiMCP::ImVec2{ cx - 8, cy }, ImGuiMCP::ImVec2{ cx + 8, cy }, kLine, 1.5F);
			ImGuiMCP::ImDrawListManager::AddLine(a_dl, ImGuiMCP::ImVec2{ cx, cy - 8 }, ImGuiMCP::ImVec2{ cx, cy + 8 }, kLine, 1.5F);
			ImGuiMCP::ImDrawListManager::AddText(a_dl, ImGuiMCP::ImVec2{ cx - hw + 4, cy - hh - 16 }, kText, a_name);
		}

		void __stdcall DrawHud() { DrawAll(); }  // AMF HUD-element entry (closed-menu path)
	}

	void DrawAll()
	{
		if (!settings::preview::show || !state::Config().loaded()) {
			return;
		}
		ImGuiMCP::ImGuiIO*    io = ImGuiMCP::GetIO();
		ImGuiMCP::ImDrawList* dl = ImGuiMCP::GetForegroundDrawList();
		if (!io || !dl) {
			return;
		}
		const float sw = io->DisplaySize.x, sh = io->DisplaySize.y;
		const auto& els = skyhud::Elements();
		auto&       ghosts = settings::preview::Ghosts();
		for (std::size_t i = 0; i < els.size(); ++i) {
			const auto& el = els[i];
			if (i < ghosts.size() && !ghosts[i].show) {
				continue;  // this element's ghost is turned off
			}
			if (!el.lockKey || !el.lockKey[0] || el.positions.empty()) {
				continue;  // no position to preview
			}
			const auto locked = state::Config().Get("Position", el.lockKey);
			if (locked && *locked == "1") {
				continue;  // uses SkyHUD's default position - unknown to us
			}
			const auto& pp = el.positions.front();
			const auto vx = (pp.xKey && pp.xKey[0]) ? state::Config().Get("Position", pp.xKey) : std::nullopt;
			const auto vy = (pp.yKey && pp.yKey[0]) ? state::Config().Get("Position", pp.yKey) : std::nullopt;
			if (!((vx && !vx->empty()) || (vy && !vy->empty()))) {
				continue;  // no explicit coordinate set yet
			}
			const float px = AsFloat("Position", pp.xKey ? pp.xKey : "", 0.0F);
			const float py = AsFloat("Position", pp.yKey ? pp.yKey : "", 0.0F);
			const float sc = (el.scaleKey && el.scaleKey[0]) ? AsFloat("Scale", el.scaleKey, 1.0F) : 1.0F;
			const int   color = (i < ghosts.size()) ? ghosts[i].color : 0;
			Marker(dl, sw, sh, el.name, px, py, sc, color);
		}
	}

	int PaletteCount() { return kPaletteCount; }
	const char* PaletteName(int a_i) { return (a_i >= 0 && a_i < kPaletteCount) ? kPalette[a_i].name : ""; }

	int DefaultColorFor(const char* a_name)
	{
		const std::string n = a_name ? a_name : "";
		if (n == "Health") { return kRed; }
		if (n == "Stamina") { return kGreen; }
		if (n == "Magicka") { return kBlue; }
		return kGold;  // no natural colour - a neutral, readable default
	}

	void Register()
	{
		if (!SKSEMenuFramework::IsInstalled()) {
			return;
		}
		SKSEMenuFramework::AddHudElement(DrawHud);
		logger::info("Registered the position-preview HUD overlay (visible while the menu is closed)");
	}
}
