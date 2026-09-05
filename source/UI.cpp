#include "UI.h"

#include "HudElements.h"
#include "SKSEMenuFramework.h"
#include "HudPreview.h"
#include "Settings.h"
#include "SkyHudState.h"
#include "utils/Toggle.h"
#include "utils/Logger.h"

#include <charconv>
#include <cstdlib>
#include <optional>
#include <string>

namespace UI
{
	namespace
	{
		std::string g_status;

		// --- small helpers over the string-valued skyhud.txt model -----------------------------

		float ValueAsFloat(const std::string& a_section, const std::string& a_key, float a_fallback)
		{
			const auto v = state::Config().Get(a_section, a_key);
			if (!v || v->empty()) {
				return a_fallback;
			}
			try {
				return std::stof(*v);
			} catch (...) {
				return a_fallback;
			}
		}

		std::string FloatToValue(float a_v)
		{
			// Trim trailing zeros so "1" stays "1" and "0.85" stays "0.85".
			char   buf[32];
			std::snprintf(buf, sizeof(buf), "%.4f", a_v);
			std::string s{ buf };
			if (s.find('.') != std::string::npos) {
				while (s.back() == '0') {
					s.pop_back();
				}
				if (s.back() == '.') {
					s.pop_back();
				}
			}
			return s;
		}

		bool HasKey(const char* a_section, const char* a_key)
		{
			return a_key && a_key[0] && state::Config().Get(a_section, a_key).has_value();
		}

		// Whether any control on this tab exists in the user's file.
		bool ElementPresent(const skyhud::Element& a_el)
		{
			if (HasKey("Scale", a_el.scaleKey) || HasKey("Position", a_el.lockKey)) { return true; }
			for (const auto& p : a_el.positions) { if (HasKey("Position", p.xKey) || HasKey("Position", p.yKey)) { return true; } }
			for (const auto& t : a_el.toggles) { if (HasKey(t.section, t.key)) { return true; } }
			for (const auto& d : a_el.dropdowns) { if (HasKey(d.section, d.key)) { return true; } }
			return false;
		}

		bool ValueAsBool(const std::string& a_section, const std::string& a_key)
		{
			const auto v = state::Config().Get(a_section, a_key);
			return v && (*v == "1");
		}

		// A scale slider (0.5x .. 2.0x) for [Scale] a_key.
		void ScaleControl(const char* a_key)
		{
			float v = ValueAsFloat("Scale", a_key, 1.0F);
			if (ImGuiMCP::SliderFloat("Scale", &v, 0.25F, 3.0F, "%.2fx")) {
				state::Config().Set("Scale", a_key, FloatToValue(v));
			}
		}

		void PositionControl(const skyhud::PosPair& a_pos)
		{
			if (!HasKey("Position", a_pos.xKey) && !HasKey("Position", a_pos.yKey)) { return; }
			ImGuiMCP::TextDisabled("%s", a_pos.label);
			if (HasKey("Position", a_pos.xKey)) {
				float x = ValueAsFloat("Position", a_pos.xKey, 0.0F);
				const std::string lbl = std::string("X##") + a_pos.xKey;
				if (ImGuiMCP::InputFloat(lbl.c_str(), &x, 1.0F, 10.0F, "%.0f")) {
					state::Config().Set("Position", a_pos.xKey, FloatToValue(x));
				}
			}
			if (HasKey("Position", a_pos.yKey)) {
				float y = ValueAsFloat("Position", a_pos.yKey, 0.0F);
				const std::string lbl = std::string("Y##") + a_pos.yKey;
				if (ImGuiMCP::InputFloat(lbl.c_str(), &y, 1.0F, 10.0F, "%.0f")) {
					state::Config().Set("Position", a_pos.yKey, FloatToValue(y));
				}
			}
		}

		void ToggleControl(const skyhud::ToggleField& a_t)
		{
			if (!HasKey(a_t.section, a_t.key)) { return; }
			bool on = ValueAsBool(a_t.section, a_t.key);
			if (ImGuiMCP::Toggle(a_t.label, &on)) {
				state::Config().Set(a_t.section, a_t.key, on ? "1" : "0");
			}
		}

		void DropdownControl(const skyhud::DropdownField& a_d)
		{
			if (!HasKey(a_d.section, a_d.key)) { return; }
			const auto current = state::Config().Get(a_d.section, a_d.key).value_or("");
			int         index = 0;
			std::vector<const char*> labels;
			labels.reserve(a_d.options.size());
			for (std::size_t i = 0; i < a_d.options.size(); ++i) {
				labels.push_back(a_d.options[i].first);
				if (current == a_d.options[i].second) {
					index = static_cast<int>(i);
				}
			}
			if (ImGuiMCP::Combo(a_d.label, &index, labels.data(), static_cast<int>(labels.size()))) {
				state::Config().Set(a_d.section, a_d.key, a_d.options[static_cast<std::size_t>(index)].second);
			}
		}

		void RenderElement(const skyhud::Element& a_el, std::size_t a_index)
		{
			// Per-element ghost: whether this element's on-screen marker shows, and its colour.
			auto& ghosts = settings::preview::Ghosts();
			if (a_index < ghosts.size()) {
				bool gshow = ghosts[a_index].show;
				if (ImGuiMCP::Toggle("Show this ghost on screen", &gshow)) {
					ghosts[a_index].show = gshow;
					settings::Save();
				}
				if (gshow) {
					std::vector<const char*> colors;
					for (int c = 0; c < preview::PaletteCount(); ++c) {
						colors.push_back(preview::PaletteName(c));
					}
					int ci = ghosts[a_index].color;
					if (ci < 0 || ci >= preview::PaletteCount()) {
						ci = 0;
					}
					if (ImGuiMCP::Combo("Ghost colour", &ci, colors.data(), static_cast<int>(colors.size()))) {
						ghosts[a_index].color = ci;
						settings::Save();
					}
				}
				ImGuiMCP::Separator();
			}
			if (HasKey("Scale", a_el.scaleKey)) {
				ScaleControl(a_el.scaleKey);
			}
			if (HasKey("Position", a_el.lockKey)) {
				bool locked = ValueAsBool("Position", a_el.lockKey);
				if (ImGuiMCP::Toggle("Locked (use SkyHUD's default position)", &locked)) {
					state::Config().Set("Position", a_el.lockKey, locked ? "1" : "0");
				}
				if (!locked && !a_el.positions.empty()) {
					ImGuiMCP::SeparatorText("Position");
					for (const auto& p : a_el.positions) {
						PositionControl(p);
					}
				}
			}
			bool anyToggle = false;
			for (const auto& t : a_el.toggles) { anyToggle |= HasKey(t.section, t.key); }
			if (anyToggle) {
				ImGuiMCP::SeparatorText("Options");
				for (const auto& t : a_el.toggles) {
					ToggleControl(t);
				}
			}
			for (const auto& d : a_el.dropdowns) {
				DropdownControl(d);
			}

		}
	}

	void __stdcall RenderPage();

	void Register()
	{
		if (!SKSEMenuFramework::IsInstalled()) {
			logger::info("Apocrypha Menu Framework not installed; SkyHUD Settings Menu has no page to show");
			return;
		}
		SKSEMenuFramework::SetSection("SkyHUD Settings Menu");
		SKSEMenuFramework::AddSectionItem("HUD elements", RenderPage);
		logger::info("Registered the SkyHUD Settings Menu page");
	}

	void __stdcall RenderPage()
	{
		// Pick up any external edits each time the page draws while it is not dirty.
		if (!state::Config().loaded()) {
			state::LoadFromDisk();
		}
		if (!state::Config().loaded()) {
			ImGuiMCP::TextWrapped("skyhud.txt was not found. This menu edits SkyHUD's settings, so "
								  "SkyHUD (or a preset that provides skyhud.txt) must be installed.");
			return;
		}

		ImGuiMCP::TextWrapped("Editing %s", state::Config().path().c_str());
		ImGuiMCP::TextDisabled("Each tab is one HUD element. Save writes skyhud.txt; SkyHUD reads it when the "
							   "game starts, so a saved change shows after a restart.");

		if (ImGuiMCP::Toggle("Show on-screen position markers", &settings::preview::show)) {
			settings::Save();
		}
		ImGuiMCP::SameLine(0.0F, 8.0F);
		ImGuiMCP::TextDisabled("ghosts of every positioned element - visible with the menu closed too");

		preview::DrawAll();

		// Nineteen element tabs do not fit the default window width; scrolling keeps every label whole
		// (the default policy squeezes them to "H... M... St..."), and the list button on the left opens
		// every tab by name.
		if (ImGuiMCP::BeginTabBar("SkyHudElements", ImGuiMCP::ImGuiTabBarFlags_FittingPolicyScroll | ImGuiMCP::ImGuiTabBarFlags_TabListPopupButton)) {
			const auto& els = skyhud::Elements();
			for (std::size_t i = 0; i < els.size(); ++i) {
				if (!ElementPresent(els[i])) {
					continue;  // none of its keys are in this file
				}
				if (ImGuiMCP::BeginTabItem(els[i].name)) {
					RenderElement(els[i], i);
					ImGuiMCP::EndTabItem();
				}
			}
			ImGuiMCP::EndTabBar();
		}

		ImGuiMCP::SeparatorText("");
		ImGuiMCP::TextDisabled("SkyHUD reads skyhud.txt only when the game starts, so a saved change shows after a");
		ImGuiMCP::TextDisabled("restart - the markers show now where each element will land.");
		if (ImGuiMCP::Button("Save (shows after a restart)")) {
			g_status = state::WriteAndApply() ? "Saved to skyhud.txt. Restart the game to see it in the HUD."
											  : "Could not write skyhud.txt. See the log.";
		}
		if (ImGuiMCP::Button("Reload from file")) {
			state::LoadFromDisk();
			g_status = "Reloaded skyhud.txt from disk.";
		}
		if (!g_status.empty()) {
			ImGuiMCP::TextWrapped("%s", g_status.c_str());
		}
	}
}
