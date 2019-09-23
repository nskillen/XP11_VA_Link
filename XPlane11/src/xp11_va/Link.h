#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <XPLM/XPLMProcessing.h>

#include "DataCache.h"
#include "Pipe.h"

namespace xp11_va {
	class Link {
	public:
		typedef std::function<void()> Callback;
		typedef std::vector<Callback> CallbackList;
		
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
		std::mutex callbackMutex;
		CallbackList flightLoopCallbacks;

		XPLMFlightLoopID createFlightLoop();
		float onFlightLoop(float, float, int);
		void runOnSimThread(const Callback&);

		DataCache refCache;
		std::string processRequest(const std::string&);
		std::string setDataref(EnvData&);
	};
}
