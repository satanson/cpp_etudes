#!/bin/bash
find -name "*.bak" |xargs -i{} rm -fr '{}'
