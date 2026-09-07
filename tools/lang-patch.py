# -*- coding: utf-8 -*-
"""lang-patch.py - one-shot, re-runnable language-support patch for SkyHUD Settings Menu.

Applies the consumer-side mechanism from D:\\Claude output\\4. plans\\translation-rollout\\plan.md
sections 2 and 4.1: strings::TR() routing for every literal the settings page draws, the
"!ApocryphaMenuFramework" module-name lookup, strings::Configure() at kDataLoaded, and a
"strings" DevBench op. Every edit below is a must-match anchor replace: if an anchor is not
found EXACTLY ONCE the script raises instead of silently doing nothing, so a stale run against
changed source fails loudly rather than leaving the code half patched.

Run from anywhere: `python tools/lang-patch.py` (paths are relative to the repo root, taken as
this script's grandparent directory).
"""
import os
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def read(path):
    # Universal-newline read: a CRLF-vendored file (SKSEMenuFramework.h) and the repo's own
    # LF sources both come back with plain "\n", so every anchor below is written once, in "\n".
    with open(path, "r", encoding="utf-8", newline=None) as f:
        return f.read()


def write(path, text, crlf=False):
    with open(path, "w", encoding="utf-8", newline="\r\n" if crlf else "\n") as f:
        f.write(text)


def apply_one(text, anchor, replacement, label):
    n = text.count(anchor)
    if n != 1:
        raise RuntimeError(f"[{label}] anchor found {n} time(s), expected exactly 1:\n{anchor!r}")
    return text.replace(anchor, replacement, 1)


# ------------------------------------------------------------------------------------------------
# 1) include/SKSEMenuFramework.h - "!ApocryphaMenuFramework" first, ahead of the alias name.
# ------------------------------------------------------------------------------------------------
def patch_skse_menu_framework_h():
    path = os.path.join(REPO, "include", "SKSEMenuFramework.h")
    text = read(path)
    anchor = (
        "inline HMODULE GetMenuFrameworkModule() {\n"
        "    static HMODULE menuFramework = nullptr;\n"
        "    if (!menuFramework) {\n"
        "        menuFramework = GetModuleHandleW(L\"ApocryphaMenuFramework\");\n"
        "        if (!menuFramework) {\n"
        "            menuFramework = GetModuleHandleW(L\"SKSEMenuFramework\");\n"
        "        }\n"
        "    }\n"
        "    return menuFramework;\n"
        "}"
    )
    replacement = (
        "inline HMODULE GetMenuFrameworkModule() {\n"
        "    static HMODULE menuFramework = nullptr;\n"
        "    if (!menuFramework) {\n"
        "        menuFramework = GetModuleHandleW(L\"!ApocryphaMenuFramework\");\n"
        "        if (!menuFramework) {\n"
        "            menuFramework = GetModuleHandleW(L\"ApocryphaMenuFramework\");\n"
        "        }\n"
        "        if (!menuFramework) {\n"
        "            menuFramework = GetModuleHandleW(L\"SKSEMenuFramework\");\n"
        "        }\n"
        "    }\n"
        "    return menuFramework;\n"
        "}"
    )
    text = apply_one(text, anchor, replacement, "SKSEMenuFramework.h:GetMenuFrameworkModule")
    write(path, text, crlf=True)


# ------------------------------------------------------------------------------------------------
# 2) source/MessageListeners.cpp - strings::Configure("SkyHudMenu") at kDataLoaded.
# ------------------------------------------------------------------------------------------------
def patch_message_listeners_cpp():
    path = os.path.join(REPO, "source", "MessageListeners.cpp")
    text = read(path)

    text = apply_one(
        text,
        "#include \"utils/Logger.h\"\n\n#include <SKSE/SKSE.h>",
        "#include \"utils/Logger.h\"\n#include \"utils/Strings.h\"\n\n#include <SKSE/SKSE.h>",
        "MessageListeners.cpp:include",
    )

    anchor = (
        "\tcase SKSE::MessagingInterface::kDataLoaded:\n"
        "\t\t// skyhud.txt is a loose Data file; it is readable by now. Load it so the page and the\n"
        "\t\t// DevBench tool have it in hand, and make the last DevBench registration attempt.\n"
        "\t\tstate::LoadFromDisk();\n"
        "\t\tDevBenchTool::Init(/* a_lastAttempt = */ true);\n"
        "\t\tbreak;"
    )
    replacement = (
        "\tcase SKSE::MessagingInterface::kDataLoaded:\n"
        "\t\t// skyhud.txt is a loose Data file; it is readable by now. Load it so the page and the\n"
        "\t\t// DevBench tool have it in hand, and make the last DevBench registration attempt.\n"
        "\t\tstate::LoadFromDisk();\n"
        "\t\tstrings::Configure(\"SkyHudMenu\");\n"
        "\t\tDevBenchTool::Init(/* a_lastAttempt = */ true);\n"
        "\t\tbreak;"
    )
    text = apply_one(text, anchor, replacement, "MessageListeners.cpp:kDataLoaded")
    write(path, text)


# ------------------------------------------------------------------------------------------------
# 3) source/DevBenchTool.cpp - a "strings" op returning strings::StatusJson(); descriptor updated.
# ------------------------------------------------------------------------------------------------
def patch_devbench_tool_cpp():
    path = os.path.join(REPO, "source", "DevBenchTool.cpp")
    text = read(path)

    text = apply_one(
        text,
        "#include \"SkyHudState.h\"\n#include \"utils/Logger.h\"",
        "#include \"SkyHudState.h\"\n#include \"utils/Logger.h\"\n#include \"utils/Strings.h\"",
        "DevBenchTool.cpp:include",
    )

    anchor = (
        "\t\t\tif (op == \"clips\") {\n"
        "\t\t\t\ta_write(a_sink, std::format(R\"({{\"ok\":true,\"op\":\"clips\",\"clips\":{}}})\", preview::LiveReport()).c_str());\n"
        "\t\t\t\treturn;\n"
        "\t\t\t}\n"
        "\t\t\ta_write(a_sink, R\"({\"ok\":false,\"error\":\"unknown op\"})\");"
    )
    replacement = (
        "\t\t\tif (op == \"clips\") {\n"
        "\t\t\t\ta_write(a_sink, std::format(R\"({{\"ok\":true,\"op\":\"clips\",\"clips\":{}}})\", preview::LiveReport()).c_str());\n"
        "\t\t\t\treturn;\n"
        "\t\t\t}\n"
        "\t\t\tif (op == \"strings\") {\n"
        "\t\t\t\ta_write(a_sink, std::format(R\"({{\"ok\":true,\"op\":\"strings\",\"strings\":{}}})\", strings::StatusJson()).c_str());\n"
        "\t\t\t\treturn;\n"
        "\t\t\t}\n"
        "\t\t\ta_write(a_sink, R\"({\"ok\":false,\"error\":\"unknown op\"})\");"
    )
    text = apply_one(text, anchor, replacement, "DevBenchTool.cpp:ConfigTool ops")

    text = apply_one(
        text,
        "\"op=clips reads every known widget's live position, bounds and scale from the HUD movie.\\\",\"",
        "\"op=clips reads every known widget's live position, bounds and scale from the HUD movie. \"\n\t\t\t\"op=strings reports the active language, source and loaded translation count.\\\",\"",
        "DevBenchTool.cpp:descriptor",
    )
    write(path, text)


# ------------------------------------------------------------------------------------------------
# 4) source/UI.cpp - route every drawn literal through strings::TR(), tab/label keys derived from
#    the HudElements.cpp table text via DeriveKey (matched exactly by tools/gen-translations.py).
# ------------------------------------------------------------------------------------------------
def patch_ui_cpp():
    path = os.path.join(REPO, "source", "UI.cpp")
    text = read(path)

    text = apply_one(
        text,
        "#include \"utils/Toggle.h\"\n#include \"utils/Logger.h\"",
        "#include \"utils/Toggle.h\"\n#include \"utils/Logger.h\"\n#include \"utils/Strings.h\"",
        "UI.cpp:include",
    )

    # --- DeriveKey helper, placed right after g_status ------------------------------------------
    text = apply_one(
        text,
        "\tnamespace\n\t{\n\t\tstd::string g_status;\n",
        "\tnamespace\n\t{\n\t\tstd::string g_status;\n\n"
        "\t\t// The same deterministic label->key mapping tools/gen-translations.py uses for every\n"
        "\t\t// HudElements.cpp table label (tab names, position/toggle/dropdown labels and dropdown\n"
        "\t\t// option text): each run of alphanumerics becomes one word, capitalised on its first\n"
        "\t\t// character, words are concatenated, and the prefix is prepended verbatim. Two labels\n"
        "\t\t// with the same text (e.g. every plain \"Position\" tab) share one key on purpose - one\n"
        "\t\t// translation serves both.\n"
        "\t\tstd::string DeriveKey(const char* a_prefix, const char* a_text)\n"
        "\t\t{\n"
        "\t\t\tstd::string out{ a_prefix };\n"
        "\t\t\tbool        startWord = true;\n"
        "\t\t\tfor (const char* p = a_text ? a_text : \"\"; *p; ++p) {\n"
        "\t\t\t\tconst unsigned char c = static_cast<unsigned char>(*p);\n"
        "\t\t\t\tif (std::isalnum(c)) {\n"
        "\t\t\t\t\tout += startWord ? static_cast<char>(std::toupper(c)) : static_cast<char>(c);\n"
        "\t\t\t\t\tstartWord = false;\n"
        "\t\t\t\t} else {\n"
        "\t\t\t\t\tstartWord = true;\n"
        "\t\t\t\t}\n"
        "\t\t\t}\n"
        "\t\t\treturn out;\n"
        "\t\t}\n",
        "UI.cpp:DeriveKey",
    )

    # --- PositionControl: label + X/Y id-suffixed inputs -----------------------------------------
    text = apply_one(
        text,
        "\t\tvoid PositionControl(const skyhud::PosPair& a_pos)\n"
        "\t\t{\n"
        "\t\t\tif (!HasKey(\"Position\", a_pos.xKey) && !HasKey(\"Position\", a_pos.yKey)) { return; }\n"
        "\t\t\tImGuiMCP::TextDisabled(\"%s\", a_pos.label);\n"
        "\t\t\tif (HasKey(\"Position\", a_pos.xKey)) {\n"
        "\t\t\t\tfloat x = ValueAsFloat(\"Position\", a_pos.xKey, 0.0F);\n"
        "\t\t\t\tconst std::string lbl = std::string(\"X##\") + a_pos.xKey;\n"
        "\t\t\t\tif (ImGuiMCP::InputFloat(lbl.c_str(), &x, 1.0F, 10.0F, \"%.0f\")) {\n"
        "\t\t\t\t\tstate::Config().Set(\"Position\", a_pos.xKey, FloatToValue(x));\n"
        "\t\t\t\t}\n"
        "\t\t\t}\n"
        "\t\t\tif (HasKey(\"Position\", a_pos.yKey)) {\n"
        "\t\t\t\tfloat y = ValueAsFloat(\"Position\", a_pos.yKey, 0.0F);\n"
        "\t\t\t\tconst std::string lbl = std::string(\"Y##\") + a_pos.yKey;\n"
        "\t\t\t\tif (ImGuiMCP::InputFloat(lbl.c_str(), &y, 1.0F, 10.0F, \"%.0f\")) {\n"
        "\t\t\t\t\tstate::Config().Set(\"Position\", a_pos.yKey, FloatToValue(y));\n"
        "\t\t\t\t}\n"
        "\t\t\t}\n"
        "\t\t}\n",
        "\t\tvoid PositionControl(const skyhud::PosPair& a_pos)\n"
        "\t\t{\n"
        "\t\t\tif (!HasKey(\"Position\", a_pos.xKey) && !HasKey(\"Position\", a_pos.yKey)) { return; }\n"
        "\t\t\tImGuiMCP::TextDisabled(\"%s\", strings::TR(DeriveKey(\"SHM_El_\", a_pos.label).c_str(), a_pos.label));\n"
        "\t\t\tif (HasKey(\"Position\", a_pos.xKey)) {\n"
        "\t\t\t\tfloat x = ValueAsFloat(\"Position\", a_pos.xKey, 0.0F);\n"
        "\t\t\t\tconst std::string lbl = std::string(strings::TR(\"SHM_X\", \"X\")) + \"##\" + a_pos.xKey;\n"
        "\t\t\t\tif (ImGuiMCP::InputFloat(lbl.c_str(), &x, 1.0F, 10.0F, \"%.0f\")) {\n"
        "\t\t\t\t\tstate::Config().Set(\"Position\", a_pos.xKey, FloatToValue(x));\n"
        "\t\t\t\t}\n"
        "\t\t\t}\n"
        "\t\t\tif (HasKey(\"Position\", a_pos.yKey)) {\n"
        "\t\t\t\tfloat y = ValueAsFloat(\"Position\", a_pos.yKey, 0.0F);\n"
        "\t\t\t\tconst std::string lbl = std::string(strings::TR(\"SHM_Y\", \"Y\")) + \"##\" + a_pos.yKey;\n"
        "\t\t\t\tif (ImGuiMCP::InputFloat(lbl.c_str(), &y, 1.0F, 10.0F, \"%.0f\")) {\n"
        "\t\t\t\t\tstate::Config().Set(\"Position\", a_pos.yKey, FloatToValue(y));\n"
        "\t\t\t\t}\n"
        "\t\t\t}\n"
        "\t\t}\n",
        "UI.cpp:PositionControl",
    )

    # --- ToggleControl: label -----------------------------------------------------------------
    text = apply_one(
        text,
        "\t\tvoid ToggleControl(const skyhud::ToggleField& a_t)\n"
        "\t\t{\n"
        "\t\t\tif (!HasKey(a_t.section, a_t.key)) { return; }\n"
        "\t\t\tbool on = ValueAsBool(a_t.section, a_t.key);\n"
        "\t\t\tif (ImGuiMCP::Toggle(a_t.label, &on)) {\n"
        "\t\t\t\tstate::Config().Set(a_t.section, a_t.key, on ? \"1\" : \"0\");\n"
        "\t\t\t}\n"
        "\t\t}\n",
        "\t\tvoid ToggleControl(const skyhud::ToggleField& a_t)\n"
        "\t\t{\n"
        "\t\t\tif (!HasKey(a_t.section, a_t.key)) { return; }\n"
        "\t\t\tbool on = ValueAsBool(a_t.section, a_t.key);\n"
        "\t\t\tif (ImGuiMCP::Toggle(strings::TR(DeriveKey(\"SHM_El_\", a_t.label).c_str(), a_t.label), &on)) {\n"
        "\t\t\t\tstate::Config().Set(a_t.section, a_t.key, on ? \"1\" : \"0\");\n"
        "\t\t\t}\n"
        "\t\t}\n",
        "UI.cpp:ToggleControl",
    )

    # --- DropdownControl: label + option labels -------------------------------------------------
    text = apply_one(
        text,
        "\t\tvoid DropdownControl(const skyhud::DropdownField& a_d)\n"
        "\t\t{\n"
        "\t\t\tif (!HasKey(a_d.section, a_d.key)) { return; }\n"
        "\t\t\tconst auto current = state::Config().Get(a_d.section, a_d.key).value_or(\"\");\n"
        "\t\t\tint         index = 0;\n"
        "\t\t\tstd::vector<const char*> labels;\n"
        "\t\t\tlabels.reserve(a_d.options.size());\n"
        "\t\t\tfor (std::size_t i = 0; i < a_d.options.size(); ++i) {\n"
        "\t\t\t\tlabels.push_back(a_d.options[i].first);\n"
        "\t\t\t\tif (current == a_d.options[i].second) {\n"
        "\t\t\t\t\tindex = static_cast<int>(i);\n"
        "\t\t\t\t}\n"
        "\t\t\t}\n"
        "\t\t\tif (ImGuiMCP::Combo(a_d.label, &index, labels.data(), static_cast<int>(labels.size()))) {\n"
        "\t\t\t\tstate::Config().Set(a_d.section, a_d.key, a_d.options[static_cast<std::size_t>(index)].second);\n"
        "\t\t\t}\n"
        "\t\t}\n",
        "\t\tvoid DropdownControl(const skyhud::DropdownField& a_d)\n"
        "\t\t{\n"
        "\t\t\tif (!HasKey(a_d.section, a_d.key)) { return; }\n"
        "\t\t\tconst auto current = state::Config().Get(a_d.section, a_d.key).value_or(\"\");\n"
        "\t\t\tint         index = 0;\n"
        "\t\t\t// Option text is rebuilt from TR'd entries every frame (plan 2.2); labelStore owns the\n"
        "\t\t\t// translated bytes for this call so the const char* pointers handed to Combo stay valid.\n"
        "\t\t\tstd::vector<std::string> labelStore;\n"
        "\t\t\tlabelStore.reserve(a_d.options.size());\n"
        "\t\t\tfor (std::size_t i = 0; i < a_d.options.size(); ++i) {\n"
        "\t\t\t\tlabelStore.push_back(strings::TR(DeriveKey(\"SHM_El_\", a_d.options[i].first).c_str(), a_d.options[i].first));\n"
        "\t\t\t\tif (current == a_d.options[i].second) {\n"
        "\t\t\t\t\tindex = static_cast<int>(i);\n"
        "\t\t\t\t}\n"
        "\t\t\t}\n"
        "\t\t\tstd::vector<const char*> labels;\n"
        "\t\t\tlabels.reserve(labelStore.size());\n"
        "\t\t\tfor (const auto& s : labelStore) { labels.push_back(s.c_str()); }\n"
        "\t\t\tif (ImGuiMCP::Combo(strings::TR(DeriveKey(\"SHM_El_\", a_d.label).c_str(), a_d.label), &index, labels.data(), static_cast<int>(labels.size()))) {\n"
        "\t\t\t\tstate::Config().Set(a_d.section, a_d.key, a_d.options[static_cast<std::size_t>(index)].second);\n"
        "\t\t\t}\n"
        "\t\t}\n",
        "UI.cpp:DropdownControl",
    )

    # --- RenderElement: ghost toggle, locked toggle, "Position"/"Options" section text -----------
    text = apply_one(
        text,
        "\t\t\t\tif (ImGuiMCP::Toggle(\"Show this ghost on screen\", &gshow)) {",
        "\t\t\t\tif (ImGuiMCP::Toggle(strings::TR(\"SHM_ShowGhost\", \"Show this ghost on screen\"), &gshow)) {",
        "UI.cpp:ShowGhost",
    )
    text = apply_one(
        text,
        "\t\t\t\t\tif (ImGuiMCP::Combo(\"Ghost colour\", &ci, colors.data(), static_cast<int>(colors.size()))) {",
        "\t\t\t\t\tif (ImGuiMCP::Combo(strings::TR(\"SHM_GhostColour\", \"Ghost colour\"), &ci, colors.data(), static_cast<int>(colors.size()))) {",
        "UI.cpp:GhostColour",
    )
    text = apply_one(
        text,
        "\t\t\t\tif (ImGuiMCP::Toggle(\"Locked (use SkyHUD's default position)\", &locked)) {",
        "\t\t\t\tif (ImGuiMCP::Toggle(strings::TR(\"SHM_Locked\", \"Locked (use SkyHUD's default position)\"), &locked)) {",
        "UI.cpp:Locked",
    )
    text = apply_one(
        text,
        "\t\t\t\t\tImGuiMCP::SeparatorText(\"Position\");",
        "\t\t\t\t\tImGuiMCP::SeparatorText(strings::TR(\"SHM_Position\", \"Position\"));",
        "UI.cpp:SeparatorText Position",
    )
    text = apply_one(
        text,
        "\t\t\t\tImGuiMCP::SeparatorText(\"Options\");",
        "\t\t\t\tImGuiMCP::SeparatorText(strings::TR(\"SHM_Options\", \"Options\"));",
        "UI.cpp:SeparatorText Options",
    )

    # --- RenderPage: strings::Tick() first, then every drawn literal ----------------------------
    text = apply_one(
        text,
        "\tvoid __stdcall RenderPage()\n\t{\n\t\t// Pick up any external edits each time the page draws while it is not dirty.\n",
        "\tvoid __stdcall RenderPage()\n\t{\n\t\tstrings::Tick();\n\n"
        "\t\t// Pick up any external edits each time the page draws while it is not dirty.\n",
        "UI.cpp:Tick",
    )
    text = apply_one(
        text,
        "\t\t\tImGuiMCP::TextWrapped(\"skyhud.txt was not found. This menu edits SkyHUD's settings, so \"\n"
        "\t\t\t\t\t\t\t\t  \"SkyHUD (or a preset that provides skyhud.txt) must be installed.\");",
        "\t\t\tImGuiMCP::TextWrapped(\"%s\", strings::TR(\"SHM_NoConfig\", \"skyhud.txt was not found. This menu edits SkyHUD's settings, so SkyHUD (or a preset that provides skyhud.txt) must be installed.\"));",
        "UI.cpp:NoConfig",
    )
    text = apply_one(
        text,
        "\t\tImGuiMCP::TextWrapped(\"Editing %s\", state::Config().path().c_str());",
        "\t\tImGuiMCP::TextWrapped(strings::TR(\"SHM_EditingPath\", \"Editing %s\"), state::Config().path().c_str());",
        "UI.cpp:EditingPath",
    )
    text = apply_one(
        text,
        "\t\tImGuiMCP::TextDisabled(\"Each tab is one HUD element. Save writes skyhud.txt; SkyHUD reads it when the \"\n"
        "\t\t\t\t\t\t\t   \"game starts, so a saved change shows after a restart.\");",
        "\t\tImGuiMCP::TextDisabled(\"%s\", strings::TR(\"SHM_TabHint\", \"Each tab is one HUD element. Save writes skyhud.txt; SkyHUD reads it when the game starts, so a saved change shows after a restart.\"));",
        "UI.cpp:TabHint",
    )
    text = apply_one(
        text,
        "\t\tif (ImGuiMCP::Toggle(\"Show on-screen position markers\", &settings::preview::show)) {",
        "\t\tif (ImGuiMCP::Toggle(strings::TR(\"SHM_ShowMarkers\", \"Show on-screen position markers\"), &settings::preview::show)) {",
        "UI.cpp:ShowMarkers",
    )
    text = apply_one(
        text,
        "\t\tImGuiMCP::TextDisabled(\"ghosts of every positioned element - visible with the menu closed too\");",
        "\t\tImGuiMCP::TextDisabled(\"%s\", strings::TR(\"SHM_MarkersHint\", \"ghosts of every positioned element - visible with the menu closed too\"));",
        "UI.cpp:MarkersHint",
    )
    text = apply_one(
        text,
        "\t\t\t\tif (ImGuiMCP::BeginTabItem(els[i].name)) {\n"
        "\t\t\t\t\tRenderElement(els[i], i);\n"
        "\t\t\t\t\tImGuiMCP::EndTabItem();\n"
        "\t\t\t\t}",
        "\t\t\t\tconst std::string tabLabel = strings::TR(DeriveKey(\"SHM_El_\", els[i].name).c_str(), els[i].name);\n"
        "\t\t\t\tif (ImGuiMCP::BeginTabItem(tabLabel.c_str())) {\n"
        "\t\t\t\t\tRenderElement(els[i], i);\n"
        "\t\t\t\t\tImGuiMCP::EndTabItem();\n"
        "\t\t\t\t}",
        "UI.cpp:TabItem",
    )
    text = apply_one(
        text,
        "\t\tImGuiMCP::TextDisabled(\"SkyHUD reads skyhud.txt only when the game starts, so a saved change shows after a\");\n"
        "\t\tImGuiMCP::TextDisabled(\"restart - the markers show now where each element will land.\");",
        "\t\tImGuiMCP::TextDisabled(\"%s\", strings::TR(\"SHM_FooterA\", \"SkyHUD reads skyhud.txt only when the game starts, so a saved change shows after a\"));\n"
        "\t\tImGuiMCP::TextDisabled(\"%s\", strings::TR(\"SHM_FooterB\", \"restart - the markers show now where each element will land.\"));",
        "UI.cpp:Footer",
    )
    text = apply_one(
        text,
        "\t\tif (ImGuiMCP::Button(\"Save (shows after a restart)\")) {\n"
        "\t\t\tg_status = state::WriteAndApply() ? \"Saved to skyhud.txt. Restart the game to see it in the HUD.\"\n"
        "\t\t\t\t\t\t\t\t\t\t\t  : \"Could not write skyhud.txt. See the log.\";\n"
        "\t\t}\n"
        "\t\tif (ImGuiMCP::Button(\"Reload from file\")) {\n"
        "\t\t\tstate::LoadFromDisk();\n"
        "\t\t\tg_status = \"Reloaded skyhud.txt from disk.\";\n"
        "\t\t}",
        "\t\tif (ImGuiMCP::Button(strings::TR(\"SHM_SaveBtn\", \"Save (shows after a restart)\"))) {\n"
        "\t\t\tg_status = state::WriteAndApply() ? strings::TR(\"SHM_StatusSaved\", \"Saved to skyhud.txt. Restart the game to see it in the HUD.\")\n"
        "\t\t\t\t\t\t\t\t\t\t\t  : strings::TR(\"SHM_StatusSaveFail\", \"Could not write skyhud.txt. See the log.\");\n"
        "\t\t}\n"
        "\t\tif (ImGuiMCP::Button(strings::TR(\"SHM_ReloadBtn\", \"Reload from file\"))) {\n"
        "\t\t\tstate::LoadFromDisk();\n"
        "\t\t\tg_status = strings::TR(\"SHM_StatusReloaded\", \"Reloaded skyhud.txt from disk.\");\n"
        "\t\t}",
        "UI.cpp:SaveReload",
    )

    write(path, text)


def main():
    patch_skse_menu_framework_h()
    patch_message_listeners_cpp()
    patch_devbench_tool_cpp()
    patch_ui_cpp()
    print("lang-patch.py: all anchors matched and patched.")


if __name__ == "__main__":
    main()
