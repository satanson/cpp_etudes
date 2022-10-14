#include <chrono>
#include <string>
#include <thread>

#include "lru_cache/cache_manager.hh"
using CacheManager = starrocks::query_cache::CacheManager;
using CacheValue = starrocks::query_cache::CacheValue;
using Column = starrocks::query_cache::Column;
using ColumnPtr = starrocks::query_cache::ColumnPtr;
using Chunk = starrocks::query_cache::Chunk;
using ChunkPtr = starrocks::query_cache::ChunkPtr;
using CacheValue = starrocks::query_cache::CacheValue;
using namespace std::chrono_literals;
CacheValue create_cache_value(size_t n) {
    auto col = std::make_shared<Column>();
    col->resize(n);
    auto chunk = std::make_shared<Chunk>();
    chunk->append_column(col);
    CacheValue cache_value{.result = {chunk}};
    return cache_value;
}
int main(int argc, char** argv) {
    auto cache_mgr = new CacheManager(512 * 1024 * 1024);
    for (int i = 0; i < 10000; ++i) {
        std::string cache_key = "key_" + std::to_string(i);
        auto status = cache_mgr->populate(cache_key, create_cache_value(8*1024 * 1024));
        if (!status.ok()) {
            std::cout << "key=" << cache_key << ", FAIL to populate it in cache" << std::endl;
        }
        {
            for (int k = 0; k < 100; ++k) {
                auto cache_value = cache_mgr->probe(cache_key);
                if (!cache_value.ok()) {
                    std::cout << "key=" << cache_key << ", NOT FOUND" << std::endl;
                } else {
                    std::cout << "key=" << cache_key << ", FOUND, bytes=" << cache_value->size() << std::endl;
                }
            }
        }
    }
    std::this_thread::sleep_for(3s);
}