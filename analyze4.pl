#!/usr/bin/perl
use strict;
use warnings;
use Data::Dumper;


my ($fragment, $instance, $pipeline, $driver, $operator, $opid)=(undef)x6;

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
  else {
    die "undefined time format!";
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

while(<>) {
  if (/Fragment\s+(\d+)/){
    $fragment = "Fragment($1)";
    next;
  }
  if (/Instance\s+(\S+)/){
    $instance = "Instance($1)";
    next;
  }
  if (/Pipeline\s+\(id=(\d+)\)/) {
    $pipeline = "Pipeline($1)";
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{id}=join "_", ($fragment, $instance, $pipeline);
    next;
  }

  if (/PipelineDriver\s+\(id=(\d+)\):/){
    $driver = "Driver($1)";
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{id}=join "_", ($fragment, $instance, $pipeline, $driver);
    next;
  }

  if (/(\w+)\s+\(plan_node_id=(\d+)\):/){
    $operator = "$1($2)";
    $opid="plan_node_id=$2";
    next;
  }
  if (/(\w+)\s+\(pseudo_plan_node_id=(-\d+)\):/){
    $operator = "$1($2)";
    $opid="pseudo_plan_node_id=$2";
    next;
  }


  if (/DegreeOfParallelism:\s+(\d+)/) {
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{DegreeOfParallelism}=$1+0;
    next;
  }

  if (/(DriverActiveTime|DriverPendingTime|DriverInputEmptyTime|DriverFirstInputEmptyTime|DriverFollowupInputEmptyTime|DriverOutputFullTime|DriverPreconditionBlockTime|DriverTotalTime):\s+(\S+)/) {
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{$1}=norm_time($2);
    next;
  }

  if (/(LocalRfWaitingSet|ScheduleAccumulatedChunkMoved|ScheduleAccumulatedRowsPerChunk|ScheduleCounter|ScheduleEffectiveCounter):\s+(\d+)/){
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{$1}=$2;
    next;
  }

  if (/(?:Pull|Push)TotalTime:\s+(\S+)/) {
    if (!defined($operator)){
      next;
    }

    my $uniq_key = join "_", ($fragment, $instance, $pipeline, $driver, $operator);

    if (!exists $opcost{$uniq_key}) {
      $opcost{$uniq_key} = 0;
      $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{ops}{$uniq_key}{op}=$operator;
      $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{ops}{$uniq_key}{key}=$uniq_key;
      $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{ops}{$uniq_key}{id}=$opid;
    }
    $opcost{$uniq_key}+=norm_time($1);
  }

  if (/(PullChunkNum|PushChunkNum|PullRowNum|PushRowNum):\s+(\S+)/) {
    if (!defined($operator)){
      next;
    }
    my $uniq_key = join "_", ($fragment, $instance, $pipeline, $driver, $operator);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{ops}{$uniq_key}{$1}=norm_num($2);
  }
}

my @opcost=sort {$a->[1] <=> $b->[1]} map{[$_,$opcost{$_}]} keys %opcost;
#print join ("\n", map {sprintf("%0.3f", $_->[1])."\t". $_->[0]} @opcost);
#print "\n";
#print Dumper($plan);
my $ops=[map {values %{$_->{ops}}} map {values %{$_->{drivers}}} map {values %$_} map {values %$_}  map {values %$_} values %$plan];
#print Dumper($ops);

use List::Util qw(sum);
for my $op (@$ops) {
  $op->{AvgPullChunkSize}=$op->{PullChunkNum}==0? 0 : $op->{PullRowNum}/$op->{PullChunkNum};
  $op->{AvgPushChunkSize}=$op->{PushChunkNum}==0? 0 : $op->{PushRowNum}/$op->{PushChunkNum};
}

my $limit = 100;
if ((exists $ENV{limit}) && int($ENV{limit})>0) {
  $limit = int($ENV{limit});
}

if ((exists $ENV{type}) && $ENV{type} eq "pull") {
  my $index="AvgPullChunkSize";
  print join "\n", map {sprintf "%0.3f\t%d\t%s\t%s", $_->{$index}, $_->{PullChunkNum}, $index, $_->{id}} sort{$b->{$index}<=>$a->{$index}} grep {$_->{PullChunkNum}>$limit} @$ops;
  print "\n";
} else {
  my $index="AvgPushChunkSize";
  print join "\n", map {sprintf "%0.3f\t%d\t%s\t%s", $_->{$index}, $_->{PushChunkNum}, $index, $_->{id}} sort{$b->{$index}<=>$a->{$index}} grep {$_->{PushChunkNum}>$limit} @$ops;
  print "\n";
}
