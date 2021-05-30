#!/bin/bash
git branch |grep '\.tmp\.' |xargs -i{} git branch -D '{}'
