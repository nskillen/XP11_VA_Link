#pragma once

#include "EnvData.h"

namespace xp11_va {
	class DataCache {
	public:
		XPLMDataRef GetDataref(const std::string&);
		void Clear();
		
	private:
		std::map<std::string, XPLMDataRef> refCache;
	};
}