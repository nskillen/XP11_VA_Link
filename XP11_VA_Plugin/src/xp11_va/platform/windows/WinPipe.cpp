#include "pch.h"
#include "WinPipe.h"

const wchar_t* PIPE_NAME = L"\\\\.\\pipe\\{2145AB63-BF83-40A4-8A9D-A358D45AF1C1}";

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

namespace xp11_va::platform::windows {
	/* PUBLIC API */

	WinPipe::WinPipe() : pipe(INVALID_HANDLE_VALUE), connected(false) {
		pipe = CreateNamedPipe(
			PIPE_NAME,
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFFER_SIZE,
			BUFFER_SIZE,
			0, //default 50ms
			nullptr);

		if (pipe == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Failed to create pipe handle: " + lastErrorToString());
		}
	}
	
	WinPipe::~WinPipe() {
		if (pipe != INVALID_HANDLE_VALUE) {
			DisconnectNamedPipe(pipe);
			CloseHandle(pipe);
		}
	}

	void WinPipe::Connect() {
		if (!ConnectNamedPipe(pipe, nullptr)) {
			if (GetLastError() == ERROR_OPERATION_ABORTED) {
				return;
			}
			throw std::runtime_error("Error connecting pipe: " + lastErrorToString());
		}
		connected = true;
	}

	bool WinPipe::IsConnected() {
		return connected;
	}
	
	std::optional<std::string> WinPipe::ReadPipe() {
		if (!connected) {
			throw std::runtime_error("Attempt to read from non-connected pipe!");
		}
		
		char buffer[BUFFER_SIZE];
		DWORD bytesRead;
		
		if(ReadFile(pipe, buffer, BUFFER_SIZE, &bytesRead, nullptr)) {
			if (bytesRead < BUFFER_SIZE - 1) {
				buffer[bytesRead] = 0;
			} else {
				buffer[BUFFER_SIZE - 1] = 0;
			}
			return buffer;
		}

		if (GetLastError() == ERROR_OPERATION_ABORTED) {
			return {};
		}

		if (GetLastError() == ERROR_BROKEN_PIPE) {
			throw std::runtime_error("Client disconnected");
		}

		throw std::runtime_error("Error reading from pipe: " + lastErrorToString());
	}

	bool WinPipe::WritePipe(std::string msg) {
		if (!connected) {
			throw std::runtime_error("Attempt to write to non-connected pipe!");
		}
		
		DWORD bytesWritten;

		if (WriteFile(pipe, msg.c_str(), msg.length() * sizeof(std::string::value_type), &bytesWritten, nullptr)) {
			return true;
		}

		if (GetLastError() == ERROR_OPERATION_ABORTED) {
			return false;
		}

		if (GetLastError() == ERROR_BROKEN_PIPE) {
			throw std::runtime_error("Client disconnected");
		}

		throw std::runtime_error("Error writing to pipe: " + lastErrorToString());
	}

	void WinPipe::Abort(std::thread::native_handle_type handle) {
		CancelSynchronousIo(handle);
	}
}