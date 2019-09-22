#pragma once

namespace xp11_va {
	class Pipe {
	public:
		static std::shared_ptr<Pipe> get();

	public:
		Pipe() = default;
		virtual ~Pipe() = default;

		virtual void Connect() = 0;
		virtual bool IsConnected() = 0;
		virtual std::optional<std::string> ReadPipe() = 0;
		virtual bool WritePipe(std::string) = 0;
		virtual void Abort(std::thread::native_handle_type) = 0;
	};
}