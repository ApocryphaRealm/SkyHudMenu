#pragma once

// Address Library guard (1.2.9). oproso on the Nexus page, 16-17 Sep 2026, running Fluorine on
// SteamOS: "wheeler.dll - CommonLibSSE-NG/include/REL/ID(219): failed to open the address library
// file", and the same from our CNO patch. That text is CommonLibSSE-NG's own, raised the first
// time an address is resolved, and it says nothing about WHICH file it wanted, WHERE it looked,
// or what game version it decided on - and under Proton any of those can be the wrong one. So
// before this plugin resolves anything, the same decision CommonLib will make is made here and
// written down: the runtime it detected, the executable it read that from, the file it will open,
// the working directory that path is relative to, and whether the file is there (also beside the
// executable, in case the working directory is the problem). If it is missing, a message names the
// file and the plugin loads inert instead of leaving the player with the bare CommonLib line.
//
// Own code, GPL-3.0-or-later. Meant to be copied into every mod of ours built on CommonLibSSE-NG.

#include <cstdlib>
#include <iterator>
#include <filesystem>
#include <format>
#include <string>

#include <Windows.h>

namespace AddressLibraryGuard
{
	struct Report
	{
		std::string runtime;      // "1-6-1170-0"
		std::string edition;      // "AE", "SE" or "VR"
		std::string exe;          // the executable CommonLib read the version from
		std::string file;         // the relative path CommonLib will open
		std::string cwd;          // what that path is relative to
		bool existsAtCwd = false; // the check that decides CommonLib's outcome
		bool existsAtExe = false; // the same file beside the executable
		bool simulated = false;   // WHEELER_SIMULATE_MISSING_ADDRESS_LIBRARY=1 forced the missing verdict
	};

	inline std::string Narrow(std::wstring_view a_w)
	{
		if (a_w.empty()) { return {}; }
		const int n = WideCharToMultiByte(CP_UTF8, 0, a_w.data(), static_cast<int>(a_w.size()), nullptr, 0, nullptr, nullptr);
		std::string out(static_cast<std::size_t>(n), '\0');
		WideCharToMultiByte(CP_UTF8, 0, a_w.data(), static_cast<int>(a_w.size()), out.data(), n, nullptr, nullptr);
		return out;
	}

	// Mirrors REL::IDDatabase::load() in CommonLibSSE-NG: versionlib-<v>.bin for AE, version-<v>.bin
	// for SE, version-<v>.csv for VR, all under Data/SKSE/Plugins relative to the working directory.
	inline Report Check()
	{
		Report r;
		const auto& mod = REL::Module::get();
		r.runtime = mod.version().string();
		// The full path of the running executable, asked of Windows directly: Module::filePath() can
		// carry the bare file name, and a bare name's parent is the working directory again.
		{
			wchar_t buf[MAX_PATH * 2] = {};
			const DWORD n = GetModuleFileNameW(nullptr, buf, static_cast<DWORD>(std::size(buf)));
			r.exe = n > 0 ? Narrow(std::wstring_view(buf, n)) : Narrow(mod.filePath());
		}
#ifdef ENABLE_SKYRIM_VR
		if (REL::Module::IsVR()) {
			r.edition = "VR";
			r.file = std::format("Data/SKSE/Plugins/version-{}.csv", r.runtime);
		} else
#endif
		if (REL::Module::IsAE()) {
			r.edition = "AE";
			r.file = std::format("Data/SKSE/Plugins/versionlib-{}.bin", r.runtime);
		} else {
			r.edition = "SE";
			r.file = std::format("Data/SKSE/Plugins/version-{}.bin", r.runtime);
		}
		std::error_code ec;
		const auto cwd = std::filesystem::current_path(ec);
		r.cwd = ec ? "(unreadable)" : cwd.string();
		r.existsAtCwd = !ec && std::filesystem::exists(cwd / r.file, ec);
		// Diagnostic only: with this environment variable set the file is reported missing whatever is on
		// disk, so the failure path can be driven on a machine where every other plugin needs the file
		// (removing it for real stops Engine Fixes' preloader before SKSE ever runs, 2026-09-17).
		if (const char* sim = std::getenv("WHEELER_SIMULATE_MISSING_ADDRESS_LIBRARY"); sim && sim[0] == '1') {
			r.existsAtCwd = false;
			r.simulated = true;
		}
		const auto exeDir = std::filesystem::path(r.exe).parent_path();
		r.existsAtExe = std::filesystem::exists(exeDir / r.file, ec);
		return r;
	}

	// Logs the report; when the file CommonLib will open is missing, shows a message naming it and
	// returns false so the caller can load inert. Never resolves an address itself.
	inline bool Guard(const char* a_modName)
	{
		const Report r = Check();
		logger::info("[AddressLibrary] runtime {} ({}) read from \"{}\"; will open \"{}\" relative to \"{}\": {}{}{}",
			r.runtime, r.edition, r.exe, r.file, r.cwd, r.existsAtCwd ? "present" : "MISSING", r.simulated ? " (SIMULATED by environment variable)" : "",
			r.existsAtCwd == r.existsAtExe ? "" : (r.existsAtExe ? " (but present beside the executable - the working directory is wrong)" : " (and missing beside the executable too)"));
		if (r.existsAtCwd) {
			return true;
		}
		const std::string text = std::format(
			"{} cannot start: the Address Library file for this game version is missing.\n\n"
			"Game version detected: {} ({})\nRead from: {}\nFile needed: {}\nLooked in: {}\n{}\n\n"
			"Install Address Library for SKSE Plugins for your game edition ({} and {} are separate downloads) "
			"and make sure that file reaches Data\\SKSE\\Plugins. The plugin has loaded inert; the game continues.",
			a_modName, r.runtime, r.edition, r.exe, r.file, r.cwd,
			r.existsAtExe ? "The file IS beside the executable: the game's working directory is not its folder." : "The file is not beside the executable either.",
			"SE 1.5.97", "AE 1.6.1170");
		logger::critical("[AddressLibrary] {}", text);
		MessageBoxA(nullptr, text.c_str(), a_modName, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		return false;
	}
}
