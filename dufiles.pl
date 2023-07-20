#!/usr/bin/perl
use warnings;
use strict;
use Data::Dumper;
sub devno($){
  my $f=shift;
  my ($no)=map{chop;$_} qx(stat -c "%Hd.%Ld" "$f");
  return $no;
}
my $dev=devno(".");
die "invalid dev no:$dev" unless $dev=~/\d+\.\d+/;
my @subfiles=grep { $_ ne ".." && $_ ne "." }grep{devno($_) eq $dev}grep {(-f $_ || -d $_) && !(-l $_)} map {chop;$_} qx(ls -a);
my $subfiles=join "\n", map{qq/"$_"/} @subfiles;
$subfiles=~s/([ &()])/\\$1/g;
system(qq/echo -e "$subfiles" |xargs -i{} du -sh '{}'|sort -k1,1 -h/);
