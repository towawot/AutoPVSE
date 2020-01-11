#include "skse64/PluginAPI.h"
#include "skse64_common/skse_version.h"
#include "skse64/PapyrusVM.h"
#include <shlobj.h>
#include "papyrus.h"
#include "hooks.h"

IDebugLog				gLog;
PluginHandle			g_pluginHandle = kPluginHandle_Invalid;
SKSEMessagingInterface	*	g_messaging = nullptr;

void MessageHandler(SKSEMessagingInterface::Message * msg)
{
	switch (msg->type)
	{
	case SKSEMessagingInterface::kMessage_DataLoaded:
		papyrus::RegisterFuncs((*g_skyrimVM)->GetClassRegistry());
		hooks::init();
		break;
	}
}

extern "C"
{

	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{

		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\AutoPV.log");

		info->infoVersion = PluginInfo::kInfoVersion;

		info->name = "AutoPV";
		info->version = CURRENT_RELEASE_RUNTIME;

		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");
			return false;
		}

		if (skse->runtimeVersion != CURRENT_RELEASE_RUNTIME)
		{
			_MESSAGE("This plugin is not compatible with this versin of game.");
			return false;
		}

		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		_MESSAGE("Load");
		g_messaging = (SKSEMessagingInterface*)skse->QueryInterface(kInterface_Messaging);
		if (!g_messaging->RegisterListener(g_pluginHandle, "SKSE", MessageHandler))
			return false;

		return true;
	}

}