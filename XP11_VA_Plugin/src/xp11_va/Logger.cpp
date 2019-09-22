#include "pch.h"
#include "Logger.h"


const Logger& Logger::get() {
#ifdef _DEBUG
	static Logger instance{ Logger::Level::Trace };
#else
	static Logger instance{ Logger::Level::Info };
#endif

	return instance;
}