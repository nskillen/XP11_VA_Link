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
		DataCache refCache;
		std::atomic_bool shouldStop;
		std::thread connectionThread;
		std::vector<std::shared_ptr<Pipe>> pipes;
		std::vector<std::unique_ptr<std::thread>> pipeThreads;
		
		XPLMFlightLoopID flightLoopID;
		std::mutex callbackMutex;
		CallbackList flightLoopCallbacks;

		XPLMFlightLoopID createFlightLoop();
		float onFlightLoop(float, float, int);
		void runCallbacks();
		void runOnSimThread(const Callback&);

		std::unique_ptr<std::thread> connectAndRunPipe(std::shared_ptr<Pipe>&&);
		static void with_lock(std::mutex&, const std::function<void()>&&);
	};
}
