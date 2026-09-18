#pragma once

#include <string>

// The on-screen position preview ("ghosts"). A labelled marker is drawn for EVERY positioned HUD
// element - every position pair of it (the charge meters have three, the activate prompt four) - at
// the same time, so the whole layout is visible at once; both while the settings page is open and
// while the AMF menu is closed (a registered HUD element). A toggle (settings::preview::show) turns
// the whole thing on and off.
//
// Where a widget's clip is exposed by the live HUD movie (the vanilla instance names, which HUD
// replacers keep), the ghost takes its real size and its real anchor. SkyHUD's X/Y for an element
// is not the clip's own _x/_y (measured 2026-09-05: three bars all at X=65 sat at three different
// _x), so the anchor is measured rather than assumed: at the first read in a session the widget
// sits where the file's X/Y put it at game start, so the offset from that X/Y to the widget's box
// in stage space is the element's anchor, and the ghost is that box moved by however much X/Y have
// been edited since. A widget the movie does not expose gets a representative box centred on X/Y.
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

	// One widget as the live HUD movie reports it, in stage units (1280x720, _root space). ok=false
	// when the clip is not exposed. x/y = the clip's own _x/_y (parent space, for the record); the
	// bounds are the box the player sees in _root space; scale = the clip's current _xscale/100.
	struct LiveClip
	{
		bool  ok = false;
		float x = 0, y = 0, xMin = 0, yMin = 0, xMax = 0, yMax = 0, scale = 1.0F;
	};
	// Read it now (a handful of Scaleform calls; not for every frame - DrawAll caches).
	LiveClip ReadLive(const char* a_clip);
	// The same, through the 500 ms cache DrawAll uses.
	LiveClip ReadLiveCached(const char* a_clip);

	// Where a position pair's widget is RIGHT NOW, in the file's own units (the clip's _x/_y, which is
	// what SkyHUD writes the file's X/Y into). The owner, 2026-09-18: "make the starting locations of the
	// overlay ghosts the current locations of each of the widgets instead of starting at 0,0" - Norden
	// UI's skyhud.txt leaves many coordinates EMPTY (SkyHUD then keeps the widget where the HUD put it),
	// and an empty coordinate read as 0 put the ghost and the X/Y boxes at the corner. Returns false when
	// the pair exposes no clip or the HUD movie is not up.
	bool LiveStart(const char* a_clip, float& a_x, float& a_y);
	// A JSON line per position pair, for the test tool.
	std::string LiveReport();
}
