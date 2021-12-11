#!/usr/bin/perl

use warnings;
use strict;
use List::Util qw(min max sum);
use Data::Dumper;

my $fileName=shift or die "missing 'fileName'";
my $indexName=shift or die "missing 'indexName'";

open my $fh, "< $fileName" or die "$!";
my $indexNamePat0=qr/\b$indexName\b/;
my $indexNamePat1=qr/\b$indexName\b\s*:\s*(\d+(?:\.\d+)?)(ns|us|ms)/;
my $indexNamePat2=qr/\b$indexName\b\s*:\s*(\d+)s(\d+)ms/;
sub norm_time1($$){
  my ($n,$u)=@_;
  if ($u eq "ns") {
    return $n/1000000000.0;
  } elsif ($u eq "us") {
    return $n/1000000.0;
  } elsif ($u eq "ms") {
    return $n;
  } else {
    return $n*1000.0;
  }
}
sub norm_time2($$) {
  my ($sec, $ms)=@_;
  return $sec*1000.0+$ms;
}

my @lines0=map {chomp;$_} <$fh>;
my @lines=grep {/$indexNamePat0/} map {$_.":".$lines0[$_-1]} 1..scalar(@lines0);
my @norm_lines1=map {/$indexNamePat1/;[norm_time1($1, $2), $_]} @lines;
my @norm_lines2=map {/$indexNamePat2/;[norm_time2($1, $2), $_]} @lines;
my @norm_lines=(@norm_lines1, @norm_lines2);
my @sorted_lines = sort {$a->[0] <=> $b->[0]} @norm_lines;
for my $line (@sorted_lines) {
  printf "cost=%d, %s\n", $line->[0], $line->[1];
}
