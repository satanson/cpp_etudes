#!/bin/bash

# Copyright (c) 2020 Ran Panfeng.  All rights reserved.
# Author: satanson
# Email: ranpanf@gmail.com
# Github repository: https://github.com/satanson/cpp_etudes.git
#
#basedir=$(cd $(dirname ${BASH_SOURCE:-$0});pwd);
#cd ${basedir}

branch_exists(){
  local br=${1:?"branch"};shift
  local res=$(git branch -v |perl -lne "print qq/ok/ if /^\\s*(\\*\\s*)?$br\\s+/"|head -1)
  if [ -n "$res" ];then
    return 0;
  else
    return 1;
  fi
}

ensure_branch_exists(){
  if !(branch_exists $1);then
    echo "ERROR: branch $1 not exists " >&2;
    exit 1
  fi
}

ensure_branch_not_exists(){
  if (branch_exists $1);then
    echo "ERROR: branch $1 exists " >&2
    exit 1
  fi
}

branch_current(){
  git branch -v |perl -lne "print \$1 if /^\\s*\\*\\s*(\\S+)\\s+/"|head -1
}

current_branch_is(){
  local br=${1:?"branch"};shift
  local cbr=$(branch_current)
  if [ "x${br}x" != "x${cbr}x" ];then
    return 1
  else
    return 0
  fi
}

ensure_current_branch(){
  local br=${1:?"branch"};shift
  if !(current_branch_is $br);then
    echo "ERROR: not on branch '${br}', current branch is '$(branch_current)'" >&2
    exit 1
  fi
}

set -e -o pipefail

notDryRun=${1}
cbr=$(branch_current)
cbr0=$(perl -le "print \$1 if qq/${cbr}/=~/^(.*)\\.tmp\\.\\d{8}_\\d{6}/")
if [ -s "${cbr0}" ];then
  echo "# ${cbr} is not a temp branch"
  exit 1
fi
echo "# mv ${cbr} ${cbr0}"
if (! branch_exists ${cbr0});then
  echo git checkout -b ${cbr0}
  if [ -n "${notDryRun}" ];then
    git checkout -b ${cbr0}
  fi
else
  echo git branch -D ${cbr0}
  echo git checkout -b ${cbr0}
  if [ -n "${notDryRun}" ];then
    git branch -D ${cbr0}
    git checkout -b ${cbr0}
  fi
fi
echo git branch -D ${cbr}
echo git push --force satanson ${cbr0}:${cbr0}
if [ -n "${notDryRun}" ];then
  git branch -D ${cbr}
  git push --force satanson ${cbr0}:${cbr0}
fi
