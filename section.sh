#!/bin/bash
set -e -o pipefail

file=${1:?"undefined file"};shift
section=${1:?"undefined section"};shift

perl -lne 'unless(/^\s*$/){push @L,$_} elsif(scalar(@L)==0 || $L[-1]=~/\S/){push @L, $_}}{while($L[-1]=~/^\s*$/){pop @L};print "\n", join "\n", @L' $file |
perl -lpe "BEGIN{\$n=0;} do {s/^\s*$/${section}_\$n/g; \$n++} if /^\s*$/"
