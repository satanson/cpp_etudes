#!/usr/bin/perl
#
# Copyright (c) 2020 Ran Panfeng.  All rights reserved.
# Author: satanson
# Email: ranpanf@gmail.com
# Github repository: https://github.com/satanson/cpp_etudes.git

use strict;
use warnings;

my ($expect_fg, $expect_bg, $expect_ef) = @ARGV;

my @fg = (31 .. 37, 90 .. 97);
my @bg = (31 .. 47, 100 .. 106);
my @ef = (0 .. 8);
my $count = 0;
for (1 .. @fg * @bg * @ef) {
  #my $i=($_-1)/(@bg*@ef);
  #my $j=($_-1)%(@bg*@ef)/@ef;
  #my $k=($_-1)%@ef;
  my $i = ($_ - 1) / (@bg * @ef);
  my $j = ($_ - 1) % (@bg * @ef) / @ef;
  my $k = ($_ - 1) % @ef;
  my $fg = $fg[$i];
  my $bg = $bg[$j];
  my $ef = $ef[$k];

  if ((!defined($expect_fg) || $expect_fg == $fg) &&
    (!defined($expect_bg) || $expect_bg == $bg) &&
    (!defined($expect_ef) || $expect_ef == $ef)) {
    print "\e[${fg};${bg};${ef}m \\e[${fg};${bg};${ef}m\\e[m\e[m";
    if (($count+1) % 8 == 0) {
      print "\n";
    }
    else {
      print " ";
    }
    $count += 1;
  }
}
print "\n";
