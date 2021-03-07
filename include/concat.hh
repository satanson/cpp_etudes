// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/19.
//

#ifndef CPP_ETUDES_INCLUDE_CONCAT_HH_
#define CPP_ETUDES_INCLUDE_CONCAT_HH_
#include <binary_column.hh>
#include <memory>
#include <raw_container.hh>
typedef std::shared_ptr<BinaryColumn> ColumnPtr;
typedef std::vector<ColumnPtr> Columns;

uint32_t get_max_serialize_size(const Columns &key_columns) {
  uint32_t max_size = 0;
  for (const auto &key_column : key_columns) {
    max_size += key_column->get_max_serialized_size();
  }
  return max_size;
}

void concat_vertically(Columns &columns, std::vector<uint8_t> &buffer,
                       std::vector<uint32_t> &slice_sizes) {
  slice_sizes.assign(4096, 0);
  auto max_one_row_size = get_max_serialize_size(columns);
  buffer.assign(0, 0);
  raw::make_room(buffer, 4096 * max_one_row_size);
  for (const auto &key_column : columns) {
    key_column->serialize_batch(buffer.data(), slice_sizes, columns[0]->size(),
                                max_one_row_size);
  }
}

ColumnPtr concat_horizontally(Columns &columns) {
  ColumnPtr result = std::make_shared<BinaryColumn>();
  auto &bytes = result->get_bytes();
  auto &offsets = result->get_offsets();
  const size_t num_rows = columns[0]->size();
  raw::make_room(offsets, num_rows + 1);
  uint32_t bytes_size = 0;
  for (auto &col : columns) {
    bytes_size += col->get_bytes().size();
  }
  bytes.reserve(std::min(bytes_size, 16u << 20));

  for (int i = 0; i < num_rows; i++) {
    for (auto &col : columns) {
      auto s = col->get_slice(i);
      bytes.insert(bytes.end(), s.data, s.data + s.size);
    }
    offsets[i + 1] = bytes.size();
  }
  return result;
}

#endif // CPP_ETUDES_INCLUDE_CONCAT_HH_
