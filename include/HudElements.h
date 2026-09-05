#pragma once

// ============================================================================================
// The HUD-element table: the backbone of the UI. skyhud.txt groups its settings by KIND
// ([Scale], [Position], [Interface], [Gameplay]); this menu regroups them by the HUD ELEMENT
// they affect, one tab per element (Liam's layout, 2026-09-05). Each Element names the
// skyhud.txt keys that belong to it, and the page renders one tab per Element from this table -
// so adding or moving a control is a data edit here, never UI code.
// ============================================================================================

#include <string>
#include <utility>
#include <vector>

namespace skyhud
{
	// A position control: a lock checkbox is on the Element; each PosPair is one X/Y coordinate
	// (most elements have one; Activate Prompt has four sub-parts; Animated Letters is Y-only, so
	// xKey may be empty).
	struct PosPair
	{
		const char* label;
		const char* xKey;  // "" = no X (Y-only)
		const char* yKey;  // "" = no Y
	};

	struct ToggleField
	{
		const char* label;
		const char* section;
		const char* key;
	};

	struct DropdownField
	{
		const char*                                          label;
		const char*                                          section;
		const char*                                          key;
		std::vector<std::pair<const char*, const char*>>     options;  // {display, stored value}
	};

	struct Element
	{
		const char* name;  // tab label

		const char* scaleKey = "";  // key in [Scale], "" if the element has no scale
		const char* lockKey = "";   // key in [Position], "" if it cannot be locked/moved

		std::vector<PosPair>       positions;
		std::vector<ToggleField>   toggles;
		std::vector<DropdownField> dropdowns;
	};

	// Built once; the order here is the tab order.
	const std::vector<Element>& Elements();
}
