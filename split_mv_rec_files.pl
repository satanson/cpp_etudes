#!/usr/bin/perl

use strict;
use warnings;

my @ignores=(qr/^\s*$/);
my @matches=(qr/^--(?:\s*)?Id=(\d+)/);

my $file=undef;
while(<>){
  my $ignored="false";
  for my $igRe (@ignores){
    if ($_ =~ $igRe){
      $ignored="true";
      last;
    }
  }
  if ($ignored eq "true"){
    next;
  }

  my $matched="false";
  for my $mRe (@matches){
    if ($_ =~ $mRe){
      $matched="true";
      if (defined($file)){
        close($file);
      }
      my $name="mv_".substr("00$1",-2) . ".sql";
      open $file, "> $name" or die "$!";
      print $file "$_";
      last;
    }
  }

  if ($matched ne "true"){
    print $file "$_";
  }
}

if (defined($file)){
  close($file);
}
