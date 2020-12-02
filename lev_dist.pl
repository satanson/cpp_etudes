#!/usr/bin/perl
use strict;
use warnings;

use List::Util qw/min/;
use Data::Dumper;

sub lev_dist($$){
  my ($a, $b)=@_;
  my ($a_len, $b_len)=(0, 0);
  $a_len=length($a) if defined($a);
  $b_len=length($b) if defined($b);
  return $b_len if $a_len == 0;
  return $a_len if $b_len == 0;
  
  my @a = split //, lc $a;
  my @b = split //, lc $b;
  
  my ($ii, $jj) = ($a_len+1, $b_len+1);
  my @d = map{[(0) x $jj]} 1 .. $ii;
 
  for (my $i=0; $i < $ii; ++$i){
    $d[$i][0] = $i;
  }

  for (my $j=0; $j <= $jj; ++$j){
    $d[0][$j] = $j;
  }

  for (my $i=1; $i < $ii; ++$i){
    for (my $j=1; $j < $jj; ++$j){
      my ($ci, $cj) = ($a[$i-1], $b[$j-1]);
      if ($ci ge $cj) {
        $d[$i][$j] = $d[$i-1][$j-1];
      } else {
        $d[$i][$j] = 1 + min($d[$i-1][$j], $d[$i][$j-1], $d[$i-1][$j-1]);
      }
    }
  }
  return $d[-1][-1];
}

my $a = "result";
my @b = qw/result res resultabc PTransmitDataParams  PTransmitChunkParams PFetchDataResult/;

for my $b (@b) {
  my $dist = lev_dist($a,$b);
  print "lev_dist($a, $b)=$dist\n";
}
