#pragma once

// The on-screen position preview ("ghosts"). A gold labelled marker is drawn for EVERY HUD
// element the user has given an explicit position, at the same time, so the whole layout is
// visible at once - and it is drawn both while the settings page is open (page render) and while
// the AMF menu is closed (a registered HUD element), so the player can see the layout while
// actually playing. A toggle (settings::preview::show) turns the whole thing on and off.
//
// Only elements that are UNLOCKED with an explicit X/Y get a ghost; a locked element uses SkyHUD's
// own default position, which this mod cannot know without reading the live HUD (that is Approach
// B). SkyHUD applies the real move on the next HUD load; the ghost is how you see where it goes.
namespace preview
{
	// Draw all element ghosts from the current settings. Safe to call every frame; a no-op when
	// the toggle is off or the framework is absent.
	void DrawAll();

	// Register the closed-menu HUD overlay with AMF (so ghosts show while the menu is closed).
	void Register();

	// The marker-colour palette, for the settings page.
	int PaletteCount();
	const char* PaletteName(int a_i);

	// The default palette index for an element, matching its bar colour where one exists
	// (Health red, Stamina green, Magicka blue; others gold).
	int DefaultColorFor(const char* a_name);
}
