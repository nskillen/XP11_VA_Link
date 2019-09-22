#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <sstream>
#include <XPLM/XPLMUtilities.h>

class Logger {
public:
	enum class Level : uint8_t {
		Trace = 0,
		Info,
		Warn,
		Error,
		Critical,
		Fatal
	};
	
	static const Logger& get();

	void SetMinLevel(Level l) { this->minLevel = l; }

	void Trace(const std::string& msg)    const noexcept { log(Level::Trace,    msg); }
	void Info(const std::string& msg)     const noexcept { log(Level::Info,     msg); }
	void Warn(const std::string& msg)     const noexcept { log(Level::Warn,     msg); }
	void Error(const std::string& msg)    const noexcept { log(Level::Error,    msg); }
	void Critical(const std::string& msg) const noexcept { log(Level::Critical, msg); }
	void Fatal(const std::string& msg)    const noexcept { log(Level::Fatal,    msg); }

private:
	Level minLevel;

private:
	Logger(const Level l) : minLevel(l) {}

	void log(const Level l, const std::string& message) const noexcept {
		try {
			if (l < minLevel) { return; }

			std::stringstream ss;
			ss << "[XP11_VA_Link :: ";
			ss << std::this_thread::get_id();
			ss << " :: ";

			switch (l) {
			case Level::Trace:
				ss << "TRACE   ]";
				break;
			case Level::Info:
				ss << "INFO    ]";
				break;
			case Level::Warn:
				ss << "WARN    ]";
				break;
			case Level::Error:
				ss << "ERROR   ]";
				break;
			case Level::Critical:
				ss << "CRITICAL]";
				break;
			case Level::Fatal:
				ss << "FATAL   ]";
				break;
			}

			ss << ": " << message << std::endl;

			XPLMDebugString(ss.str().c_str());
		} catch (...) {}
	}

};

#endif