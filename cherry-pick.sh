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

branch_switch(){
  local br=${1:?"branch"};shift
  if (current_branch_is $br);then
    echo "INFO: already on current branch '$br'";
  elif (branch_exists $br);then
    git checkout $br
  else
    git checkout -b $br;
  fi
  ensure_current_branch $br;
}

branch_rename(){
  local br0=${1:?"branch0"};shift
  local br1=${1:?"branch1"};shift
  ensure_branch_exists $br0
  ensure_branch_not_exists $br1
  git branch -m $br0 $br1
}

now_ts(){
    date +"%Y%m%d_%H%M%S"
}

branch_backup(){
  local br=${1:?"branch"};shift
  local br_backup="${br}.backup.$(now_ts)"
  branch_rename ${br} ${br_backup}
  echo ${br_backup}
}

branch_topmost_commits(){
  local br=${1:?"branch"};shift
  local n=${1:?"num"};shift
  ensure_branch_exists ${br}
  git log  -${n} --pretty=oneline ${br} |perl -lne 'print $1 if /^(\S+)\s/' |tac
}
branch_cherry_pick_one(){
  local cmt=${1:?"commit"};shift
  git cherry-pick ${cmt}
}
branch_fetch(){
  local br=${1:?"branch"};shift
  git fetch origin ${br}:${br}
}

cherry_pick() {
  local br0=${1:?"base branch"};shift
  local br1=${1:?"target branch"};shift
  local n=${1:?"num"};shift
  local fetch_latest=${1:?"fetch_latest"};shift
  
  # force fetch latest branch
  if [  "x${fetch_latest}x" = "xfetchx" ];then
    if (branch_exists ${br0}); then
      branch_backup ${br0}
    fi
    ensure_branch_not_exists ${br0}
    branch_fetch ${br0}
  fi

  # fetch if not exists br0
  if ! (branch_exists ${br0});then
    branch_fetch ${br0}
  fi

  ensure_branch_exists ${br0}
  ensure_branch_exists ${br1}

  local br1_tmp=${br1}.tmp.$(now_ts)
  branch_switch ${br0}
  ensure_current_branch ${br0}
  ensure_branch_not_exists ${br1_tmp}
  branch_switch ${br1_tmp}
  ensure_current_branch ${br1_tmp}

  for cmt in $(branch_topmost_commits ${br1} ${n});do
    branch_cherry_pick_one ${cmt}
  done

  branch_switch ${br0}
  ensure_current_branch ${br0}
  branch_backup ${br1}
  branch_rename ${br1_tmp} ${br1}
  ensure_branch_exists $br1
  branch_switch $br1
}

set -e -o pipefail

baseBranch=${1:?"missing <baseBranch>"};shift
targetBranch=${1:?"missing <targetBranch>"};shift
commitNum=${1:?"missing <commitNum>"};shift
fetchLatest=${1:?"missing <fetchLatest>"};shift

echo "cherry-pick ${commitNum} commits from ${targetBranch} to ${baseBranch} fetchLatest=${fetchLatest}"
cherry_pick ${baseBranch} ${targetBranch} ${commitNum} ${fetchLatest}
