#!/usr/bin/perl

use strict;
use warnings;
my $n = shift or die "Missing argument 'n'";
my @ifiles=map {open my $h, "<$_" or die "Open file '$_' error=$!";$h} @ARGV;

my @contents = map{my $h=$_;[map{chomp $_;$_} <$h>]} @ifiles;
my @total_lines = map{scalar(@$_)} @contents;

for my $i (0..$n-1) {
  print join "\t", map {$contents[$_][$i%$total_lines[$_]]} 0..$#contents;
  print "\n";
}
