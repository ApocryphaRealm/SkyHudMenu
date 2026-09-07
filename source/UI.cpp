#include "UI.h"

#include "HudElements.h"
#include "SKSEMenuFramework.h"
#include "HudPreview.h"
#include "Settings.h"
#include "SkyHudState.h"
#include "utils/Toggle.h"
#include "utils/Logger.h"
#include "utils/Strings.h"

#include <charconv>
#include <cstdlib>
#include <optional>
#include <string>

namespace UI
{
	namespace
	{
		std::string g_status;

		// The same deterministic label->key mapping tools/gen-translations.py uses for every
		// HudElements.cpp table label (tab names, position/toggle/dropdown labels and dropdown
		// option text): each run of alphanumerics becomes one word, capitalised on its first
		// character, words are concatenated, and the prefix is prepended verbatim. Two labels
		// with the same text (e.g. every plain "Position" tab) share one key on purpose - one
		// translation serves both.
		std::string DeriveKey(const char* a_prefix, const char* a_text)
		{
			std::string out{ a_prefix };
			bool        startWord = true;
			for (const char* p = a_text ? a_text : ""; *p; ++p) {
				const unsigned char c = static_cast<unsigned char>(*p);
				if (std::isalnum(c)) {
					out += startWord ? static_cast<char>(std::toupper(c)) : static_cast<char>(c);
					startWord = false;
				} else {
					startWord = true;
				}
			}
			return out;
		}

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
			const std::string lbl = std::string(strings::TR("SHM_Scale", "Scale")) + "##" + a_key;
			if (ImGuiMCP::SliderFloat(lbl.c_str(), &v, 0.25F, 3.0F, "%.2fx")) {
				state::Config().Set("Scale", a_key, FloatToValue(v));
			}
		}

		void PositionControl(const skyhud::PosPair& a_pos)
		{
			if (!HasKey("Position", a_pos.xKey) && !HasKey("Position", a_pos.yKey)) { return; }
			ImGuiMCP::TextDisabled("%s", strings::TR(DeriveKey("SHM_El_", a_pos.label).c_str(), a_pos.label));
			if (HasKey("Position", a_pos.xKey)) {
				float x = ValueAsFloat("Position", a_pos.xKey, 0.0F);
				const std::string lbl = std::string(strings::TR("SHM_X", "X")) + "##" + a_pos.xKey;
				if (ImGuiMCP::InputFloat(lbl.c_str(), &x, 1.0F, 10.0F, "%.0f")) {
					state::Config().Set("Position", a_pos.xKey, FloatToValue(x));
				}
			}
			if (HasKey("Position", a_pos.yKey)) {
				float y = ValueAsFloat("Position", a_pos.yKey, 0.0F);
				const std::string lbl = std::string(strings::TR("SHM_Y", "Y")) + "##" + a_pos.yKey;
				if (ImGuiMCP::InputFloat(lbl.c_str(), &y, 1.0F, 10.0F, "%.0f")) {
					state::Config().Set("Position", a_pos.yKey, FloatToValue(y));
				}
			}
		}

		void ToggleControl(const skyhud::ToggleField& a_t)
		{
			if (!HasKey(a_t.section, a_t.key)) { return; }
			bool on = ValueAsBool(a_t.section, a_t.key);
			if (ImGuiMCP::Toggle(strings::TR(DeriveKey("SHM_El_", a_t.label).c_str(), a_t.label), &on)) {
				state::Config().Set(a_t.section, a_t.key, on ? "1" : "0");
			}
		}

		void DropdownControl(const skyhud::DropdownField& a_d)
		{
			if (!HasKey(a_d.section, a_d.key)) { return; }
			const auto current = state::Config().Get(a_d.section, a_d.key).value_or("");
			int         index = 0;
			// Option text is rebuilt from TR'd entries every frame (plan 2.2); labelStore owns the
			// translated bytes for this call so the const char* pointers handed to Combo stay valid.
			std::vector<std::string> labelStore;
			labelStore.reserve(a_d.options.size());
			for (std::size_t i = 0; i < a_d.options.size(); ++i) {
				labelStore.push_back(strings::TR(DeriveKey("SHM_El_", a_d.options[i].first).c_str(), a_d.options[i].first));
				if (current == a_d.options[i].second) {
					index = static_cast<int>(i);
				}
			}
			std::vector<const char*> labels;
			labels.reserve(labelStore.size());
			for (const auto& s : labelStore) { labels.push_back(s.c_str()); }
			if (ImGuiMCP::Combo(strings::TR(DeriveKey("SHM_El_", a_d.label).c_str(), a_d.label), &index, labels.data(), static_cast<int>(labels.size()))) {
				state::Config().Set(a_d.section, a_d.key, a_d.options[static_cast<std::size_t>(index)].second);
			}
		}

		void RenderElement(const skyhud::Element& a_el, std::size_t a_index)
		{
			// Per-element ghost: whether this element's on-screen marker shows, and its colour.
			auto& ghosts = settings::preview::Ghosts();
			if (a_index < ghosts.size()) {
				bool gshow = ghosts[a_index].show;
				if (ImGuiMCP::Toggle(strings::TR("SHM_ShowGhost", "Show this ghost on screen"), &gshow)) {
					ghosts[a_index].show = gshow;
					settings::Save();
				}
				if (gshow) {
					std::vector<std::string> colorStore;
					std::vector<const char*> colors;
					for (int c = 0; c < preview::PaletteCount(); ++c) {
						colorStore.push_back(strings::TR(DeriveKey("SHM_Col_", preview::PaletteName(c)).c_str(), preview::PaletteName(c)));
					}
					for (const auto& cs : colorStore) { colors.push_back(cs.c_str()); }
					int ci = ghosts[a_index].color;
					if (ci < 0 || ci >= preview::PaletteCount()) {
						ci = 0;
					}
					if (ImGuiMCP::Combo(strings::TR("SHM_GhostColour", "Ghost colour"), &ci, colors.data(), static_cast<int>(colors.size()))) {
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
				if (ImGuiMCP::Toggle(strings::TR("SHM_Locked", "Locked (use SkyHUD's default position)"), &locked)) {
					state::Config().Set("Position", a_el.lockKey, locked ? "1" : "0");
				}
				if (!locked && !a_el.positions.empty()) {
					ImGuiMCP::SeparatorText(strings::TR("SHM_Position", "Position"));
					for (const auto& p : a_el.positions) {
						PositionControl(p);
					}
				}
			}
			bool anyToggle = false;
			for (const auto& t : a_el.toggles) { anyToggle |= HasKey(t.section, t.key); }
			if (anyToggle) {
				ImGuiMCP::SeparatorText(strings::TR("SHM_Options", "Options"));
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
		strings::Tick();

		// Pick up any external edits each time the page draws while it is not dirty.
		if (!state::Config().loaded()) {
			state::LoadFromDisk();
		}
		if (!state::Config().loaded()) {
			ImGuiMCP::TextWrapped("%s", strings::TR("SHM_NoConfig", "skyhud.txt was not found. This menu edits SkyHUD's settings, so SkyHUD (or a preset that provides skyhud.txt) must be installed."));
			return;
		}

		ImGuiMCP::TextWrapped(strings::TR("SHM_EditingPath", "Editing %s"), state::Config().path().c_str());
		ImGuiMCP::TextDisabled("%s", strings::TR("SHM_TabHint", "Each tab is one HUD element. Save writes skyhud.txt; SkyHUD reads it when the game starts, so a saved change shows after a restart."));

		if (ImGuiMCP::Toggle(strings::TR("SHM_ShowMarkers", "Show on-screen position markers"), &settings::preview::show)) {
			settings::Save();
		}
		ImGuiMCP::SameLine(0.0F, 8.0F);
		ImGuiMCP::TextDisabled("%s", strings::TR("SHM_MarkersHint", "ghosts of every positioned element - visible with the menu closed too"));

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
				// "###name" pins the tab's ImGui id to the English name, so a language switch keeps the selected tab.
				const std::string tabLabel = std::string(strings::TR(DeriveKey("SHM_El_", els[i].name).c_str(), els[i].name)) + "###" + els[i].name;
				if (ImGuiMCP::BeginTabItem(tabLabel.c_str())) {
					RenderElement(els[i], i);
					ImGuiMCP::EndTabItem();
				}
			}
			ImGuiMCP::EndTabBar();
		}

		ImGuiMCP::SeparatorText("");
		ImGuiMCP::TextDisabled("%s", strings::TR("SHM_FooterA", "SkyHUD reads skyhud.txt only when the game starts, so a saved change shows after a"));
		ImGuiMCP::TextDisabled("%s", strings::TR("SHM_FooterB", "restart - the markers show now where each element will land."));
		if (ImGuiMCP::Button(strings::TR("SHM_SaveBtn", "Save (shows after a restart)"))) {
			g_status = state::WriteAndApply() ? strings::TR("SHM_StatusSaved", "Saved to skyhud.txt. Restart the game to see it in the HUD.")
											  : strings::TR("SHM_StatusSaveFail", "Could not write skyhud.txt. See the log.");
		}
		if (ImGuiMCP::Button(strings::TR("SHM_ReloadBtn", "Reload from file"))) {
			state::LoadFromDisk();
			g_status = strings::TR("SHM_StatusReloaded", "Reloaded skyhud.txt from disk.");
		}
		if (!g_status.empty()) {
			ImGuiMCP::TextWrapped("%s", g_status.c_str());
		}
	}
}
