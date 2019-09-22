#include "pch.h"
#include "Link.h"
#include "Pipe.h"
#include "Logger.h"

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
		shouldStop = false;
		flightLoopID = createFlightLoop();
		XPLMScheduleFlightLoop(flightLoopID, -1, true);
	}

	Link::~Link() {
		Stop();
		if (flightLoopID) {
			logger.Info("Nuking flight loop");
			XPLMDestroyFlightLoop(flightLoopID);
		}
	}

	void Link::Start() {
		if (started) {
			throw std::runtime_error("Link already started");
		}

		connectionThread = std::make_unique<std::thread>(std::thread([this]() {
			while (!shouldStop) {
				connectingPipe = Pipe::get();
				try {
					connectingPipe->Connect();
					if (!connectingPipe->IsConnected()) {
						continue;
					}
					
					auto pipe_thread = std::make_unique<std::thread>([this, pipe = connectingPipe]() {
						try {
							while (!shouldStop) {
								auto maybe_request = pipe->ReadPipe();
								if (!maybe_request.has_value()) { break; }
								
								const auto request = maybe_request.value();
								const auto response = processRequest(request);

								if (!pipe->WritePipe(response + "\n")) { break; }
								logger.Info("Responded with: " + response);
							}
						}
						catch (...) {
							logger.Error("Error on pipe thread: " + what());
						}

						logger.Trace("Pipe thread terminating");
						});
					{
						std::lock_guard<std::mutex> lock(pipesMutex);
						pipes.emplace_back(std::make_pair(std::move(pipe_thread), connectingPipe));
					}
				}
				catch (...) {
					logger.Error("Error on connect thread: " + what());
				}
			}
		}));
		started = true;
	}

	void Link::Stop() {
		if (!started) { return; }
		
		try {
			shouldStop = true;
			
			if (!pipes.empty()) {
				logger.Info("Killing " + std::to_string(pipes.size()) + " pipes");
				for (auto& pipe_pair : pipes) {
					auto& thread = pipe_pair.first;
					auto& pipe = pipe_pair.second;
					pipe->Abort(thread->native_handle());
					std::stringstream ss;
					ss << "Pipe thread " << thread->get_id() << " killed, joining";
					logger.Info(ss.str());
					if (thread->joinable()) {
						thread->join();
					}
				}
				logger.Info("Pipe threads killed, clearing list");
				pipes.clear();
			}

			if (connectionThread) {
				logger.Info("Killing connectingPipe");
				connectingPipe->Abort(connectionThread->native_handle());
				logger.Info("Connecting thread killed, joining");
				if (connectionThread->joinable()) {
					connectionThread->join();
				}
				logger.Info("Clearing connection thread");
				connectionThread.reset();
			}
			
			logger.Info("All pipes killed");
		} catch (...) {
			logger.Error("Error while stopping pipes: " + what());
		}
	}

	XPLMFlightLoopID Link::createFlightLoop() {
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

		return id;
	}

	float Link::onFlightLoop(float /*elapsedSinceLastCall*/, float /*elapsedSinceLastLoop*/, int count) {
		std::lock_guard<std::mutex> lock(callbackMutex);
		if (!flightLoopCallbacks.empty()) {
			logger.Info("Running " + std::to_string(flightLoopCallbacks.size()) + " callbacks");
			for (auto& cb : flightLoopCallbacks) {
				cb();
			}
			flightLoopCallbacks.clear();
		}
		return 0.25;
	}

	void Link::runOnSimThread(const Callback& callback) {
		std::lock_guard<std::mutex> lock(callbackMutex);

		if (shouldStop) { return; } // do nothing, plugin is terminating

		flightLoopCallbacks.push_back(callback);
	}

	std::string Link::processRequest(const std::string& request) {
		logger.Info("Received dataRef request: " + request);

		if (request.substr(0, 3) == "get") {
			const std::string requested_data_ref = request.substr(4);
			std::promise<std::string> value_promise;
			auto value_future = value_promise.get_future();

			runOnSimThread([this, &value_promise, &requested_data_ref]() {
				try {
					auto data = refCache.getData(requested_data_ref);
					value_promise.set_value(data.ToString());
				}
				catch (...) {
					value_promise.set_exception(std::current_exception());
				}
				});

			try {
				return value_future.get();
			}
			catch (...) {
				return "{invalid_dataref}";
			}
		}
		else if (request.substr(0, 3) == "set") {
			try {
				const std::string data_ref_part = request.substr(request.find(';') + 1);
				const std::string updated_data_ref_name = data_ref_part.substr(0, data_ref_part.find(';'));
				const std::string updated_data_ref_value = data_ref_part.substr(data_ref_part.find(';') + 1);
				const auto ed = EnvData::fromString(updated_data_ref_value);
				refCache.setData(updated_data_ref_name, ed);
				return "{ok}";
			} catch (...) {
				logger.Error("Error setting dataref: " + what());
				return "{set_failed}";
			}
		}
		else {
			return "{invalid_command_" + request.substr(0, request.find(';')) + "}";
		}
	}

}