#!/bin/perl
use Digest::SHA;
use warnings;
use strict;
use Data::Dumper;

my $topdir=shift @ARGV or die "missing argument!";
my @files=map {chomp; $_} qx(find $topdir -type f);
my $sha2files={};

my $sha512=Digest::SHA->new(512);
foreach my $f (@files) {
  $sha512->addfile($f);
  my $sha=$sha512->b64digest;
  if (! exists $sha2files->{$sha}) {
    $sha2files->{$sha}=[$f];
  } else {
    push @{$sha2files->{$sha}}, $f;
  }
}
#print Dumper($sha2files);
my $dupfiles={map {$_=>$sha2files->{$_}} grep {scalar(@{$sha2files->{$_}})>1} keys %$sha2files};
sub count(){
  my $n = 0;
  sub {
    $n++;
  };
}

for my $key (keys %$dupfiles) {
  my $cnt=count();

  print join "", map {
    $cnt->().":$key:".$_."\n"
  } map {
    $_->[0];
  } sort {
    my $r = $a->[1]<=>$b->[1];
    $r = ($r==0?$a->[2]<=>$b->[2]:$r);
    $r = ($r==0?$b->[3]<=>$a->[3]:$r);
    $r;
  } map{
    [$_, $_=~y/()//, -M $_, scalar(split "/", $_)]
  } @{$dupfiles->{$key}};
}
