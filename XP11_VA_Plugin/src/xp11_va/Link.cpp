#include "pch.h"
#include "Link.h"
#include "Pipe.h"
#include "Logger.h"

#include <future>

#include <XPLM/XPLMProcessing.h>

using namespace std::chrono_literals;

namespace xp11_va {
	std::string what(const std::exception_ptr& e = std::current_exception()) {
		if (!e) { throw std::bad_exception(); }

		try { std::rethrow_exception(e); }
		catch (const std::exception& e) { return e.what(); }
		catch (const std::string& s) { return s; }
		catch (const char* c) { return c; }
		catch (...) { return "unknown exception type"; }
	}

	const Logger& logger = Logger::get();
	
	Link::Link() : started(false) {
		logger.Trace("Link constructor enter");
		shouldStop = false;
		flightLoopID = createFlightLoop();
		XPLMScheduleFlightLoop(flightLoopID, -1, true);
		logger.Trace("Link constructor leave");
	}

	Link::~Link() {
		logger.Trace("Link destructor enter");
		Stop();
		if (flightLoopID) {
			XPLMDestroyFlightLoop(flightLoopID);
		}
		if (connectionThread.joinable()) {
			logger.Trace("joining pipeThread...");
			connectionThread.join();
			logger.Trace("pipeThread joined (aka it's done)");
		}
		logger.Trace("Link destructor leave");
	}

	void Link::Start() {
		logger.Trace("Link::Start enter");

		if (started) {
			throw std::runtime_error("Link already started");
		}
		
		connectionThread = std::thread([this]() {
			logger.Trace("Link background thread start");
			try {
				while (!shouldStop) {
					try {
						pipeThreads.push_back(connectAndRunPipe(Pipe::get().get()));
					} catch (std::runtime_error& err) {
						logger.Error(err.what());
					}
				}
				logger.Info("shouldStop has gone false");
				logger.Info("Waiting for pipe threads to terminate...");
				while (!pipeThreads.empty()) {
					auto& pipe_thread = pipeThreads.back();
					pipe_thread->join();
					pipeThreads.pop_back();
				}
				logger.Info("All pipe threads have terminated");
			} catch (const std::exception& ex) {
				logger.Critical(ex.what());
			}
			logger.Trace("Link background thread terminating");
		});
		started = true;
		logger.Trace("Link::Start leave");
	}

	void Link::Stop() {
		logger.Trace("Link::Stop enter");

		if (!started) {
			logger.Trace("Link::Stop leave");
			return;
		}
		
		try {
			shouldStop = true;
			for (auto& pipe : pipes) {
				pipe.reset();
			}
		} catch (const std::exception& e) {
			logger.Critical(e.what());
		}
		logger.Trace("Link::Stop leave");
	}

	void Link::runOnSimThread(const Callback& callback) {
		logger.Trace("Link::runOnSimThread enter");

		std::lock_guard<std::mutex> lock(callbackMutex);

		if (shouldStop) { return; } // do nothing, plugin is terminating
		
		flightLoopCallbacks.push_back(callback);
		
		logger.Trace("Link::runOnSimThread leave");
	}

	XPLMFlightLoopID Link::createFlightLoop() {
		logger.Trace("Link::createFlightLoop enter");
		XPLMCreateFlightLoop_t loop;
		loop.structSize = sizeof(XPLMCreateFlightLoop_t);
		loop.phase = xplm_FlightLoop_Phase_BeforeFlightModel;
		loop.refcon = this;
		loop.callbackFunc = [](float f1, float f2, int c, void* ref) -> float {
			if (!ref) { return 0; }
			auto* us = reinterpret_cast<Link*>(ref);
			return us->onFlightLoop(f1, f2, c);
		};

		XPLMFlightLoopID id = XPLMCreateFlightLoop(&loop);
		if (!id) {
			throw std::runtime_error("Couldn't create flight loop");
		}

		logger.Trace("Link::createFlightLoop leave");
		return id;
	}

	float Link::onFlightLoop(float /*elapsedSinceLastCall*/, float /*elapsedSinceLastLoop*/, int count) {
		runCallbacks();
		return 0.25;
	}

	void Link::runCallbacks() {
		std::lock_guard<std::mutex> lock(callbackMutex);
		if (!flightLoopCallbacks.empty()) {
			logger.Info("Running " + std::to_string(flightLoopCallbacks.size()) + " callbacks");
			for (auto& cb : flightLoopCallbacks) {
				cb();
			}
			flightLoopCallbacks.clear();
		}
	}

	std::unique_ptr<std::thread> Link::connectAndRunPipe(std::shared_ptr<Pipe>&& _pipe) {
		pipes.push_back(std::move(_pipe));
		auto pipe = pipes.back();
		auto pi = pipes.size() - 1;

		pipe->Connect().get();

		return std::make_unique<std::thread>(std::thread{ [this, pi]() {
			while (!shouldStop) {
				auto pipe = pipes.at(pi);
				auto data_ref_name = pipe->ReadPipe().get();

				std::promise<std::string> reply_promise;
				auto reply_future = reply_promise.get_future();

				runOnSimThread([&reply_promise, &data_ref_name, this]() {
					logger.Trace("Sim-thread callback running");
					try {
						auto data = refCache.getData(data_ref_name);
						reply_promise.set_value(data.ToString());
						logger.Info("Got value: " + data_ref_name + "=" + data.ToString());
					}
					catch (...) {
						reply_promise.set_exception(std::current_exception());
					}
					});

				std::string reply{ };
				try {
					logger.Info("Awaiting a reply from the future...");
					reply = reply_future.get();
				}
				catch (std::runtime_error& e) {
					logger.Error(e.what());
					reply = "invalid data ref " + data_ref_name;
				}

				logger.Trace("Will write reply to pipe...");
				pipe->WritePipe(reply).get();
			}
		} });
	}

	void Link::with_lock(std::mutex& mutex, const std::function<void()>&& func) {
		std::lock_guard<std::mutex> lock(mutex);
		logger.Trace("mutex locked");

		func();
		
		logger.Trace("mutex unlocking");
	}
}