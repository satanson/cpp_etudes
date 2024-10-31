#!/bin/bash
find -name "*.bak" |xargs -i{} rm -fr '{}'
find -name "*.backup.[0-9]*" |xargs -i{} rm -fr '{}'
find -name "*.backup" |xargs -i{} rm -fr '{}'
