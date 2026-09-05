#include "Settings.h"

#include "HudElements.h"
#include "HudPreview.h"

#include <cctype>
#include <fstream>
#include <sstream>

namespace settings
{
	namespace
	{
		std::string iniPath;

		std::string ResolvePath(const std::string& a_iniFileName)
		{
			return "Data/SKSE/Plugins/" + a_iniFileName;
		}

		logger::level ClampLevel(std::uint32_t a_raw)
		{
			return a_raw <= static_cast<std::uint32_t>(logger::level::off)
					   ? static_cast<logger::level>(a_raw)
					   : logger::level::trace;
		}

		// A stable INI key from an element name: letters and digits only ("Ammo / arrow count" ->
		// "Ammoarrowcount"). Used so per-element ghost prefs round-trip by name, not position.
		std::string Key(const std::string& a_name)
		{
			std::string k;
			for (const char c : a_name) {
				if (std::isalnum(static_cast<unsigned char>(c))) {
					k.push_back(c);
				}
			}
			return k;
		}

		// Ensure the per-element vector is sized to the element list, each with a distinct default
		// colour (cycling the palette). Called before reading overrides.
		void EnsureDefaults()
		{
			auto&      ghosts = preview::Ghosts();
			const auto count = skyhud::Elements().size();
			const auto& els = skyhud::Elements();
			ghosts.assign(count, {});
			for (std::size_t i = 0; i < count; ++i) {
				ghosts[i].show = true;
				ghosts[i].color = ::preview::DefaultColorFor(els[i].name);
			}
		}

		int ElementIndex(const std::string& a_keyName)
		{
			const auto& els = skyhud::Elements();
			for (std::size_t i = 0; i < els.size(); ++i) {
				if (Key(els[i].name) == a_keyName) {
					return static_cast<int>(i);
				}
			}
			return -1;
		}
	}

	namespace preview
	{
		std::vector<GhostPref>& Ghosts()
		{
			static std::vector<GhostPref> g;
			return g;
		}
	}

	void Reload()
	{
		EnsureDefaults();

		std::ifstream in(iniPath);
		if (!in) {
			return;  // no INI yet: keep the compiled defaults
		}
		std::string line;
		while (std::getline(in, line)) {
			const auto eq = line.find('=');
			if (eq == std::string::npos) {
				continue;
			}
			const std::string key = line.substr(0, eq);
			const std::string val = line.substr(eq + 1);

			if (key.find("uLogLevel") != std::string::npos) {
				try {
					debug::logLevel = ClampLevel(static_cast<std::uint32_t>(std::stoul(val)));
				} catch (...) {
				}
			} else if (key.find("bShowPreview") != std::string::npos) {
				preview::show = val.find('0') == std::string::npos;
			} else if (key.size() > 6 && key.compare(key.size() - 6, 6, "_Ghost") == 0) {
				const int idx = ElementIndex(key.substr(0, key.size() - 6));
				if (idx >= 0) {
					preview::Ghosts()[static_cast<std::size_t>(idx)].show = val.find('0') == std::string::npos;
				}
			} else if (key.size() > 6 && key.compare(key.size() - 6, 6, "_Color") == 0) {
				const int idx = ElementIndex(key.substr(0, key.size() - 6));
				if (idx >= 0) {
					try {
						preview::Ghosts()[static_cast<std::size_t>(idx)].color = std::stoi(val);
					} catch (...) {
					}
				}
			}
		}
	}

	void Init(const std::string& a_iniFileName)
	{
		iniPath = ResolvePath(a_iniFileName);
		Reload();
	}

	bool Save()
	{
		std::ofstream out(iniPath, std::ios::trunc);
		if (!out) {
			return false;
		}
		out << "[Debug]\r\n";
		out << "; Log verbosity: 0 trace, 1 debug, 2 info, 3 warn, 4 error, 5 critical, 6 off.\r\n";
		out << "uLogLevel=" << static_cast<std::uint32_t>(debug::logLevel) << "\r\n\r\n";
		out << "[Preview]\r\n";
		out << "; bShowPreview - master on/off for the on-screen position markers.\r\n";
		out << "bShowPreview=" << (preview::show ? 1 : 0) << "\r\n";
		out << "; Per element: <Element>_Ghost (1 shown / 0 hidden) and <Element>_Color (palette index).\r\n";
		const auto& els = skyhud::Elements();
		auto&       ghosts = preview::Ghosts();
		for (std::size_t i = 0; i < els.size() && i < ghosts.size(); ++i) {
			const std::string k = Key(els[i].name);
			out << k << "_Ghost=" << (ghosts[i].show ? 1 : 0) << "\r\n";
			out << k << "_Color=" << ghosts[i].color << "\r\n";
		}
		return true;
	}

	const std::string& GetIniPath() { return iniPath; }
}
