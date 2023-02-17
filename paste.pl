#!/usr/bin/perl

use strict;
use warnings;
use List::Util qw/max/;
use Data::Dumper;
my $n = shift or die "Missing argument 'n'";
my $sep = shift or die "Missing argument 'sep'";
my $fill = shift or die "Missing argument 'fill'";

my @ifiles=map {open my $h, "<$_" or die "Open file '$_' error=$!";$h} @ARGV;

my @contents = map{my $h=$_;[map{chomp $_;$_} <$h>]} @ifiles;
@contents = map {[@{$_}[0..$n-1]] } map{[@$_, ($fill) x $n]} @contents;

#print Dumper(\@contents);

for my $i (0..$n-1) {
  print join "$sep", map {$contents[$_][$i]} 0..$#contents;
  print "\n";
}
