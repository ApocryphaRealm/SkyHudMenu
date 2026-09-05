SkyHUD Settings Menu
====================
Version 1.0.0

An in-game settings menu for SkyHUD. It puts SkyHUD's hide / scale / move options - the ones you
normally edit by hand in skyhud.txt - onto a real settings page in the Apocrypha Menu Framework,
one tab per HUD element, and draws a live on-screen marker showing where each element will sit as
you adjust it.

This mod ships NO SkyHUD content. It reads and writes your own skyhud.txt; SkyHUD itself does the
actual HUD work. It configures SkyHUD, it does not replace or include it.


WHAT IT DOES
------------
- One tab per HUD element (Health, Magicka, Stamina, the charge meters, Compass, Crosshair,
  Enemy health, Stealth meter, Subtitles, Ammo, Activate prompt, Location / Message info,
  Objective text, Level-up meter, Animated letters, Clock) plus a General tab.
- Each tab gathers that element's scale, lock and X/Y position, and its SkyHUD options in one
  place, reading and writing your skyhud.txt.
- A live on-screen preview: a labelled marker for every element you have positioned, drawn at the
  spot it will occupy, sized to roughly match the widget's shape, and moving in real time as you
  drag the sliders.
  * Per element you can turn its marker on or off and pick its colour. Health, Stamina and Magicka
    default to their bar colours (red, green, blue); everything else defaults to gold.
  * A master toggle turns the whole preview on or off.
  * The markers show whether the menu is open or closed, so you can see the layout while playing.


HONEST LIMITATION - CHANGES APPLY ON THE NEXT HUD LOAD
------------------------------------------------------
SkyHUD only reads skyhud.txt when the HUD movie loads, so a change you save takes effect the next
time the HUD reloads (load a save) - it does not move the real widget the instant you change it.
That is why the on-screen marker exists: it shows you exactly where an element will go, live, so
you can dial it in before you commit, and the widget snaps there on your next load.


REQUIREMENTS
------------
- SKSE64
- Address Library for SKSE Plugins
- SkyHUD (the mod this configures) - or any mod that provides Data/Interface/skyhud/skyhud.txt
- Apocrypha Menu Framework 1.5.8 or newer (the on-screen preview uses drawing added in 1.5.8)
- DevBench is optional (only for the skyhud.config testing tool)


HOW TO USE
----------
1. Install with your mod manager, after SkyHUD and Apocrypha Menu Framework.
2. In game, open the Apocrypha Menu Framework menu and pick SkyHUD Settings Menu.
3. Choose an element's tab, unlock it, and set its scale / position; the gold (or coloured) marker
   shows where it lands. Turn each element's marker on/off and set its colour on its tab.
4. Press Save. Load a save (or let the HUD reload) for the change to take effect in SkyHUD.


COMPATIBILITY
-------------
Works with any skyhud.txt-based setup, including preset mods that provide their own skyhud.txt -
this menu edits whichever one is active. It needs a framework that answers the SKSE Menu Framework
API and exposes the screen-wide draw list; Apocrypha Menu Framework 1.5.8+ does.


LICENCE
-------
MIT. Original work; ships no SkyHUD files. SkyHUD is by Fhaarkas and is required separately.
Built on the Apocrypha Menu Framework (MIT) and CommonLibSSE-NG. The DevBench C-ABI consumer
header is vendored (MIT); DevBench itself is an optional runtime tool, never linked.


BUILDING
--------
Requires Visual Studio 2022, CMake and a vcpkg checkout (VCPKG_ROOT set). Run configure.bat then
build.bat; the DLL is written to build/relwithdebinfo-se-only.
