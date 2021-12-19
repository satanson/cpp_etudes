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

  if (/(\d+(?:\.\d+)?)(ns|us|ms)/){
    return ($1+0.0)/$unit{$2};
  }
  elsif (/(\d+)s(\d+)ms/) {
    return ($1+0)*1000+$2;
  }
  else {
    die "undefined time format!";
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

  if (/(ActiveTime|PendingTime|OverheadTime|ScheduleTime|InputEmptyTime|FirstInputEmptyTime|FollowupInputEmptyTime|OutputFullTime|PreconditionBlockTime|OverallTime):\s+(\S+)/) {
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{$1}=norm_time($2);
    next;
  }

  if (/(ScheduleCounter|ScheduleEffectiveCounter):\s+(\d+)/){
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{$1}=$2+0;
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
      push @uniq_keys, $uniq_key;
      my $ops = $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{ops};
      if (!defined($ops)) {
        $ops=$plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{ops}=[];
        my $ops=$plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{ops};
        unshift @{$ops}, {op=>$operator, key=>$uniq_key, id=>$opid};
      } else {
        my $ops=$plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{ops};
        unshift @{$ops}, {op=>$operator, key=>$uniq_key, id=>$opid};
      }
    }
    $opcost{$uniq_key}+=norm_time($1);
  }
}

my @opcost=sort {$a->[1] <=> $b->[1]} map{[$_,$opcost{$_}]} keys %opcost;
#print join ("\n", map {sprintf("%0.3f", $_->[1])."\t". $_->[0]} @opcost);
#print "\n";
#print Dumper($plan);
my $drivers=[map {values %{$_->{drivers}}} map {values %$_} map {values %$_}  map {values %$_} values %$plan];

use List::Util qw(sum);
for my $d (@$drivers) {
  #$d->{OperatorTotalCost}=sum(map{$_->{Cost}=$opcost{$_->{key}}} @{$d->{ops}});
  #$d->{DriverActiveTime_sub_OperatorTotalCost}=$d->{DriverActiveTime}-$d->{OperatorTotalCost};
  #$d->{DriverTotalTime_sub_DriverActiveTime_sub_DriverPendingTime}=$d->{DriverTotalTime}-$d->{DriverActiveTime}-$d->{DriverPendingTime};
}
#print Dumper($drivers);
my $index="ScheduleTime";
if (exists $ENV{index}){
  $index=$ENV{index};
}
print join "\n", map {sprintf "%0.3f\t%s\t%s", $_->{$index}, $index, $_->{id}} sort{$a->{$index}<=>$b->{$index}} @$drivers;
print "\n";
