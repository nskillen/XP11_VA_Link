#include "pch.h"
#include "EnvData.h"
#include "Logger.h"

namespace xp11_va {
	EnvData EnvData::fromString(const std::string& dataref_name, const std::string& dataref_type, const std::string& dataref_value) {
		const auto type = std::strtol(dataref_type.c_str(), nullptr, 10);

		EnvData ed{};
		ed.name = dataref_name;
		ed.type = type;

		std::string val;

		switch (type) {
		case xplmType_Int:
			ed.intVal = std::strtol(dataref_value.c_str(), nullptr, 10);
			break;
		case xplmType_Float:
			ed.floatVal = std::strtof(dataref_value.c_str(), nullptr);
			break;
		case xplmType_Double:
			ed.doubleVal = std::strtod(dataref_value.c_str(), nullptr);
			break;
		case xplmType_FloatArray:
		{
			size_t v_start = 0;
			size_t v_end = dataref_value.find(',', v_start);
			while (ed.arrayElemCount < MAX_ARRAY_ELEMS) {
				ed.floatArray[ed.arrayElemCount] = std::strtof(dataref_value.substr(v_start, v_end - v_start).c_str(), nullptr);
				ed.arrayElemCount += 1;

				if (v_end == std::string::npos) { break; }
				v_start = v_end + 1;
				v_end = dataref_value.find(',', v_start);
			}
			break;
		}
		case xplmType_IntArray:
		{
			size_t v_start = 0;
			size_t v_end = dataref_value.find(',', v_start);
			while (ed.arrayElemCount < MAX_ARRAY_ELEMS) {
				ed.intArray[ed.arrayElemCount] = std::strtol(dataref_value.substr(v_start, v_end - v_start).c_str(), nullptr, 10);
				ed.arrayElemCount += 1;

				if (v_end == std::string::npos) { break; }
				v_start = v_end + 1;
				v_end = dataref_value.find(',', v_start);
			}
			break;
		}
		case xplmType_Data:
			ed.arrayElemCount = min(dataref_value.length() * sizeof(std::string::value_type), MAX_ARRAY_ELEMS);
			memcpy_s(ed.byteArray, MAX_ARRAY_ELEMS, dataref_value.c_str(), ed.arrayElemCount);
			break;
		case xplmType_Unknown:
		default:
			throw std::runtime_error("Unknown dataref type " + std::to_string(type));
		}

		return ed;
	}

	EnvData EnvData::fromDataref(const std::string&name, XPLMDataRef ref) {
		if (!ref) {
			throw std::runtime_error("No ref passed in toEnvData");
		}

		EnvData res{};
		res.name = name;

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

	std::string EnvData::dataToString() {
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
}