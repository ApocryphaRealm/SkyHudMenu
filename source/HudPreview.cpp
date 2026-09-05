#include "HudPreview.h"

#include "HudElements.h"
#include "SKSEMenuFramework.h"
#include "Settings.h"
#include "SkyHudState.h"
#include "utils/Logger.h"

#include <RE/Skyrim.h>

#include <cstdio>

#include <chrono>
#include <map>
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

		// The live-clip cache: read at most every 500 ms per clip (Scaleform calls are not free, and
		// the ghosts draw every frame).
		std::map<std::string, std::pair<preview::LiveClip, std::chrono::steady_clock::time_point>> g_live;
		preview::LiveClip Cached(const char* a_clip)
		{
			if (!a_clip || !a_clip[0]) { return {}; }
			const auto now = std::chrono::steady_clock::now();
			auto it = g_live.find(a_clip);
			if (it != g_live.end() && now - it->second.second < std::chrono::milliseconds(500)) { return it->second.first; }
			const auto lc = preview::ReadLive(a_clip);
			g_live[a_clip] = { lc, now };
			return lc;
		}

		// The measured anchor per clip: the file's X/Y at the first read of the session, and the box the
		// widget sat in then (in _root space). Both are from the same moment, so the difference is the
		// element's anchor for the rest of the session.
		struct Anchor { float x0, y0, xMin, yMin, xMax, yMax, scale; };
		std::map<std::string, Anchor> g_anchor;
		const Anchor* AnchorFor(const char* a_clip, float a_fileX, float a_fileY, float a_fileScale)
		{
			if (!a_clip || !a_clip[0]) { return nullptr; }
			if (auto it = g_anchor.find(a_clip); it != g_anchor.end()) { return &it->second; }
			const auto lc = Cached(a_clip);
			if (!lc.ok || lc.xMax <= lc.xMin || lc.yMax <= lc.yMin) { return nullptr; }
			auto& a = g_anchor[a_clip];
			a = { a_fileX, a_fileY, lc.xMin, lc.yMin, lc.xMax, lc.yMax, a_fileScale > 0.0F ? a_fileScale : 1.0F };
			logger::info("ghost anchor for {}: file X/Y {:.0f}/{:.0f} -> widget box [{:.1f},{:.1f}]-[{:.1f},{:.1f}] at scale {:.2f}",
						 a_clip, a_fileX, a_fileY, lc.xMin, lc.yMin, lc.xMax, lc.yMax, a.scale);
			return &a;
		}

		// a_stageX/Y = the file's X/Y for the element, in stage units. With a measured anchor the ghost
		// is the widget's real box moved by the edit since the anchor was taken, resized by the edited
		// scale over the anchor's; otherwise a representative box is centred on X/Y.
		void Marker(ImGuiMCP::ImDrawList* a_dl, float a_sw, float a_sh, const char* a_name, const char* a_label, float a_stageX, float a_stageY, float a_scale, int a_color, const Anchor* a_anchor)
		{
			const float sc = a_scale <= 0.0F ? 1.0F : a_scale;
			float l, t, r, b;   // stage units
			if (a_anchor)
			{
				const float k = sc / a_anchor->scale;
				const float dx = a_stageX - a_anchor->x0, dy = a_stageY - a_anchor->y0;
				// resize about the box's own top-left, which is what a scale change does to a left-anchored
				// bar; a centred widget is off by half the growth, which is small at the scales in use
				l = a_anchor->xMin + dx;  r = l + (a_anchor->xMax - a_anchor->xMin) * k;
				t = a_anchor->yMin + dy;  b = t + (a_anchor->yMax - a_anchor->yMin) * k;
			}
			else
			{
				const Size box = SizeFor(a_name);
				l = a_stageX - box.w * 0.5F * sc;  r = a_stageX + box.w * 0.5F * sc;
				t = a_stageY - box.h * 0.5F * sc;  b = a_stageY + box.h * 0.5F * sc;
			}
			// Stage units scale onto the screen by the same factor the HUD movie does.
			const float cx = a_stageX / kStageW * a_sw, cy = a_stageY / kStageH * a_sh;
			const float x0 = l / kStageW * a_sw, x1 = r / kStageW * a_sw, y0 = t / kStageH * a_sh, y1 = b / kStageH * a_sh;
			const Swatch& sw2 = SwatchAt(a_color);
			const ImGuiMCP::ImU32 kFill = IM_COL32(sw2.r, sw2.g, sw2.b, 40);
			const ImGuiMCP::ImU32 kLine = IM_COL32(sw2.r, sw2.g, sw2.b, 220);
			constexpr ImGuiMCP::ImU32 kText = IM_COL32(255, 255, 255, 235);
			ImGuiMCP::ImDrawListManager::AddRectFilled(a_dl, ImGuiMCP::ImVec2{ x0, y0 }, ImGuiMCP::ImVec2{ x1, y1 }, kFill, 3.0F, 0);
			ImGuiMCP::ImDrawListManager::AddRect(a_dl, ImGuiMCP::ImVec2{ x0, y0 }, ImGuiMCP::ImVec2{ x1, y1 }, kLine, 3.0F, 0, 2.0F);
			ImGuiMCP::ImDrawListManager::AddLine(a_dl, ImGuiMCP::ImVec2{ cx - 8, cy }, ImGuiMCP::ImVec2{ cx + 8, cy }, kLine, 1.5F);
			ImGuiMCP::ImDrawListManager::AddLine(a_dl, ImGuiMCP::ImVec2{ cx, cy - 8 }, ImGuiMCP::ImVec2{ cx, cy + 8 }, kLine, 1.5F);
			ImGuiMCP::ImDrawListManager::AddText(a_dl, ImGuiMCP::ImVec2{ x0 + 4, y0 - 16 }, kText, a_label);
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
			const float sc = (el.scaleKey && el.scaleKey[0]) ? AsFloat("Scale", el.scaleKey, 1.0F) : 1.0F;
			const int   color = (i < ghosts.size()) ? ghosts[i].color : 0;
			for (const auto& pp : el.positions) {
				const auto vx = (pp.xKey && pp.xKey[0]) ? state::Config().Get("Position", pp.xKey) : std::nullopt;
				const auto vy = (pp.yKey && pp.yKey[0]) ? state::Config().Get("Position", pp.yKey) : std::nullopt;
				if (!((vx && !vx->empty()) || (vy && !vy->empty()))) {
					continue;  // no explicit coordinate set for this pair
				}
				const float px = AsFloat("Position", pp.xKey ? pp.xKey : "", 0.0F);
				const float py = AsFloat("Position", pp.yKey ? pp.yKey : "", 0.0F);
				const std::string label = el.positions.size() > 1 ? std::string(el.name) + " - " + pp.label : std::string(el.name);
				Marker(dl, sw, sh, el.name, label.c_str(), px, py, sc, color, AnchorFor((pp.measure && pp.measure[0]) ? pp.measure : pp.clip, px, py, sc));
			}
		}
	}

	LiveClip ReadLive(const char* a_clip)
	{
		LiveClip out;
		if (!a_clip || !a_clip[0]) { return out; }
		auto* ui = RE::UI::GetSingleton();
		auto  menu = ui ? ui->GetMenu(RE::HUDMenu::MENU_NAME) : RE::GPtr<RE::IMenu>{};
		auto* movie = (menu && menu->uiMovie) ? menu->uiMovie.get() : nullptr;
		if (!movie) { return out; }
		const std::string path = std::string("_root.HUDMovieBaseInstance.") + a_clip;
		RE::GFxValue clip;
		if (!movie->GetVariable(&clip, path.c_str()) || !clip.IsObject()) { return out; }
		RE::GFxValue v;
		auto num = [&](const char* a_name, float& a_out) { if (clip.GetMember(a_name, &v) && v.IsNumber()) { a_out = static_cast<float>(v.GetNumber()); return true; } return false; };
		if (!num("_x", out.x) || !num("_y", out.y)) { return out; }
		float xs = 100.0F; num("_xscale", xs); out.scale = xs / 100.0F;
		// The box the player sees, in _root space (the 1280x720 stage the file's X/Y are in).
		RE::GFxValue root;
		if (!movie->GetVariable(&root, "_root") || !root.IsObject()) { return out; }
		RE::GFxValue bounds;
		if (!clip.Invoke("getBounds", &bounds, &root, 1) || !bounds.IsObject()) { return out; }
		RE::GFxValue m;
		auto bnum = [&](const char* a_name, float& a_out) { if (bounds.GetMember(a_name, &m) && m.IsNumber()) { a_out = static_cast<float>(m.GetNumber()); return true; } return false; };
		if (!bnum("xMin", out.xMin) || !bnum("xMax", out.xMax) || !bnum("yMin", out.yMin) || !bnum("yMax", out.yMax)) { return out; }
		out.ok = true;
		return out;
	}

	std::string LiveReport()
	{
		std::string s = "[";
		bool first = true;
		for (const auto& el : skyhud::Elements()) {
			for (const auto& pp : el.positions) {
				if (!pp.clip || !pp.clip[0]) { continue; }
				const char* which = (pp.measure && pp.measure[0]) ? pp.measure : pp.clip;
				const auto lc = ReadLive(which);
				char buf[320];
				std::snprintf(buf, sizeof(buf), "%s{\"element\":\"%s\",\"pair\":\"%s\",\"clip\":\"%s\",\"ok\":%s,\"x\":%.1f,\"y\":%.1f,\"xMin\":%.1f,\"yMin\":%.1f,\"xMax\":%.1f,\"yMax\":%.1f,\"scale\":%.2f}",
							  first ? "" : ",", el.name, pp.label, which, lc.ok ? "true" : "false", lc.x, lc.y, lc.xMin, lc.yMin, lc.xMax, lc.yMax, lc.scale);
				s += buf;
				first = false;
			}
		}
		return s + "]";
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
