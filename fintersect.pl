#!/usr/bin/perl
use strict;
use warnings;

my %linesA = map {chomp;($_,1)} qx(cat $ARGV[0]);
my %linesB = map {chomp;($_,1)} qx(cat $ARGV[1]);
print join "\n", grep { exists $linesB{$_} } sort keys %linesA;
print "\n";
