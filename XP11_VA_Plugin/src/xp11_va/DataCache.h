#pragma once

#include <map>
#include <sstream>
#include <string>
#include <XPLM/XPLMDataAccess.h>

constexpr size_t MAX_ARRAY_ELEMS = 1024;

namespace xp11_va {
	struct EnvData {
		XPLMDataTypeID type;
		size_t arrayElemCount;
		union {
			int32_t intVal;
			float floatVal;
			double doubleVal;
			int8_t byteArray[MAX_ARRAY_ELEMS];
			int32_t intArray[MAX_ARRAY_ELEMS];
			float floatArray[MAX_ARRAY_ELEMS];
		};

		std::string ToString() {
			if (type & xplmType_Int) {
				return "int;;" + DataToString();
			}
			else if (type & xplmType_Float) {
				return "float;;" + DataToString();
			}
			else if (type & xplmType_Double) {
				return "double;;" + DataToString();
			}
			else if (type & xplmType_Data) {
				return "byte[];;" + DataToString();
			}
			else if (type & xplmType_IntArray) {
				return "int[];;" + DataToString();
			}
			else if (type & xplmType_FloatArray) {
				return "float[];;" + DataToString();
			}
			else {
				throw std::runtime_error("Unknown data type id " + std::to_string(type));
			}
		}

		std::string DataToString() {
			if (type & xplmType_Int) {
				return std::to_string(intVal);
			}
			else if (type & xplmType_Float) {
				return std::to_string(floatVal);
			}
			else if (type & xplmType_Double) {
				return std::to_string(doubleVal);
			}
			else if (type & xplmType_Data) {
				std::stringstream ss;
				for (size_t i = 0; i < arrayElemCount; i++) {
					if (i < arrayElemCount - 1) {
						ss << byteArray[i] << ",";
					}
					else {
						ss << byteArray[i];
					}
				}
				return ss.str();
			}
			else if (type & xplmType_IntArray) {
				std::stringstream ss;
				for (size_t i = 0; i < arrayElemCount; i++) {
					if (i < arrayElemCount - 1) {
						ss << intArray[i] << ",";
					}
					else {
						ss << intArray[i];
					}
				}
				return ss.str();
			}
			else if (type & xplmType_FloatArray) {
				std::stringstream ss;
				for (size_t i = 0; i < arrayElemCount; i++) {
					if (i < arrayElemCount - 1) {
						ss << floatArray[i] << ",";
					}
					else {
						ss << floatArray[i];
					}
				}
				return ss.str();
			}
			else {
				throw std::runtime_error("Unknown dataref type id " + std::to_string(type));
			}
		}
	};

	class DataCache {
	public:
		DataCache();
		~DataCache();

		EnvData getData(const std::string& dataRef);
	private:
		std::map<std::string, XPLMDataRef> refCache;

		XPLMDataRef createDataRef(const std::string& dataRef);
		EnvData toEnvData(XPLMDataRef ref);
	};
}