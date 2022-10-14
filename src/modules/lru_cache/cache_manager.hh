// This file is licensed under the Elastic License 2.0. Copyright 2021-present, StarRocks Limited.
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "lru_cache/lru_cache.hh"
#include "lru_cache/slice.hh"

namespace starrocks {
namespace query_cache {
using Status = absl::Status;
template <typename T>
using StatusOr = absl::StatusOr<T>;
class CacheManager;
using CacheManagerRawPtr = CacheManager*;
using CacheManagerPtr = std::shared_ptr<CacheManager>;
struct Column {
    std::vector<char> data;
    void reserve(size_t n) { data.reserve(n); }
    void resize(size_t n) { data.resize(n); }
    size_t size() const { return data.size(); }
};

using ColumnPtr = std::shared_ptr<Column>;
struct Chunk {
    std::vector<ColumnPtr> columns;
    void append_column(const ColumnPtr& column) { columns.push_back(column); }

    size_t bytes_usage() {
        size_t n = 0;
        for (const auto& column : columns) {
            n += column->size();
        }
        return n;
    }
};

using ChunkPtr = std::shared_ptr<Chunk>;
using CacheResult = std::vector<ChunkPtr>;

struct CacheValue {
    int64_t latest_hit_time;
    int64_t hit_count;
    int64_t populate_time;
    int64_t version;
    CacheResult result;
    size_t size() {
        size_t value_size = 0;
        for (auto& chk : result) {
            value_size += chk->bytes_usage();
        }
        return value_size;
    }
};

class CacheManager {
public:
    explicit CacheManager(size_t capacity);
    ~CacheManager() = default;
    Status populate(const std::string& key, const CacheValue& value);
    StatusOr<CacheValue> probe(const std::string& key);
    size_t memory_usage();
    size_t capacity();

private:
    ShardedLRUCache _cache;
};

} // namespace query_cache
} // namespace starrocks
