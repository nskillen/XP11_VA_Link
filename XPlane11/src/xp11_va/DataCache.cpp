#include "pch.h"
#include "DataCache.h"

namespace xp11_va {
	/* PUBLIC API */
	XPLMDataRef DataCache::GetDataref(const std::string& datarefName) {
		if (refCache.find(datarefName) == refCache.end()) {
			const auto dataref = XPLMFindDataRef(datarefName.c_str());
			refCache.emplace(datarefName, dataref);
		}
		return refCache[datarefName];
	}

	void DataCache::Clear() {
		refCache.clear();
	}
}