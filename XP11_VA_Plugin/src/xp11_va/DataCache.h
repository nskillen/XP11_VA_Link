#pragma once

namespace xp11_va {
	constexpr size_t MAX_ARRAY_ELEMS = 1024;
	
	struct EnvData {
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

		static EnvData fromString(const std::string&);

		std::string ToString() {
			return std::to_string(type) + ";" + DataToString();
		}

		std::string DataToString();
	};

	class DataCache {
	public:
		EnvData getData(const std::string& dataRef);
		void setData(const std::string& name, EnvData data);
		
	private:
		std::map<std::string, XPLMDataRef> refCache;

		static XPLMDataRef createDataRef(const std::string& dataRef);
		static EnvData toEnvData(XPLMDataRef ref);
	};
}