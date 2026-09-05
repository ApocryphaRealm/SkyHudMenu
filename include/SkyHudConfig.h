#pragma once

// ============================================================================================
// The skyhud.txt model. SkyHUD (by Fhaarkas, Nexus 463) is configured by a single INI-shaped
// file, Data/Interface/skyhud/skyhud.txt, with sections [Gameplay] [Interface] [Scale]
// [Position] [Version]. This mod edits the user's OWN copy of that file and writes it back; it
// ships and includes NO SkyHUD content (the licence permits sharing skyhud.txt but not the rest
// of the mod). SkyHUD reads the file only when the HUD movie loads, so a write is followed by a
// HUD reload (see HudReload.h) for it to take effect.
//
// The model preserves the file VERBATIM - every comment, blank line and ordering - and rewrites
// only the value of a key that actually changed. A HUD-config file is something users hand-tune
// and share as presets, so round-tripping it losslessly is a hard requirement.
// ============================================================================================

#include <optional>
#include <string>
#include <vector>

namespace skyhud
{
	// One physical line of skyhud.txt. Exactly one of the interpretations applies; `raw` always
	// holds the original text so anything not a key/value survives untouched.
	struct Line
	{
		std::string raw;      // the original line, verbatim (no trailing newline)
		std::string section;  // the [Section] this line sits under ("" before the first header)
		bool        isKey = false;
		std::string key;    // when isKey: the setting name (left of '=')
		std::string value;  // when isKey: the current value (right of '=', trimmed)
	};

	class Config
	{
	public:
		// The resolved path SkyHUD reads: Data/Interface/skyhud/skyhud.txt. Through MO2's VFS a
		// read resolves to the winning mod's copy and a write lands in the overwrite, which then
		// wins - so SkyHUD reads back what we wrote on the next HUD load.
		static constexpr const char* kPath = "Data/Interface/skyhud/skyhud.txt";

		// Load kPath (or a_path). False if the file could not be opened - SkyHUD (or a preset mod
		// that provides skyhud.txt) is then not installed, which the page reports rather than
		// writing a file SkyHUD would ignore.
		bool Load(const char* a_path = kPath);

		[[nodiscard]] bool loaded() const noexcept { return _loaded; }
		[[nodiscard]] const std::string& path() const noexcept { return _path; }

		// Current value of section/key, if present.
		[[nodiscard]] std::optional<std::string> Get(const std::string& a_section, const std::string& a_key) const;

		// Set section/key to a_value in memory. Returns false if that section/key is not in the
		// file (we never invent keys - the file's own schema is authoritative). Marks dirty.
		bool Set(const std::string& a_section, const std::string& a_key, const std::string& a_value);

		[[nodiscard]] bool dirty() const noexcept { return _dirty; }

		// Write the model back to its path, verbatim except for changed values. Clears dirty.
		bool Save() const;

		[[nodiscard]] const std::vector<Line>& lines() const noexcept { return _lines; }

	private:
		std::vector<Line> _lines;
		std::string       _path;
		bool              _loaded = false;
		mutable bool      _dirty = false;
	};
}
