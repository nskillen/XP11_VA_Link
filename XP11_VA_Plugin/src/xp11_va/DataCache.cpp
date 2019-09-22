#include "pch.h"
#include "DataCache.h"

namespace xp11_va {
	DataCache::DataCache() {}

	DataCache::~DataCache() {}

	EnvData DataCache::getData(const std::string& dataRef) {
		XPLMDataRef ref = nullptr;

		auto iter = refCache.find(dataRef);
		if (iter == refCache.end()) {
			ref = createDataRef(dataRef);
			refCache.insert(std::make_pair(dataRef, ref));
		}
		else {
			ref = iter->second;
		}

		return toEnvData(ref);
	}

	XPLMDataRef DataCache::createDataRef(const std::string& dataRef) {
		XPLMDataRef ref = XPLMFindDataRef(dataRef.c_str());
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
		XPLMDataTypeID idMask = XPLMGetDataRefTypes(ref);
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