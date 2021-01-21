#!/usr/bin/perl
#
# Copyright (c) 2020 Ran Panfeng.  All rights reserved.
# Author: satanson
# Email: ranpanf@gmail.com
# Github repository: https://github.com/satanson/cpp_etudes.git
#
use strict;
use warnings;

my %linesA = map {chomp;($_,1)} qx(cat $ARGV[0]);
my %linesB = map {chomp;($_,1)} qx(cat $ARGV[1]);
print join "\n", grep { exists $linesB{$_} } sort keys %linesA;
print "\n";
