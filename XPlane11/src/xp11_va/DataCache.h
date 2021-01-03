#pragma once

#include <optional>

#include "EnvData.h"

namespace xp11_va {
	template <typename Key, typename Value>
	class DataCache {
	public:
		DataCache(std::function<Value(const Key&)> fetch) : fetch(fetch) {}

		std::optional<Value> Get(const Key& key) {
			if (cache.find(key) == cache.end()) {
				cache.emplace(key, fetch(key));
			}
			return cache[key];
		}

		void Clear() { cache.clear(); }

	private:
		std::map<Key, Value> cache;
		std::function<Value(const Key&)> fetch;
	};
}