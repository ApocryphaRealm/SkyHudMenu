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
			  { { "Position", "fHealthPosX", "fHealthPosY", "Health", "Health.HealthMeter_mc" } },
			  { { "Alternative style (left-aligned)", "Interface", "bAltHealth" } }, {} },

			{ "Magicka", "fMagica", "bLockMagickaMeter",
			  { { "Position", "fMagicaPosX", "fMagicaPosY", "Magica", "Magica.MagickaMeter_mc" } }, {}, {} },

			{ "Stamina", "fStamina", "bLockStaminaMeter",
			  { { "Position", "fStaminaPosX", "fStaminaPosY", "Stamina", "Stamina.StaminaMeter_mc" } },
			  { { "Alternative style (left-aligned)", "Interface", "bAltStamina" } }, {} },

			{ "Charge meters", "fChargeMeter", "bLockLeftChargeMeter",
			  { { "Left", "fLeftChargeMeterPosX", "fLeftChargeMeterPosY", "BottomLeftLockInstance.LeftHandChargeMeterInstance", "BottomLeftLockInstance.LeftHandChargeMeterInstance.ChargeMeter_mc" },
				{ "Right", "fRightChargeMeterPosX", "fRightChargeMeterPosY", "BottomRightLockInstance.RightHandChargeMeterInstance", "BottomRightLockInstance.RightHandChargeMeterInstance.ChargeMeter_mc" },
				{ "Combined", "fAltChargeMeterPosX", "fAltChargeMeterPosY" } },
			  { { "Combined charge meter", "Interface", "bAltCharge" } }, {} },

			{ "Compass", "fCompass", "bLockCompass",
			  { { "Position", "fCompassPosX", "fCompassPosY", "CompassShoutMeterHolder", "CompassShoutMeterHolder.Compass" } },
			  { { "Slim compass", "Interface", "bAltCompass" },
				{ "Hide the compass (Dragonborn UI)", "*", "bHideCompass" },
				{ "Hide enemy markers", "Gameplay", "bHideEnemyMarker" },
				{ "Hide location markers", "Gameplay", "bHideLocations" },
				{ "Hide undiscovered locations", "Gameplay", "bHideUndiscoveredLocations" },
				{ "Hide quest markers", "Gameplay", "bHideQuest" } }, {} },

			{ "Compass markers", "fCompassMarker", "", {}, {}, {} },

			{ "Crosshair", "fCrosshair", "bLockCrosshair",
			  { { "Position", "fCrosshairPosX", "fCrosshairPosY", "Crosshair" } },
			  { { "Dot crosshair", "Interface", "bDotCrosshair" } }, {} },

			{ "Enemy health", "fEnemyHealth", "bLockEnemyHealth",
			  { { "Position", "fEnemyHealthPosX", "fEnemyHealthPosY", "EnemyHealth_mc" } },
			  { { "Hide enemy health bar", "Gameplay", "bHideEnemyHealth" } }, {} },

			{ "Stealth meter", "fStealthMeter", "bLockStealthMeter",
			  { { "Position", "fStealthMeterPosX", "fStealthMeterPosY", "StealthMeterInstance" } },
			  { { "Hide the \"Hidden/Detected\" text", "Gameplay", "bHideStealthText" },
				{ "Hide the stealth meter", "Gameplay", "bHideStealthMeter" } }, {} },

			{ "Subtitles", "fSubtitle", "bLockSubtitle",
			  { { "Position", "fSubtitlePosX", "fSubtitlePosY", "SubtitleTextHolder" } }, {}, {} },

			{ "Ammo / arrow count", "fArrowInfo", "bLockArrowInfo",
			  { { "Position", "fArrowInfoPosX", "fArrowInfoPosY", "ArrowInfoInstance" } },
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
			  { { "Position", "fMessageInfoPosX", "fMessageInfoPosY", "MessagesBlock" } }, {}, {} },

			{ "Objective text", "fObjectiveText", "bLockObjectiveText",
			  { { "Position", "fObjectiveTextPosX", "fObjectiveTextPosY", "QuestUpdateBaseInstance" } }, {},
			  { { "Text alignment", "Interface", "sObjectiveAlign", kAlign } } },

			{ "Level-up meter", "fLevelUpMeter", "bLockLevelUpMeter",
			  { { "Position", "fLevelUpMeterPosX", "fLevelUpMeterPosY" } },
			  { { "Hide the level-up meter (Dragonborn UI)", "*", "bHideLevelUpMeter" } }, {} },

			// Dragonborn UI separates the shout meter from the compass and adds these keys; the tab
			// appears only when the user's file has them.
			{ "Shout meter", "fShoutMeter", "bLockShoutMeter",
			  { { "Position", "fShoutMeterPosX", "fShoutMeterPosY" } },
			  { { "Separate shout meter (Dragonborn UI)", "*", "bAltShoutMeter" } }, {} },

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
