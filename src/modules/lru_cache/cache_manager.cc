// This file is licensed under the Elastic License 2.0. Copyright 2021-present, StarRocks Limited.
#include "lru_cache/cache_manager.hh"

#include "absl/status/status.h"
#include "lru_cache/lru_cache.hh"
#include "lru_cache/slice.hh"

struct DeferOp {
    DeferOp(std::function<void()>&& f) : f(std::move(f)) {}
    ~DeferOp() { f(); }

private:
    std::function<void()> f;
};

namespace starrocks {
namespace query_cache {
using Status = absl::Status;
CacheManager::CacheManager(size_t capacity) : _cache(capacity) {}
static void delete_cache_entry(const CacheKey& key, void* value) {
    auto* cache_value = (CacheValue*)value;
    delete cache_value;
}

Status CacheManager::populate(const std::string& key, const CacheValue& value) {
    CacheValue* cache_value = new CacheValue(value);
    auto* handle = _cache.insert(key, cache_value, cache_value->size(), &delete_cache_entry, CachePriority::NORMAL);
    DeferOp defer([this, handle]() { _cache.release(handle); });
    if (_cache.get_memory_usage() > _cache.get_capacity()) {
        _cache.prune();
    }
    return handle != nullptr ? absl::OkStatus() : absl::InternalError("Insert failure");
}

static const Status CACHE_MISS = absl::NotFoundError("CacheMiss");

StatusOr<CacheValue> CacheManager::probe(const std::string& key) {
    auto* handle = _cache.lookup(key);
    if (handle == nullptr) {
        return CACHE_MISS;
    }
    DeferOp defer([this, handle]() { _cache.release(handle); });
    CacheValue cache_value = *reinterpret_cast<CacheValue*>(_cache.value(handle));
    return cache_value;
}

size_t CacheManager::memory_usage() {
    return _cache.get_memory_usage();
}

size_t CacheManager::capacity() {
    return _cache.get_capacity();
}

} // namespace query_cache
} // namespace starrocks
