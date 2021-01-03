#pragma once

namespace xp11_va {
	constexpr size_t MAX_ARRAY_ELEMS = 1024;

	struct EnvData {
		std::string name;
		XPLMDataTypeID type;
		size_t arrayElemCount;
		union {
			int32_t intVal;
			float floatVal;
			double doubleVal;
			int32_t intArray[MAX_ARRAY_ELEMS];
			float floatArray[MAX_ARRAY_ELEMS];
			uint8_t byteArray[MAX_ARRAY_ELEMS];
		};

		static EnvData fromString(const std::string&, const std::string&, const std::string&);
		static EnvData fromDataref(const std::string&, XPLMDataRef);

		inline std::string ToString() {
			return name + ":" + std::to_string(type) + ":" + dataToString();
		}

	private:
		std::string dataToString();
	};
}