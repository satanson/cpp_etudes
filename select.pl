#!/usr/bin/perl
use strict;
use warnings;

my $file = shift or die "missing <file>";
my $pattern_file = shift or die "missing <pattern>";

my @pattern = map {chomp;$_} qx(cat $pattern_file);
my @lines = map{chomp;$_} qx(cat $file);

sub any(\@$){
  my ($args, $pred) = @_;
  my $num =()= grep {$pred->($_)} @$args;
  print "num=$num\n";
  return $num;
}

@lines = grep{
  my $line = $_;
  my $pred = sub {
    index($line, $_[0]) >= 0;
  };
  any(@pattern, $pred);
} @lines;

my $new_file = "$file.selected";
open my $new_file_h, ">$new_file" or die "Fail to open '$new_file' for writing: $!";
print $new_file_h join("\n", @lines);
close($new_file_h);
