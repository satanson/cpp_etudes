#!/usr/bin/perl
use strict;
use warnings;
use Data::Dumper;

my $handler = sub {
  my $args = join qq/,/,@_;
  print "Exit caused by die: $args\n";
  exit 0;
};

@SIG{qw/__DIE__ ABRT INT QUIT TERM/}=($handler) x 100;
print Dumper(\%SIG);
my $a=shift;
if (defined($a) && ($a cmp "Y")){
  print "register END\n";
  END{
    @SIG{keys %SIG} = qw/DEFAULT/ x (keys %SIG);
    print "EXIT normally\n";
  }
}

print "pid=$$\n";
die "absc";
sleep 100;
