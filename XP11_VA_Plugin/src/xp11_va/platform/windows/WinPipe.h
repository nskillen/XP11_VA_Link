#pragma once

#include "xp11_va/Pipe.h"
#include <Windows.h>

namespace xp11_va::platform::windows {
	constexpr size_t BUFFER_SIZE = 1024;

	class WinPipe final : public Pipe {
	public:
		WinPipe();
		virtual ~WinPipe() override;

		std::future<bool> Connect() override;
		std::future<std::string> ReadPipe() override;
		std::future<bool> WritePipe(std::string) override;
		
	private:
		template <typename T>
		struct OVERLAPPED_BUFFER {
			OVERLAPPED overlap;
			char buffer[BUFFER_SIZE];
			DWORD count;
			std::promise<T> promise;
		};

		HANDLE pipe;
		
		std::promise<bool> connectPromise;
		std::thread connectThread;
		OVERLAPPED connectOverlap;
		HANDLE connectEvent;

		OVERLAPPED_BUFFER<std::string> readOverlap;
		OVERLAPPED_BUFFER<bool> writeOverlap;

		friend void readCompleteCallback(DWORD, DWORD, LPOVERLAPPED);
		friend void writeCompleteCallback(DWORD, DWORD, LPOVERLAPPED);
	};
}