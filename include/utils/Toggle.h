#pragma once

// A shared on/off toggle-switch widget for every SKSE Menu Framework settings page in this
// project (see .MD\CLAUDE.md rule 32: boolean settings render as a sliding switch, not a
// tick-box). SMF's ImGuiMCP wrapper only exposes cimgui's plain Checkbox - there is no native
// switch export - so this is hand-drawn from the framework's lower-level draw-list primitives,
// the same discipline as every other ImGuiMCP wrapper in this project.
//
// Drop-in replacement for ImGuiMCP::Checkbox at any boolean call site: identical signature,
// identical "returns true the frame the value changed" contract, identical trailing label
// drawn to the right of the widget on the same line.
//
// Every cimgui export this file resolves at runtime must be probed by the including repo's own
// HasRequiredExports() in source/UI.cpp before the settings page is allowed to register - a
// page that needs an export the installed SMF build doesn't have must refuse to register (with
// a clear log message) instead of crashing on first draw the way a null function pointer would.
// This file needs, by resolved export name (not the C++-looking wrapper name - see the
// igTextDisabledV-not-igTextDisabled gotcha at the end of CLAUDE.md):
//
//   igGetCursorScreenPos, igGetWindowDrawList, igGetFrameHeight, igInvisibleButton,
//   igIsItemHovered, igPushID_Str, igPopID, igSameLine, igTextV,
//   ImDrawList_AddRectFilled, ImDrawList_AddCircleFilled
//
// (igSameLine and igTextV are already required by every existing settings page for other
// widgets; the rest are new.)
//
// Vendored byte-identical into every repo with an SMF settings page, the same way
// include/SKSEMenuFramework.h itself is vendored per-repo - these are separate CMake projects
// with no shared library link between them.

#include "SKSEMenuFramework.h"

namespace ImGuiMCP
{
	// Hand-drawn pill track (~2:1 width:height) with a circular knob that occupies most of the
	// track's height, sliding left when off / right when on. The track color itself differs
	// between states - red off, green on - so the state reads even without watching the knob
	// move. No animation: the knob jumps straight to its new position, which reads as instant
	// given SMF redraws every frame anyway.
	inline bool Toggle(const char* a_label, bool* a_value)
	{
		PushID(a_label);

		const float height = GetFrameHeight();
		const float width = height * 2.0f;
		const float radius = height * 0.5f;

		const ImVec2 pos = GetCursorScreenPos();
		ImDrawList* drawList = GetWindowDrawList();

		const bool changed = InvisibleButton("##toggle", ImVec2{ width, height });
		const bool hovered = IsItemHovered();

		if (changed && a_value)
		{
			*a_value = !*a_value;
		}

		const bool isOn = a_value && *a_value;

		// Green when on, red when off; a hovered track brightens slightly so the widget still
		// reads as interactive without needing the knob to move first.
		const ImU32 trackColor = isOn ? (hovered ? IM_COL32(92, 191, 96, 255) : IM_COL32(76, 175, 80, 255))
		                               : (hovered ? IM_COL32(207, 84, 84, 255) : IM_COL32(191, 68, 68, 255));

		const float knobX = pos.x + radius + (isOn ? (width - height) : 0.0f);

		ImDrawListManager::AddRectFilled(drawList, pos, ImVec2{ pos.x + width, pos.y + height }, trackColor, radius, 0);
		ImDrawListManager::AddCircleFilled(drawList, ImVec2{ knobX, pos.y + radius }, radius - 2.0f, IM_COL32(240, 240, 240, 255), 32);

		PopID();

		if (a_label)
		{
			// Mirrors ImGui's own "##" convention: anything from "##" onward is an ID
			// disambiguator, not part of the visible label, so it is not drawn.
			std::string_view label{ a_label };
			size_t hashPos = label.find("##");
			std::string_view visible = (hashPos == std::string_view::npos) ? label : label.substr(0, hashPos);

			if (!visible.empty())
			{
				SameLine();
				Text("%.*s", static_cast<int>(visible.size()), visible.data());
			}
		}

		return changed;
	}
}
