#pragma once

#include "xp11_va/Pipe.h"
#include <Windows.h>

namespace xp11_va::platform::windows {
	constexpr size_t BUFFER_SIZE = 1024;

	class WinPipe final : public Pipe {
	public:
		WinPipe();
		~WinPipe() override;

		void Connect() override;
		bool IsConnected() override;
		std::optional<std::string> ReadPipe() override;
		bool WritePipe(std::string) override;
		void Abort(std::thread::native_handle_type) override;
		
	private:
		HANDLE pipe;
		bool connected;
	};
}