// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "xp11_va/Link.h"
#include "xp11_va/Logger.h"

std::unique_ptr<xp11_va::Link> link{ nullptr };

const Logger& logger = Logger::get();

std::string what(const std::exception_ptr& e = std::current_exception()) {
	if (!e) { throw std::bad_exception(); }

	try { std::rethrow_exception(e); }
	catch (const std::exception& e) { return e.what(); }
	catch (const std::string& s) { return s; }
	catch (const char* c) { return c; }
	catch (...) { return "unknown exception type"; }
}

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc) {
	logger.Trace("XPluginStart enter");
	strncpy_s(outName, 255, "XP11/VoiceAttack Connector", 255);
	strncpy_s(outSig, 255, "ndjsoft.xp11va.connector", 255);
	strncpy_s(outDesc, 255, "A connector plugin to link X-Plane 11 with VoiceAttack", 255);

	link = std::make_unique<xp11_va::Link>();

	logger.Info("plugin started");

	logger.Trace("XPluginStart leave");
	return 1;
}

PLUGIN_API int XPluginEnable(void) {
	logger.Trace("XPluginEnable enter");
	if (!link) {
		logger.Error("link pointer is not set");
		logger.Trace("XPluginEnable leave");
		return 0;
	}

	try {
		link->Start();
	} catch (...) {
		logger.Error("Unhandled exception: " + what());
	}

	logger.Info("plugin enabled");
	logger.Trace("XPluginEnable leave");
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID src, int msg, void *inParam) {
	// TODO:
	// handle messages from X-plane. for example, datarefs are basically
	// meaningless until a plane is loaded. so don't return anything until then?
}

PLUGIN_API void XPluginDisable() {
	logger.Trace("XPluginDisable enter");
	if (!link) { return; }

	try {
		link->Stop();
	} catch (...) {
		logger.Error("Unhandled exception: " + what());
	}
	
	logger.Info("plugin disabled");
	logger.Trace("XPluginDisable leave");
}

PLUGIN_API void XPluginStop() {
	logger.Trace("XPluginStop enter");

	link.reset(nullptr);
	
	logger.Info("plugin stopped");
	logger.Trace("XPluginStop leave");
}

#ifdef _WIN32

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
	return TRUE;
}

#endif