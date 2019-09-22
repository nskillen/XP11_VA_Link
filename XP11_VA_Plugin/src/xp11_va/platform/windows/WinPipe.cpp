#include "pch.h"
#include "WinPipe.h"
#include "xp11_va/Logger.h"

#include <codecvt>
#include <cstdlib>

using namespace std::chrono_literals;

const wchar_t* PIPE_NAME = L"\\\\.\\pipe\\{2145AB63-BF83-40A4-8A9D-A358D45AF1C1}";
const wchar_t* PIPE_GUID = L"{2145AB63-BF83-40A4-8A9D-A358D45AF1C1}";
const wchar_t* PIPE_REG_KEY = L"SOFTWARE\\{2145AB63-BF83-40A4-8A9D-A358D45AF1C1}\\XP11_VA_Link";
const wchar_t* PIPE_MUTEX_NAME = L"{2145AB63-BF83-40A4-8A9D-A358D45AF1C1}/XP11_VA_Link";

std::string fromWS(const std::wstring& str) {
	const size_t bytes = str.length() * sizeof(std::wstring::value_type);
	const auto buf = new char[bytes];
	size_t bytesConverted;
	wcstombs_s(&bytesConverted, buf, bytes, str.c_str(), bytes);
	std::string s{ buf };
	delete[] buf;
	return s;
}

std::string lastErrorToString() {
	static const auto wbuf = new wchar_t[1024];
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr,
		GetLastError(),
		NULL,
		wbuf,
		1024,
		nullptr);
	return fromWS(wbuf);
}

void logLastError() {
	static const auto& logger = Logger::get();
	logger.Error(lastErrorToString());
}

namespace xp11_va::platform::windows {
	const auto& logger = Logger::get();
	
	/* PUBLIC API */

	WinPipe::WinPipe()
	: pipe(INVALID_HANDLE_VALUE), connectOverlap({}), connectEvent(INVALID_HANDLE_VALUE), readOverlap({}), writeOverlap({}) {
		logger.Trace("WinPipe::constructor enter");

		pipe = CreateNamedPipe(
			PIPE_NAME,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFFER_SIZE,
			BUFFER_SIZE,
			0, //default 50ms
			nullptr);

		if (pipe == INVALID_HANDLE_VALUE) {
			logger.Error("Pipe handle invalid");
			logLastError();
			logger.Trace("WinPipe::constructor leave");
			throw std::runtime_error("Failed to create pipe handle");
		}

		readOverlap.overlap.hEvent = INVALID_HANDLE_VALUE;
		readOverlap.overlap.hEvent = CreateEvent(nullptr, true, false, nullptr);
		if (readOverlap.overlap.hEvent == INVALID_HANDLE_VALUE) {
			logger.Error("Error creating read event: " + lastErrorToString());
			CloseHandle(pipe);
			logger.Trace("WinPipe::constructor leave");
			throw std::runtime_error("Failed to create read event");
		}

		writeOverlap.overlap.hEvent = INVALID_HANDLE_VALUE;
		writeOverlap.overlap.hEvent = CreateEvent(nullptr, true, false, nullptr);
		if (writeOverlap.overlap.hEvent == INVALID_HANDLE_VALUE) {
			logger.Error("Error creating write event: " + lastErrorToString());
			CloseHandle(readOverlap.overlap.hEvent);
			CloseHandle(pipe);
			logger.Trace("WinPipe::constructor leave");
			throw std::runtime_error("Failed to create write event");
		}

		logger.Info("Pipe created");
		logger.Trace("WinPipe::constructor leave");
	}	

	WinPipe::~WinPipe() {
		logger.Trace("WinPipe::destructor enter");

		if (pipe != INVALID_HANDLE_VALUE) {
			logger.Info("Disconnecting pipe and closing handle");
			DisconnectNamedPipe(pipe);
			CloseHandle(pipe);
		} else {
			logger.Info("Pipe was not open");
		}
		
		if (connectEvent != INVALID_HANDLE_VALUE) { CloseHandle(connectEvent); }
		if (readOverlap.overlap.hEvent != INVALID_HANDLE_VALUE) { CloseHandle(readOverlap.overlap.hEvent); }
		if (writeOverlap.overlap.hEvent != INVALID_HANDLE_VALUE) { CloseHandle(writeOverlap.overlap.hEvent); }

		if (connectThread.joinable()) { connectThread.join(); }

		logger.Trace("WinPipe::destructor leave");
	}

	std::future<bool> WinPipe::Connect() {
		logger.Trace("WinPipe::Connect enter");

		connectPromise = std::promise<bool>{};
		auto connectFuture = connectPromise.get_future();

		connectThread = std::thread{ [this]() {
			logger.Trace("Pipe connection thread start");
			try {
				connectEvent = CreateEvent(nullptr, true, false, nullptr);
				if (connectEvent == INVALID_HANDLE_VALUE) {
					CloseHandle(pipe);
					throw std::runtime_error("Failed to create connect event: " + lastErrorToString());
				}

				connectOverlap.hEvent = connectEvent;

				const auto res = ConnectNamedPipe(pipe, &connectOverlap);
				const auto err = GetLastError();

				if (!res && err == ERROR_IO_PENDING) {
					logger.Info("IO Pending - waiting for connection");
					switch (WaitForSingleObject(connectEvent, INFINITE)) {
					case WAIT_OBJECT_0:
						DWORD n;
						if (GetOverlappedResult(pipe, &connectOverlap, &n, true)) {
							logger.Info("Pipe connected");
							CloseHandle(connectEvent);
							connectEvent = INVALID_HANDLE_VALUE;
							connectPromise.set_value(true);
							return;
						}

						logger.Error("Failed to connect pipe");
						logLastError();
						break;
					case WAIT_TIMEOUT:
						logger.Error("ConnectNamedPipe wait timeout - this should not happen!");
						break;
					case WAIT_FAILED:
						logger.Error("Failed while waiting for pipe to connect");
						logLastError();
						break;
					default:
						logger.Error("Unexpected error waiting for pipe to connect");
						logLastError();
						break;
					}
				}
				else if (!res && err == ERROR_PIPE_CONNECTED) {
					logger.Info("Pipe client connected between CreateNamedPipe and ConnectNamedPipe");
					CloseHandle(connectEvent);
					connectEvent = INVALID_HANDLE_VALUE;
					connectPromise.set_value(true);
					return;
				}
				else {
					logger.Error("Unexpected Error");
					logLastError();
				}
				CloseHandle(connectEvent);
				connectEvent = INVALID_HANDLE_VALUE;
				CloseHandle(pipe);
				pipe = INVALID_HANDLE_VALUE;
				connectPromise.set_value(false);
			} catch (...) {
				connectPromise.set_exception(std::current_exception());
			}
		} };

		return connectFuture;
	}

	std::future<std::string> WinPipe::ReadPipe() {
		logger.Trace("WinPipe::ReadPipe enter");

		return std::async([this]() -> std::string {
			const auto lpo = reinterpret_cast<LPOVERLAPPED>(&readOverlap);
			const auto result = ReadFile(pipe, readOverlap.buffer, BUFFER_SIZE, nullptr, lpo);

			if (result || GetLastError() == ERROR_IO_PENDING) {
				if (GetOverlappedResult(pipe, lpo, &readOverlap.count, true)) {
					auto& buf = readOverlap.buffer;
					auto& c = readOverlap.count;

					if (c < BUFFER_SIZE - 1) {
						buf[c] = '\0';
					} else {
						buf[BUFFER_SIZE - 1] = '\0';
					}

					return buf;
				} else {
					throw std::runtime_error("Error in GetOverlappedResult: " + lastErrorToString());
				}
			} else {
				throw std::runtime_error("Unexpected error in read operation: " + lastErrorToString());
			}
			});
	}

	std::future<bool> WinPipe::WritePipe(std::string msg) {
		logger.Trace("WinPipe::WritePipe enter");

		return std::async([this, &msg]() -> bool {
			const auto lpo = reinterpret_cast<LPOVERLAPPED>(&writeOverlap);
			const auto bytesToWrite = msg.length() * sizeof(std::string::value_type);
			
			const auto result = WriteFile(pipe, msg.c_str(), bytesToWrite, nullptr, lpo);

			if (result || GetLastError() == ERROR_IO_PENDING) {
				if (GetOverlappedResult(pipe, lpo, &writeOverlap.count, true)) {
					return writeOverlap.count == bytesToWrite;
				} else {
					throw std::runtime_error("Error in GetOverlappedResult: " + lastErrorToString());
				}
			} else {
				throw std::runtime_error("Error writing to pipe: " + lastErrorToString());
			}
			});
	}
}