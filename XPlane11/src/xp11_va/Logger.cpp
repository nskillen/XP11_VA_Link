#include "pch.h"
#include "Logger.h"

Logger* Logger::instance = nullptr;

Logger& Logger::get() {
	if (Logger::instance == nullptr) {
		Logger::Level minLevel =
#ifdef _DEBUG
			Logger::Level::Trace;
#else
			Logger::Level::Warn;
#endif

		Logger::instance = new Logger(minLevel);
		Logger::instance->ForceLog(Logger::instance->minLevel, "Logger initialized at this minlevel");
	}

	return *Logger::instance;
}
