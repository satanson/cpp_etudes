#!/bin/bash
set -e -o pipefail
perl -lne '@a=($_=~/(\bQ\d+\b)/g); push @b,@a;}{%h=map{$_,1} @b;print join ",", map{qq/"$_"/} sort{$a cmp $b} keys %h' $1
