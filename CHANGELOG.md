Version 1.0.0
Added an in-game settings page for SkyHUD, one tab per HUD element, that reads and writes your skyhud.txt.
Added a live on-screen marker showing where each element will sit, sized to roughly match the widget and moving as you adjust it.
Added per-element control of each marker's visibility and colour, with Health, Stamina and Magicka defaulting to their bar colours, plus a master on/off toggle.

## 1.0.3 - 2026-09-05 - working

### Fixed
- The element tabs no longer squeeze their labels to a letter or two when the window is narrow: the tab bar scrolls, keeping every name whole, and a list button at its left opens any tab by name.

## 1.0.2 - 2026-09-05 - working

### Fixed
- The page and the Save button now say plainly that SkyHUD reads skyhud.txt when the game starts, so a saved change shows after a restart; the earlier claim that a save load reloads it was wrong, and the attempt to rebuild the HUD live has been removed because the engine does not survive it.
- Each marker is now anchored from where its widget really sits: the live HUD movie is read for the widget's own box, so the marker coincides with the bar whatever the element's anchor point, and moves by exactly what you change.
- Every position pair now gets an on-screen marker: the right and combined charge meters and the activate prompt's name, info and bar parts were drawn for the first pair only.

### Added
- The skyhud.config tool gained op=clips, which reads every known widget's live position, box and scale from the HUD movie.
- Dragonborn UI support: a Shout meter tab (fShoutMeter, bLockShoutMeter, fShoutMeterPosX/Y, bAltShoutMeter) and the Hide compass / Hide level-up meter options appear when your skyhud.txt has those keys; a control whose key is not in your file is not shown, and a tab with none of its keys is hidden.

## 1.0.1 - 2026-09-05 - untested

### Added
- Added a Skyrim 1.7.99 / 1.7.104 build; the mod now installs as a FOMOD that picks the build for your game version (SE 1.5.97 / AE 1.6.1170, or Skyrim 1.7.x).
