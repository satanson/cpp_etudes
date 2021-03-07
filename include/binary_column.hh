// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/19.
//

#ifndef CPP_ETUDES_INCLUDE_BINARY_COLUMN_HH_
#define CPP_ETUDES_INCLUDE_BINARY_COLUMN_HH_
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

class Slice;
class BinaryColumn;

struct Slice {
  char *data;
  size_t size;
  std::string to_string() { return std::string(data, data + size); }
  const char *begin() { return data; }
  const char *end() { return data + size; }
};

struct BinaryColumn {
  std::vector<uint8_t> bytes;
  std::vector<uint32_t> offsets;
  BinaryColumn() : bytes({}), offsets({0}) {}
  std::vector<uint8_t> &get_bytes() { return bytes; }
  std::vector<uint32_t> &get_offsets() { return offsets; }
  size_t size() const { return offsets.size() - 1; }
  void append(std::string const &s) {
    bytes.insert(bytes.end(), s.begin(), s.end());
    offsets.push_back(offsets.back() + s.size());
  }

  uint32_t get_max_serialized_size() {
    uint32_t max_size = 0;
    const auto num_rows = offsets.size() - 1;
    for (auto i = 0; i < num_rows; ++i) {
      max_size = std::max(max_size, offsets[i + 1] - offsets[i]);
    }
    return max_size + sizeof(uint32_t);
  }

  uint32_t serialize(size_t idx, uint8_t *pos) {
    uint32_t binary_size = offsets[idx + 1] - offsets[idx];
    uint32_t offset = offsets[idx];

    memcpy(pos, &binary_size, sizeof(uint32_t));
    memcpy(pos + sizeof(uint32_t), &bytes[offset], binary_size);
    return sizeof(uint32_t) + binary_size;
  }

  void serialize_batch(uint8_t *dst, std::vector<uint32_t> &slice_sizes,
                       size_t chunk_size, uint32_t max_one_row_size) {
    for (size_t i = 0; i < chunk_size; ++i) {
      slice_sizes[i] +=
          serialize(i, dst + i * max_one_row_size + slice_sizes[i]);
    }
  }

  void append(const char *begin, const char *end) {
    bytes.insert(bytes.end(), begin, end);
    offsets.push_back(offsets.back() + (end - begin));
  }

  Slice get_slice(int i) const {
    return {.data = (char *)bytes.data() + offsets[i],
            .size = static_cast<size_t>(offsets[i + 1] - offsets[i])};
  }

  Slice get_last_slice() { return get_slice(size() - 1); }

  std::vector<std::string> to_vector() {
    std::vector<std::string> ss;
    ss.reserve(size());
    for (auto i = 0; i < size(); ++i) {
      ss.push_back(get_slice(i).to_string());
    }
    return ss;
  }
};
#endif // CPP_ETUDES_INCLUDE_BINARY_COLUMN_HH_
