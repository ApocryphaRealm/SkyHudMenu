#include "DevBenchTool.h"

#include "DevBench/DevBenchAPI.h"
#include "HudReload.h"
#include "SkyHudState.h"
#include "utils/Logger.h"

#include <format>
#include <string>
#include <string_view>

namespace DevBenchTool
{
	namespace
	{
		// Minimal JSON string-field reader (no parser dependency): "key":"value" or "key":value.
		std::string Field(std::string_view a_json, std::string_view a_key)
		{
			const std::string needle = std::string("\"") + std::string(a_key) + "\"";
			auto pos = a_json.find(needle);
			if (pos == std::string_view::npos) {
				return {};
			}
			pos = a_json.find(':', pos + needle.size());
			if (pos == std::string_view::npos) {
				return {};
			}
			++pos;
			while (pos < a_json.size() && (a_json[pos] == ' ' || a_json[pos] == '\t')) {
				++pos;
			}
			if (pos < a_json.size() && a_json[pos] == '"') {
				const auto end = a_json.find('"', pos + 1);
				return std::string(a_json.substr(pos + 1, end - pos - 1));
			}
			const auto end = a_json.find_first_of(",}", pos);
			std::string raw{ a_json.substr(pos, end - pos) };
			while (!raw.empty() && (raw.back() == ' ' || raw.back() == '\t')) {
				raw.pop_back();
			}
			return raw;
		}

		void ConfigTool(void*, const char* a_argsJson, void* a_sink, DevBenchAPI::WriteFn a_write)
		{
			const std::string_view json = a_argsJson ? a_argsJson : "";
			const std::string      op = Field(json, "op");

			if (op.empty() || op == "state") {
				auto& c = state::Config();
				a_write(a_sink, std::format(
									R"({{"ok":true,"op":"state","loaded":{},"path":"{}","dirty":{},"lines":{}}})",
									c.loaded() ? "true" : "false", c.path(), c.dirty() ? "true" : "false", c.lines().size())
									.c_str());
				return;
			}
			if (op == "get") {
				const auto v = state::Config().Get(Field(json, "section"), Field(json, "key"));
				a_write(a_sink, std::format(R"({{"ok":{},"op":"get","value":"{}"}})",
											v ? "true" : "false", v.value_or(""))
									.c_str());
				return;
			}
			if (op == "set") {
				const bool ok = state::Config().Set(Field(json, "section"), Field(json, "key"), Field(json, "value"));
				a_write(a_sink, std::format(R"({{"ok":{},"op":"set"}})", ok ? "true" : "false").c_str());
				return;
			}
			if (op == "save") {
				const bool ok = state::WriteAndApply();
				a_write(a_sink, std::format(R"({{"ok":{},"op":"save"}})", ok ? "true" : "false").c_str());
				return;
			}
			if (op == "reload") {
				const bool ok = state::LoadFromDisk();
				a_write(a_sink, std::format(R"({{"ok":{},"op":"reload"}})", ok ? "true" : "false").c_str());
				return;
			}
			if (op == "hudreload") {
				const bool ok = hudreload::Request();
				a_write(a_sink, std::format(R"({{"ok":{},"op":"hudreload"}})", ok ? "true" : "false").c_str());
				return;
			}
			a_write(a_sink, R"({"ok":false,"error":"unknown op"})");
		}
	}

	void Init(bool a_lastAttempt)
	{
		static bool registered = false;
		if (registered) {
			return;
		}
		DevBenchAPI::IDevBenchInterface001* devBench = DevBenchAPI::GetDevBenchInterface001();
		if (!devBench) {
			if (a_lastAttempt) {
				logger::info("DevBench not detected; skipping the \"skyhud.config\" tool");
			}
			return;
		}
		constexpr const char* descriptor =
			"{"
			"\"description\":\"Drive the SkyHUD config editor for testing. op=state reports whether "
			"skyhud.txt loaded, its path and line count. op=get {section,key} reads a value. "
			"op=set {section,key,value} changes one in memory. op=save writes skyhud.txt and asks "
			"the HUD to reload. op=reload re-reads skyhud.txt from disk. op=hudreload just forces "
			"the HUD reload (to test whether SkyHUD re-reads live).\","
			"\"inputSchema\":{\"type\":\"object\",\"properties\":{\"op\":{\"type\":\"string\"},"
			"\"section\":{\"type\":\"string\"},\"key\":{\"type\":\"string\"},\"value\":{\"type\":\"string\"}}},"
			"\"readOnly\":false"
			"}";
		if (devBench->RegisterTool("skyhud.config", descriptor, &ConfigTool, nullptr)) {
			logger::info("Registered \"skyhud.config\" with DevBench (build {})", devBench->GetBuildNumber());
			registered = true;
		}
	}
}
