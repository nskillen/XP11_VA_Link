#pragma once

namespace xp11_va {
	class Pipe {
	public:
		static std::future<std::shared_ptr<Pipe>> get();

	public:
		Pipe() = default;
		virtual ~Pipe() = default;

		virtual std::future<bool> Connect() = 0;
		virtual std::future<std::string> ReadPipe() = 0;
		virtual std::future<bool> WritePipe(std::string) = 0;
	};
}