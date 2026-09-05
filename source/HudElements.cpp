#include "HudElements.h"

namespace skyhud
{
	const std::vector<Element>& Elements()
	{
		// Alignment options shared by the text elements.
		static const std::vector<std::pair<const char*, const char*>> kAlign{
			{ "Left", "left" }, { "Center", "center" }, { "Right", "right" }
		};
		static const std::vector<std::pair<const char*, const char*>> kHourFormat{
			{ "12h (3:15)", "h" }, { "12h leading 0 (03:15)", "hh" }, { "24h", "H" }, { "24h leading 0", "HH" }
		};

		static const std::vector<Element> kElements = {
			{ "Health", "fHealth", "bLockHealthMeter",
			  { { "Position", "fHealthPosX", "fHealthPosY" } },
			  { { "Alternative style (left-aligned)", "Interface", "bAltHealth" } }, {} },

			{ "Magicka", "fMagica", "bLockMagickaMeter",
			  { { "Position", "fMagicaPosX", "fMagicaPosY" } }, {}, {} },

			{ "Stamina", "fStamina", "bLockStaminaMeter",
			  { { "Position", "fStaminaPosX", "fStaminaPosY" } },
			  { { "Alternative style (left-aligned)", "Interface", "bAltStamina" } }, {} },

			{ "Charge meters", "fChargeMeter", "bLockLeftChargeMeter",
			  { { "Left", "fLeftChargeMeterPosX", "fLeftChargeMeterPosY" },
				{ "Right", "fRightChargeMeterPosX", "fRightChargeMeterPosY" },
				{ "Alt", "fAltChargeMeterPosX", "fAltChargeMeterPosY" } },
			  { { "Combined charge meter", "Interface", "bAltCharge" } }, {} },

			{ "Compass", "fCompass", "bLockCompass",
			  { { "Position", "fCompassPosX", "fCompassPosY" } },
			  { { "Slim compass", "Interface", "bAltCompass" },
				{ "Hide enemy markers", "Gameplay", "bHideEnemyMarker" },
				{ "Hide location markers", "Gameplay", "bHideLocations" },
				{ "Hide undiscovered locations", "Gameplay", "bHideUndiscoveredLocations" },
				{ "Hide quest markers", "Gameplay", "bHideQuest" } }, {} },

			{ "Compass markers", "fCompassMarker", "", {}, {}, {} },

			{ "Crosshair", "fCrosshair", "bLockCrosshair",
			  { { "Position", "fCrosshairPosX", "fCrosshairPosY" } },
			  { { "Dot crosshair", "Interface", "bDotCrosshair" } }, {} },

			{ "Enemy health", "fEnemyHealth", "bLockEnemyHealth",
			  { { "Position", "fEnemyHealthPosX", "fEnemyHealthPosY" } },
			  { { "Hide enemy health bar", "Gameplay", "bHideEnemyHealth" } }, {} },

			{ "Stealth meter", "fStealthMeter", "bLockStealthMeter",
			  { { "Position", "fStealthMeterPosX", "fStealthMeterPosY" } },
			  { { "Hide the \"Hidden/Detected\" text", "Gameplay", "bHideStealthText" },
				{ "Hide the stealth meter", "Gameplay", "bHideStealthMeter" } }, {} },

			{ "Subtitles", "fSubtitle", "bLockSubtitle",
			  { { "Position", "fSubtitlePosX", "fSubtitlePosY" } }, {}, {} },

			{ "Ammo / arrow count", "fArrowInfo", "bLockArrowInfo",
			  { { "Position", "fArrowInfoPosX", "fArrowInfoPosY" } },
			  { { "Ammo display", "Interface", "bAltArrow" } }, {} },

			{ "Activate prompt", "fActivatePrompt", "bLockActivatePrompt",
			  { { "Button", "fActivateButtonPosX", "fActivateButtonPosY" },
				{ "Name", "fActivateNamePosX", "fActivateNamePosY" },
				{ "Info", "fActivateInfoPosX", "fActivateInfoPosY" },
				{ "Bar", "fActivateBarPosX", "fActivateBarPosY" } },
			  { { "Hide the activate button icon", "Gameplay", "bHideActivateButton" } }, {} },

			{ "Location info", "fLocationInfo", "bLockLocationInfo",
			  { { "Position", "fLocationInfoPosX", "fLocationInfoPosY" } }, {},
			  { { "Text alignment", "Interface", "sLocationInfoAlign", kAlign } } },

			{ "Message info", "fMessageInfo", "bLockMessageInfo",
			  { { "Position", "fMessageInfoPosX", "fMessageInfoPosY" } }, {}, {} },

			{ "Objective text", "fObjectiveText", "bLockObjectiveText",
			  { { "Position", "fObjectiveTextPosX", "fObjectiveTextPosY" } }, {},
			  { { "Text alignment", "Interface", "sObjectiveAlign", kAlign } } },

			{ "Level-up meter", "fLevelUpMeter", "bLockLevelUpMeter",
			  { { "Position", "fLevelUpMeterPosX", "fLevelUpMeterPosY" } }, {}, {} },

			{ "Animated letters", "fAnimatedLetters", "bLockAnimatedLetters",
			  { { "Word wall letters", "", "fAnimLetterPosY" },
				{ "Shout letters", "", "fShoutLetterPosY" } }, {}, {} },

			{ "Clock", "fClock", "bLockClock",
			  { { "Position", "fClockPosX", "fClockPosY" } },
			  { { "Show clock", "Interface", "bShowClock" },
				{ "Show seconds", "Interface", "bShowSeconds" },
				{ "Show AM/PM", "Interface", "bShowTOD" },
				{ "Always show (outside HUD too)", "Interface", "bPersistentClock" } },
			  { { "Hour format", "Interface", "sHourFormat", kHourFormat },
				{ "Alignment", "Interface", "sClockAlign", kAlign } } },

			{ "General", "", "", {},
			  { { "Sync meter fade", "Interface", "bSyncMeterFade" } },
			  { { "Persistent meters", "Gameplay", "uPersistentMeter",
				  { { "Auto", "0" }, { "Always on", "1" }, { "Always off", "2" } } } } },
		};
		return kElements;
	}
}
