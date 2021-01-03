#pragma once

#include <XPLM/XPLMProcessing.h>

#include "DataCache.h"
#include "Pipe.h"

namespace xp11_va {
	class Link {
	public:
		typedef std::function<bool()> Callback;
		typedef std::list<Callback> CallbackList;
		
		Link();
		~Link();

		void Start();
		void Stop();

	private:
		bool started;
		std::atomic_bool shouldStop;
		std::unique_ptr<std::thread> connectionThread;
		std::shared_ptr<Pipe> connectingPipe;
		std::vector<std::pair<
			std::unique_ptr<std::thread>,
			std::shared_ptr<Pipe>>> pipes;
		std::mutex pipesMutex;
		
		XPLMFlightLoopID flightLoopID;
		std::recursive_mutex callbackMutex;
		CallbackList flightLoopCallbacks;
		std::atomic<float> totalTimeElapsed;

		XPLMFlightLoopID createFlightLoop();
		float onFlightLoop(float, float, int);
		void runOnSimThread(const Callback&);

		DataCache<std::string, XPLMDataRef> refCache = { [](const auto& key) -> XPLMDataRef { return XPLMFindDataRef(key.c_str()); } };
		DataCache<std::string, XPLMCommandRef> cmdCache = { [](const auto& key) -> XPLMCommandRef { return XPLMFindCommand(key.c_str()); } };
		
		std::string processRequest(const std::string&);
		
		std::string handleDatarefRequest(const std::vector<std::string>&);
		std::string getDataref(const std::vector<std::string>&);
		std::string setDataref(const std::vector<std::string>&);

		std::string handleCommandRequest(const std::vector<std::string>&);
	};
}
