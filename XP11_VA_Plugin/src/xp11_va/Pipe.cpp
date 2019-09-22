#include "pch.h"
#include "Pipe.h"

enum class Platform {
	Windows,
	MacOS,
	Linux
};

#ifdef _WIN32
#include "platform/windows/WinPipe.h"
constexpr Platform CURRENT_PLATFORM = Platform::Windows;
#elif DARWIN
#include "platform/macos/MacPipe.h"
constexpr Platform CURRENT_PLATFORM = Platform::MacOS;
#elif POSIX
#include "platform/linux/LinPipe.h"
constexpr Platform CURRENT_PLATFORM = Platform::Linux;
#else
#error Unsupported platform
#endif

namespace xp11_va {
	std::future<std::shared_ptr<Pipe>> Pipe::get() {
		return std::async([]() -> std::shared_ptr<Pipe> {
			switch (CURRENT_PLATFORM) {
			case Platform::Windows:
				return std::make_shared<platform::windows::WinPipe>();
			case Platform::MacOS:
				throw std::runtime_error("MacOS is not supported");
			case Platform::Linux:
				throw std::runtime_error("Linux is not supported");
			default:
				throw std::runtime_error("Unsupported platform");
			}
		});
	}
}