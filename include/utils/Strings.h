#pragma once

// ============================================================================================
// LANGUAGE SUPPORT for a mod whose settings page is drawn on the Apocrypha Menu Framework.
//
// Vendored unchanged into <mod>\include\utils\Strings.h (canonical copy:
// D:\Claude output\.MD\templates\consumer-Strings.h). Header-only; C++17 inline state.
//
// Every string the page draws goes through TR("PREFIX_Key", "English text"). The English text is
// the compiled fallback; the shown text comes from
//     Data\Interface\Translations\<stem>_<language>.txt
// (UTF-16LE with BOM, one "$key<TAB>text" per line, "\n" for a line break - the SKSE/SkyUI
// shape, the same file the framework reads for its own text and, since AMF 1.6.5, the file it
// feeds its font atlas from, so the glyphs this page needs are already rasterised).
//
// Which language: the framework's - AMF_GetLanguage() from the framework module - so one
// Language setting (the framework's Settings page, or its INI) moves every mod's page at once.
// Without that export (an older framework, or none) the game's own sLanguage:General is used.
// A file for that language is read; anything it lacks falls back to the English file, then to
// the compiled text - a half-finished translation never shows a raw key.
//
// Call Configure(stem) once at kDataLoaded, and Tick() as the first line of each page's render
// callback: it compares the active language with the loaded one (one strcmp per drawn frame) and
// reloads only on a change. There is no background work - the page drawing is the event.
// ============================================================================================

#include "utils/Logger.h"

#include <windows.h>

#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>

namespace strings
{
	namespace detail
	{
		inline std::mutex g_lock;
		inline std::string g_stem;                                  // "SkyHudMenu"
		inline std::unordered_map<std::string, std::string> g_texts; // key (without '$') -> UTF-8
		inline std::string g_language = "english";                   // what is loaded
		inline std::string g_source = "compiled";                    // "framework" | "game" | "compiled"
		inline std::string g_file;                                   // the file the language came from, if any
		inline int g_count = 0;
		inline bool g_loadedOnce = false;
		inline bool g_noExportLogged = false;

		using GetLanguageFn = const char* (*)();

		inline GetLanguageFn ResolveFrameworkExport()
		{
			static GetLanguageFn s_fn = nullptr;
			static bool s_tried = false;
			if (s_fn || s_tried) { return s_fn; }
			// Not sticky on failure: the framework loads before consumers by its sort-first name, but a
			// first miss (an odd load order) is retried next frame rather than treated as permanent.
			HMODULE m = GetModuleHandleW(L"!ApocryphaMenuFramework");
			if (!m) { m = GetModuleHandleW(L"ApocryphaMenuFramework"); }
			if (!m) { return nullptr; }
			s_tried = true;
			s_fn = reinterpret_cast<GetLanguageFn>(GetProcAddress(m, "AMF_GetLanguage"));
			return s_fn;
		}

		inline std::string Lower(std::string a_s)
		{
			for (auto& c : a_s) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
			return a_s;
		}

		inline std::string GameLanguage()
		{
			std::string lang = "english";
			if (auto* ini = RE::INISettingCollection::GetSingleton())
			{
				if (auto* setting = ini->GetSetting("sLanguage:General"); setting && setting->GetString() && setting->GetString()[0])
				{
					lang = setting->GetString();
				}
			}
			return Lower(lang);
		}

		inline std::filesystem::path FileFor(const std::string& a_language)
		{
			return std::filesystem::current_path() / "Data" / "Interface" / "Translations" / (g_stem + "_" + a_language + ".txt");
		}

		inline std::string ToUtf8(const std::wstring& a_w)
		{
			if (a_w.empty()) { return {}; }
			const int n = WideCharToMultiByte(CP_UTF8, 0, a_w.data(), static_cast<int>(a_w.size()), nullptr, 0, nullptr, nullptr);
			std::string out(static_cast<std::size_t>(n), '\0');
			WideCharToMultiByte(CP_UTF8, 0, a_w.data(), static_cast<int>(a_w.size()), out.data(), n, nullptr, nullptr);
			return out;
		}

		// -1 = no file, -2 = not UTF-16LE+BOM, else the number of entries added.
		inline int ReadInto(const std::filesystem::path& a_path, std::unordered_map<std::string, std::string>& a_out, bool a_overwrite)
		{
			std::ifstream in(a_path, std::ios::binary);
			if (!in) { return -1; }
			std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			if (bytes.size() < 2 || static_cast<unsigned char>(bytes[0]) != 0xFF || static_cast<unsigned char>(bytes[1]) != 0xFE) { return -2; }
			std::wstring text(reinterpret_cast<const wchar_t*>(bytes.data() + 2), (bytes.size() - 2) / 2);
			int added = 0;
			std::size_t pos = 0;
			while (pos < text.size())
			{
				auto eol = text.find(L'\n', pos);
				if (eol == std::wstring::npos) { eol = text.size(); }
				std::wstring line = text.substr(pos, eol - pos);
				pos = eol + 1;
				if (!line.empty() && line.back() == L'\r') { line.pop_back(); }
				if (line.empty() || line[0] != L'$') { continue; }
				const auto tab = line.find(L'\t');
				if (tab == std::wstring::npos) { continue; }
				std::string key = ToUtf8(line.substr(1, tab - 1));
				std::wstring value = line.substr(tab + 1);
				for (std::size_t i = 0; i + 1 < value.size(); ++i)
				{
					if (value[i] == L'\\' && value[i + 1] == L'n') { value.replace(i, 2, L"\n"); }
				}
				if (!a_overwrite && a_out.count(key)) { continue; }
				a_out[key] = ToUtf8(value);
				++added;
			}
			return added;
		}

		inline void Load(const std::string& a_language, const char* a_source)
		{
			std::unordered_map<std::string, std::string> texts;
			const int fromLang = ReadInto(FileFor(a_language), texts, true);
			int fromEnglish = 0;
			if (a_language != "english") { fromEnglish = ReadInto(FileFor("english"), texts, false); }
			{
				std::scoped_lock l(g_lock);
				g_texts = std::move(texts);
				g_language = a_language;
				g_source = a_source;
				g_count = fromLang > 0 ? fromLang : 0;
				g_file = fromLang > 0 ? FileFor(a_language).filename().string() : (fromEnglish > 0 ? FileFor("english").filename().string() : "");
				g_loadedOnce = true;
			}
			if (fromLang == -2)
			{
				logger::warn("strings: {} is not UTF-16LE with a BOM (the SKSE translation format) and was ignored", FileFor(a_language).filename().string());
			}
			else if (fromLang < 0)
			{
				logger::info("strings: no translation file for \"{}\" ({}); {}", a_language, FileFor(a_language).filename().string(),
							 fromEnglish > 0 ? "the English file is used" : "the compiled English text is used");
			}
			else
			{
				logger::info("strings: {} text(s) read for \"{}\" from {} ({} language){}", fromLang, a_language, FileFor(a_language).filename().string(),
							 a_source, fromEnglish > 0 ? " - the rest from the English file" : "");
			}
		}
	}

	// Once, at kDataLoaded: the file stem (the DLL's name). Loads for the language active right now.
	inline void Configure(const char* a_fileStem)
	{
		detail::g_stem = a_fileStem ? a_fileStem : "";
		if (auto fn = detail::ResolveFrameworkExport()) { detail::Load(detail::Lower(fn()), "framework"); }
		else { detail::Load(detail::GameLanguage(), "game"); }
	}

	// First line of every page render callback. Reloads only when the framework's language changed.
	inline void Tick()
	{
		if (detail::g_stem.empty()) { return; }
		const char* source = "framework";
		std::string lang;
		if (auto fn = detail::ResolveFrameworkExport()) { lang = detail::Lower(fn()); }
		else
		{
			source = "game";
			if (!detail::g_noExportLogged) { detail::g_noExportLogged = true; logger::debug("strings: AMF_GetLanguage is not exported by the loaded framework; following the game's sLanguage"); }
			lang = detail::GameLanguage();
		}
		if (!detail::g_loadedOnce || lang != detail::g_language) { detail::Load(lang, source); }
	}

	// The text for a key in the active language, or a_english when no file has it.
	inline const char* TR(const char* a_key, const char* a_english)
	{
		std::scoped_lock l(detail::g_lock);
		const auto it = detail::g_texts.find(a_key);
		return it == detail::g_texts.end() ? a_english : it->second.c_str();
	}

	inline const std::string& Language() { return detail::g_language; }
	inline int Count() { return detail::g_count; }

	// For the mod's DevBench tool (op=strings): {"language","source","count","file"}.
	inline std::string StatusJson()
	{
		std::scoped_lock l(detail::g_lock);
		char buf[512];
		std::snprintf(buf, sizeof(buf), "{\"language\":\"%s\",\"source\":\"%s\",\"count\":%d,\"file\":\"%s\"}",
					  detail::g_language.c_str(), detail::g_source.c_str(), detail::g_count, detail::g_file.c_str());
		return buf;
	}
}
