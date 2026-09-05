#include "SkyHudConfig.h"

#include "utils/Logger.h"

#include <fstream>
#include <sstream>

namespace skyhud
{
	namespace
	{
		std::string Trim(std::string_view a_in)
		{
			const auto first = a_in.find_first_not_of(" \t\r\n");
			if (first == std::string_view::npos) {
				return {};
			}
			const auto last = a_in.find_last_not_of(" \t\r\n");
			return std::string{ a_in.substr(first, last - first + 1) };
		}
	}

	bool Config::Load(const char* a_path)
	{
		_lines.clear();
		_loaded = false;
		_dirty = false;
		_path = a_path;

		std::ifstream in(_path, std::ios::binary);
		if (!in) {
			logger::warn("skyhud.txt not found at '{}' - SkyHUD (or a preset providing skyhud.txt) is not installed", _path);
			return false;
		}

		std::string   currentSection;
		std::string   raw;
		std::size_t   keyCount = 0;
		while (std::getline(in, raw)) {
			// getline strips '\n'; a trailing '\r' from CRLF stays in `raw`. Keep the model's
			// stored raw WITHOUT the '\r' (Save re-adds the file's line ending) so comparisons
			// and value edits are clean.
			if (!raw.empty() && raw.back() == '\r') {
				raw.pop_back();
			}

			Line line;
			line.raw = raw;
			line.section = currentSection;

			const std::string trimmed = Trim(raw);
			if (!trimmed.empty() && trimmed.front() == '[' && trimmed.back() == ']') {
				currentSection = trimmed.substr(1, trimmed.size() - 2);
				line.section = currentSection;
			} else if (!trimmed.empty() && trimmed.front() != ';') {
				const auto eq = raw.find('=');
				if (eq != std::string::npos) {
					line.isKey = true;
					line.key = Trim(raw.substr(0, eq));
					line.value = Trim(raw.substr(eq + 1));
					++keyCount;
				}
			}
			_lines.push_back(std::move(line));
		}

		_loaded = true;
		logger::info("Loaded skyhud.txt from '{}': {} line(s), {} setting(s)", _path, _lines.size(), keyCount);
		return true;
	}

	std::optional<std::string> Config::Get(const std::string& a_section, const std::string& a_key) const
	{
		for (const auto& line : _lines) {
			if (line.isKey && (a_section == "*" || line.section == a_section) && line.key == a_key) {
				return line.value;
			}
		}
		return std::nullopt;
	}

	bool Config::Set(const std::string& a_section, const std::string& a_key, const std::string& a_value)
	{
		for (auto& line : _lines) {
			if (line.isKey && (a_section == "*" || line.section == a_section) && line.key == a_key) {
				if (line.value != a_value) {
					line.value = a_value;
					// Rebuild raw as "key=value", preserving any leading indentation the file used.
					const auto keyStart = line.raw.find(line.key);
					const std::string indent = (keyStart != std::string::npos) ? line.raw.substr(0, keyStart) : std::string{};
					line.raw = indent + line.key + "=" + a_value;
					_dirty = true;
				}
				return true;
			}
		}
		logger::warn("skyhud.txt has no [{}] {} to set - refusing to invent it", a_section, a_key);
		return false;
	}

	bool Config::Save() const
	{
		if (!_loaded) {
			return false;
		}
		std::ofstream out(_path, std::ios::binary | std::ios::trunc);
		if (!out) {
			logger::error("Could not open skyhud.txt for writing at '{}'", _path);
			return false;
		}
		// SkyHUD's file is a Windows text file; write CRLF so it round-trips in the shape SkyHUD
		// and users expect.
		for (const auto& line : _lines) {
			out << line.raw << "\r\n";
		}
		_dirty = false;
		logger::info("Wrote skyhud.txt to '{}'", _path);
		return true;
	}
}
