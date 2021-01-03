#include "pch.h"
#include "Link.h"
#include "Pipe.h"
#include "Logger.h"

#include <XPLM/XPLMProcessing.h>

using namespace std::chrono_literals;

namespace xp11_va {
	std::vector<std::vector<std::string>> tokenize(const std::string&);
	std::string what();
	
	Logger& logger = Logger::get();

	/* PUBLIC API */
	
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

	/* PRIVATE API */

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
		std::lock_guard<std::recursive_mutex> lock(callbackMutex);
		if (!flightLoopCallbacks.empty()) {
			logger.Info("Running " + std::to_string(flightLoopCallbacks.size()) + " callbacks");

			CallbackList remaining{};

			while (flightLoopCallbacks.size() > 0) {
				auto cb = flightLoopCallbacks.front();
				flightLoopCallbacks.pop_front();
				// callback returns true when complete, false if needs to run again
				if (!cb()) { remaining.push_back(cb); }
			}

			flightLoopCallbacks = remaining;
		}
		return 0.25;
	}

	void Link::runOnSimThread(const Callback& callback) {
		std::lock_guard<std::recursive_mutex> lock(callbackMutex);

		if (shouldStop) { return; } // do nothing, plugin is terminating

		flightLoopCallbacks.push_back(callback);
	}

	std::string Link::processRequest(const std::string& request) {
		/* Request format:
		 * data may be sent to the pipe in the following fashion:
		 *     request;request;request;...;request
		 *
		 * where each request has the following format:
		 *     request_type:dataref_name[:dataref_type:dataref_value]
		 *     request_type:command_name:command_action[:command_duration]
		 *
		 * For requests dealing with datarefs, valid values for request_type are 'get' and 'set'
		 *   - dataref_name must be a valid dataref, and must refer to a writable dataref for 'set' commands
		 *   - dataref_type must not bitwise-and with the X-Plane supplied type to 0, and is not required for 'get' commands
		 *   - dataref_value is converted to the appropriate datatype and stored in the dataref, and is not required for 'get' commands
		 *
		 * For requests dealing with commands, valid values for request_type are 'cmd'
		 *   - command_name must correspond to a valid action
		 *   - command_action must be one of 'begin', 'end', 'once', or 'hold'
		 *   - command_duration must be an integer number of milliseconds to hold the command active for, and is ignored if command_action is not 'hold'
		*/
		logger.Info("Received request: " + request);

		std::vector<std::vector<std::string>> commands = tokenize(request);
		std::vector<std::string> results{};

		logger.Info("Processed into " + std::to_string(commands.size()) + " requests");

		for (auto& cmd : commands) {
			if (cmd[0] == "get" || cmd[0] == "set") {
				// this is a dataref request
				results.push_back(handleDatarefRequest(cmd));
			}
			else if (cmd[0] == "cmd") {
				// this is an action command
				results.push_back(handleCommandRequest(cmd));
			}
			else {
				logger.Error("Invalid command: " + cmd[0]);
				results.push_back("{invalid_command}");
			}
		}

		std::stringstream ss;
		for (auto i = 0; i < results.size(); i++) {
			ss << results[i];
			if (i < results.size() - 1) {
				ss << ';';
			}
		}

		return ss.str();
	}

	std::string Link::handleDatarefRequest(const std::vector<std::string>& request) {
		if (request.size() < 1) {
			return "{malformed_request}";
		}

		try {
			const auto& action = request[0];
			if (action == "get") {
				if (request.size() < 2) {
					return "{malformed_request}";
				}
				return getDataref(request);
			}
			else if (action == "set") {
				if (request.size() != 4) {
					return "{malformed_request}";
				}
				return setDataref(request);
			}
			else {
				throw std::runtime_error("Invalid dataref request action: " + action);
			}
		}
		catch (...) {
			return "{error}";
		}
	}

	std::string Link::getDataref(const std::vector<std::string>& request) {
		const std::string& dataref_name = request[1];

		std::promise<std::string> get_promise;
		auto get_future = get_promise.get_future();

		runOnSimThread([this, &get_promise, &dataref_name]() -> bool {
			const XPLMDataRef dataref = refCache.Get(dataref_name).value();
			if (!dataref) {
				get_promise.set_value("{invalid_dataref}");
				return true;
			}

			try {
				auto data = EnvData::fromDataref(dataref_name, dataref);
				get_promise.set_value(data.ToString());
			}
			catch (...) {
				get_promise.set_exception(std::current_exception());
			}

			return true;
			});

		try {
			return get_future.get();
		}
		catch (...) {
			logger.Error("Error getting dataref: " + what());
			return "{get_failed}";
		}
	}

	std::string Link::setDataref(const std::vector<std::string>& request) {
		const auto& dataref_name = request[1];
		const auto& dataref_type = request[2];
		const auto& dataref_value = request[3];

		std::promise<std::string> set_promise;
		auto set_future = set_promise.get_future();

		runOnSimThread([this, &set_promise, dataref_name, dataref_type, dataref_value]() -> bool {
			auto ed = EnvData::fromString(dataref_name, dataref_type, dataref_value);

			XPLMDataRef dataref = refCache.Get(ed.name).value();
			if (!dataref) {
				set_promise.set_value("{invalid_dataref}");
				return true;
			}

			auto type = XPLMGetDataRefTypes(dataref);
			if ((type & ed.type) == 0) {
				std::stringstream ss;
				ss << "Dataref type mismatch, user sent " << ed.type << ", X-Plane expects " << type;
				logger.Warn(ss.str());
				set_promise.set_value("{dataref_type_mismatch}");
				return true;
			}

			if (XPLMCanWriteDataRef(dataref)) {
				switch (ed.type) {
				case xplmType_Int:
					XPLMSetDatai(dataref, ed.intVal);
					break;
				case xplmType_Float:
					XPLMSetDataf(dataref, ed.floatVal);
					break;
				case xplmType_Double:
					XPLMSetDatad(dataref, ed.doubleVal);
					break;
				case xplmType_FloatArray:
					XPLMSetDatavf(dataref, ed.floatArray, 0, static_cast<int>(ed.arrayElemCount));
					break;
				case xplmType_IntArray:
					XPLMSetDatavi(dataref, ed.intArray, 0, static_cast<int>(ed.arrayElemCount));
					break;
				case xplmType_Data:
					XPLMSetDatab(dataref, ed.byteArray, 0, static_cast<int>(ed.arrayElemCount));
					break;
				case xplmType_Unknown:
				default:
					logger.Warn("Unknown dataref type " + std::to_string(ed.type));
					set_promise.set_value("{unknown_type}");
					return true;
				}
				set_promise.set_value("{ok}");
				return true;
			}
			else {
				set_promise.set_value("{dataref_not_writable}");
			}

			return true;
			});

		try {
			return set_future.get();
		}
		catch (...) {
			logger.Error("Error setting dataref: " + what());
			return "{set_failed}";
		}
	}

	std::string Link::handleCommandRequest(const std::vector<std::string>& request) {
		if (request.size() < 3) { throw "malformed_action"; }

		const std::string& command_name = request[1];
		const std::string& command_action = request[2];
		const std::optional<std::string> command_duration = request.size() >= 4 ? request[3] : std::optional<std::string>{};

		std::promise<std::string> cmd_promise;
		auto cmd_future = cmd_promise.get_future();

		runOnSimThread([this, &cmd_promise, command_name, command_action, command_duration]() -> bool {
			const auto cmd = cmdCache.Get(command_name).value();
			if (!cmd) {
				logger.Warn("Command " + command_name + " not found");
				cmd_promise.set_value("{invalid_command}");
			}

			if (command_action == "begin") {
				logger.Trace("Command " + command_name + " beginning");
				XPLMCommandBegin(cmd);
			}
			else if (command_action == "end") {
				logger.Trace("Command " + command_name + " ending");
				XPLMCommandEnd(cmd);
			}
			else if (command_action == "once") {
				logger.Trace("Command " + command_name + " firing once");
				XPLMCommandOnce(cmd);
			}
			else if (command_action == "hold") {
				logger.Trace("Command " + command_name + " start and hold");
				if (!command_duration.has_value()) {
					logger.Trace("Command " + command_name + " missing hold duration");
					cmd_promise.set_value("{missing_hold_duration}");
					return true;
				}

				const auto hold_duration = std::strtol(command_duration.value().c_str(), nullptr, 10);

				XPLMCommandBegin(cmd);
				runOnSimThread([then = std::chrono::steady_clock::now(), duration = hold_duration, cmd, command_name]() -> bool {
					const auto now = std::chrono::steady_clock::now();
					if (std::chrono::duration_cast<std::chrono::milliseconds>(now - then).count() < duration) {
						logger.Trace("Command " + command_name + " has longer to run yet");
						return false;
					}

					logger.Trace("Command " + command_name + " hold ending");
					XPLMCommandEnd(cmd);
					return true;
					});
			}
			else {
				logger.Trace("Command action " + command_action + " invalid");
				cmd_promise.set_value("{invalid_command_action}");
			}

			cmd_promise.set_value("{ok}");
			return true;
			});

		try {
			return cmd_future.get();
		}
		catch (...) {
			logger.Error("Error in command: " + what());
			return "{cmd_failed}";
		}
	}

	/* HELPER METHODS */

	std::string what() {
		const auto e = std::current_exception();
		if (!e) { throw std::bad_exception(); }

		try { std::rethrow_exception(e); }
		catch (const std::exception& e) { return e.what(); }
		catch (const std::string& s) { return s; }
		catch (const char* c) { return c; }
		catch (...) { return "unknown exception type"; }
	}

	std::vector<std::vector<std::string>> tokenize(const std::string& request) {
		std::vector<std::vector<std::string>> commands;

		std::string remaining = request.substr(0);

		while (!remaining.empty()) {
			auto next_semi = remaining.find(';');
			std::string command = remaining.substr(0, next_semi);
			if (std::string::npos != next_semi && remaining.size() - 1 > next_semi) {
				remaining = remaining.substr(remaining.find(';') + 1);
			}
			else {
				remaining = "";
			}

			commands.push_back({});
			size_t c_idx = commands.size() - 1;

			size_t t_start = 0,
				   t_end = command.find(':', t_start);
			while (true) {
				std::string token = command.substr(t_start, t_end - t_start);
				commands[c_idx].push_back(token);

				if (t_end == std::string::npos) { break; }
				t_start = t_end + 1;
				t_end = command.find(':', t_start);
			}

			logger.Info("Processed token: " + command);
		}

		return commands;
	}
}