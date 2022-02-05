#!/usr/bin/perl
use strict;
use warnings;
use Data::Dumper;


my ($fragment, $instance, $operator, $opid)=(undef)x4;

my %opcost=();
my @uniq_keys=();
my $plan={};

sub norm_time($) {
  my $t = shift;
  my %unit=(ns=>1000000, us=>1000, ms=>1);

  if ($t=~/^(\d+(?:\.\d+)?)(ns|us|ms)$/){
    return ($1+0.0)/$unit{$2};
  }
  elsif ($t=~/^(\d+)s(\d+)ms$/) {
    return ($1+0)*1000+$2;
  }
  elsif ($t=~/^-(\d+(?:\.\d+)?)(ns|us|ms)$/){
    return -($1+0.0)/$unit{$2};
  }
  else {
    die "undefined time format!'$t'";
    return undef;
  }
}

sub norm_num($) {
  my $n = shift;
  my %unit=(M=>1000000, K=>1000);
  if ($n=~/^(\d+(?:\.\d+)?)(M|K)$/) {
    return ($1+0.0)*$unit{$2};
  } elsif ($n=~/^\d+$/) {
    return $n+0;
  } else {
    die "undefined number format!";
    return undef;
  }
}

my $pipeline_dop={};

while(<>) {
  if (/Fragment\s+(\d+)/){
    $fragment = "Fragment($1)";
    next;
  }
  if (/Instance\s+(\S+)/){
    $instance = $fragment."_Instance($1)";
    next;
  }

  if (/(\w+)\s+\(id=(\d+)\):\(Active:\s+(\S+?)\[/) {
    $operator = "$1($2)";
    my $id = join "_", ($fragment, $instance, $operator);
    $plan->{$fragment}{instances}{$instance}{operators}{$operator}{op_id}=$operator;
    $plan->{$fragment}{instances}{$instance}{operators}{$operator}{id}=$id;
    $plan->{$fragment}{instances}{$instance}{operators}{$operator}{plan_node_id}=$2;
    $plan->{$fragment}{instances}{$instance}{operators}{$operator}{ActiveTime}=norm_time($3);
    next;
  }
  if (/DataStreamSender\s+\(dst_id=(\d+).*Active:\s+(\S+?)\[/) {
    $operator = "DataStreamSender($1)";
    my $id = join "_", ($fragment, $instance, $operator);
    $plan->{$fragment}{instances}{$instance}{operators}{$operator}{op_id}=$operator;
    $plan->{$fragment}{instances}{$instance}{operators}{$operator}{id}=$id;
    $plan->{$fragment}{instances}{$instance}{operators}{$operator}{plan_node_id}=$1;
    $plan->{$fragment}{instances}{$instance}{operators}{$operator}{ActiveTime}=norm_time($2);
    next;
  }
  
  if (/ScanTime:\s+(\S+)/) {
    $plan->{$fragment}{instances}{$instance}{operators}{$operator}{ActiveTime}=norm_time($1);
  }
}

my $instances=[map {values %$_} values %$plan];
my $ops=[map {values %$_}  map {$_->{operators}} map {values %$_} @$instances];

my @ops=@$ops;
my $op_dop={};
use List::Util qw(max);
for my $op (@ops) {
  my $opid=$op->{op_id};
  if (!exists $op_dop->{$opid}){
    $op_dop->{$opid}{dop} = 0;
    $op_dop->{$opid}{plan_node_id} = $op->{plan_node_id};
    $op_dop->{$opid}{instance_num} = 0;
    $op_dop->{$opid}{ActiveTime}=$op->{ActiveTime};
    $op_dop->{$opid}{id} = $op->{id};
  }
  $op_dop->{$opid}{dop} += 1;
  $op_dop->{$opid}{instance_num} += 1;
  if ($op_dop->{$opid}{ActiveTime} < $op->{ActiveTime}) {
    $op_dop->{$opid}{ActiveTime} = $op->{ActiveTime};
    $op_dop->{$opid}{id} = $op->{id};
  }
}
#print Dumper($op_dop);
my $sort_by="plan_node_id";
if (exists $ENV{plan_node_id}) {
  $sort_by="ActiveTime";
}
print join "\n", map {
  sprintf "id=%s\tdop=%s\tinst=%s\ttime=%0.3f\t%s\t%s",
  substr("".$op_dop->{$_}{plan_node_id}."    ", 0, 4),
  substr("".$op_dop->{$_}{dop}."   ", 0, 3),
  substr("".$op_dop->{$_}{instance_num}."    ", 0, 4),
  $op_dop->{$_}{ActiveTime},
  $_,
  $op_dop->{$_}{id}
} sort{$op_dop->{$a}{$sort_by}<=> $op_dop->{$b}{$sort_by}}  keys %$op_dop;

print "\n";
