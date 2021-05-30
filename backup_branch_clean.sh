#!/bin/bash
git branch |grep backup |xargs -i{} git branch -D '{}'
