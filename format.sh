#!/bin/bash
for dir in src benchmark unittest include;do
  ag -U -g '.*\.(hh|cc)$' -l ${dir} | xargs -i{} clang-format -i --style=file '{}'
done
