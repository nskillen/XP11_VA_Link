#include "pch.h"
#include "DataCache.h"

#include "Logger.h"

namespace xp11_va {
	EnvData EnvData::fromString(const std::string& str) {
		Logger::get().Trace("EnvData::fromString called with " + str);
		const auto type = std::strtol(str.c_str(), nullptr, 10);
		const auto values = str.substr(str.find(';') + 1);

		EnvData ed{};
		ed.type = type;

		size_t start;
		std::string val;

		switch (type) {
		case xplmType_Int:
			ed.intVal = std::strtol(values.c_str(), nullptr, 10);
			break;
		case xplmType_Float:
			ed.floatVal = std::strtof(values.c_str(), nullptr);
			break;
		case xplmType_Double:
			ed.doubleVal = std::strtod(values.c_str(), nullptr);
			break;
		case xplmType_FloatArray:
			start = 0;
			while (start != std::string::npos && ed.arrayElemCount < MAX_ARRAY_ELEMS) {
				ed.floatArray[ed.arrayElemCount] = std::strtof(str.substr(start).c_str(), nullptr);
				ed.arrayElemCount += 1;
				start = values.find(',', start) + 1;
			}
			break;
		case xplmType_IntArray:
			start = 0;
			while (start != std::string::npos && ed.arrayElemCount < MAX_ARRAY_ELEMS) {
				ed.intArray[ed.arrayElemCount] = std::strtol(str.substr(start).c_str(), nullptr, 10);
				ed.arrayElemCount += 1;
				start = values.find(',', start) + 1;
			}
			break;
		case xplmType_Data:
			ed.arrayElemCount = min(values.length() * sizeof(std::string::value_type), MAX_ARRAY_ELEMS);
			memcpy_s(ed.byteArray, MAX_ARRAY_ELEMS, values.c_str(), ed.arrayElemCount);
			break;
		case xplmType_Unknown:
		default:
			throw std::runtime_error("Unknown dataref type " + std::to_string(type));
		}
		
		return ed;
	}

	std::string EnvData::DataToString() {
		if (type & xplmType_Int) {
			return std::to_string(intVal);
		}

		if (type & xplmType_Float) {
			return std::to_string(floatVal);
		}
		
		if (type & xplmType_Double) {
			return std::to_string(doubleVal);
		}
		
		if (type & xplmType_Data) {
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
		
		if (type & xplmType_IntArray) {
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
		
		if (type & xplmType_FloatArray) {
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

		throw std::runtime_error("Unknown dataref type id " + std::to_string(type));
	}
	
	EnvData DataCache::getData(const std::string& dataRef) {
		XPLMDataRef ref = nullptr;

		const auto iter = refCache.find(dataRef);
		if (iter == refCache.end()) {
			ref = createDataRef(dataRef);
			refCache.insert(std::make_pair(dataRef, ref));
		}
		else {
			ref = iter->second;
		}

		return toEnvData(ref);
	}

	void DataCache::setData(const std::string& name, EnvData data) {
		XPLMDataRef ref = nullptr;

		const auto iter = refCache.find(name);
		if (iter == refCache.end()) {
			ref = createDataRef(name);
			refCache.insert(std::make_pair(name, ref));
		}
		else {
			ref = iter->second;
		}

		if (XPLMCanWriteDataRef(ref)) {
			switch (data.type) {
			case xplmType_Int:
				XPLMSetDatai(ref, data.intVal);
				break;
			case xplmType_Float:
				XPLMSetDataf(ref, data.floatVal);
				break;
			case xplmType_Double:
				XPLMSetDatad(ref, data.doubleVal);
				break;
			case xplmType_FloatArray:
				XPLMSetDatavf(ref, data.floatArray, 0, data.arrayElemCount);
				break;
			case xplmType_IntArray:
				XPLMSetDatavi(ref, data.intArray, 0, data.arrayElemCount);
				break;
			case xplmType_Data:
				XPLMSetDatab(ref, data.byteArray, 0, data.arrayElemCount);
				break;
			case xplmType_Unknown:
			default:
				throw std::runtime_error("Unknown data ref type: " + std::to_string(data.type));
			}
		}
	}

	XPLMDataRef DataCache::createDataRef(const std::string& dataRef) {
		const auto ref = XPLMFindDataRef(dataRef.c_str());
		if (!ref) {
			throw std::runtime_error("Invalid ref: " + dataRef);
		}
		return ref;
	}

	EnvData DataCache::toEnvData(XPLMDataRef ref) {
		if (!ref) {
			throw std::runtime_error("No ref passed in toEnvData");
		}

		EnvData res{};
		const auto idMask = XPLMGetDataRefTypes(ref);
		res.type = idMask;

		if (idMask & xplmType_Int) {
			res.intVal = XPLMGetDatai(ref);
		}
		else if (idMask & xplmType_Float) {
			res.floatVal = XPLMGetDataf(ref);
		}
		else if (idMask & xplmType_Double) {
			res.doubleVal = XPLMGetDatad(ref);
		}
		else if (idMask & xplmType_IntArray) {
			res.arrayElemCount = XPLMGetDatavi(ref, res.intArray, 0, MAX_ARRAY_ELEMS);
		}
		else if (idMask & xplmType_FloatArray) {
			res.arrayElemCount = XPLMGetDatavf(ref, res.floatArray, 0, MAX_ARRAY_ELEMS);
		}
		else if (idMask & xplmType_Data) {
			res.arrayElemCount = XPLMGetDatab(ref, res.byteArray, 0, MAX_ARRAY_ELEMS);
		}
		else {
			throw std::runtime_error("Don't know how to get data of type " + std::to_string(idMask));
		}
		return res;
	}
}