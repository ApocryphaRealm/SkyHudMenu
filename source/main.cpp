#include "Settings.h"

#include "utils/Logger.h"

void SKSEMessageListener(SKSE::MessagingInterface::Message* a_msg);

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	// Workaround for static initialization order bug of CommonLibSSE-NG.
	REL::Module::reset();

	const SKSE::PluginDeclaration* plugin = SKSE::PluginDeclaration::GetSingleton();

	if (!logger::init(plugin->GetName()))
	{
		return false;
	}

	logger::info("Loading {} {}...", plugin->GetName(), plugin->GetVersion());

	SKSE::Init(a_skse);
	logger::debug("SKSE core APIs initialized");

	logger::debug("Loading settings from {}.ini", plugin->GetName());
	settings::Init(std::string(plugin->GetName()) + ".ini");

	logger::set_level(settings::debug::logLevel, settings::debug::logLevel);
	logger::debug("Settings loaded; log level set to {}", static_cast<std::uint32_t>(settings::debug::logLevel));
	logger::describe_level(std::string(plugin->GetName()) + ".ini");

	if (!SKSE::GetMessagingInterface()->RegisterListener("SKSE", SKSEMessageListener))
	{
		logger::error("Could not register the SKSE message listener; plugin load aborted");

		return false;
	}

	logger::debug("SKSE message listener registered");
	logger::info("Successfully loaded!");

	return true;
}
