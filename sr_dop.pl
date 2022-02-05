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
  if (/Pipeline\s+\(id=(\d+)\)/) {
    $pipeline = "Pipeline($1)";
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{id}=join "_", ($instance, $pipeline);
    next;
  }

  if (/PipelineDriver\s+\(id=(\d+)\):/){
    $driver = "Driver($1)";
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{id}=join "_", ($instance, $pipeline, $driver);
    next;
  }

  if (/(\w+)\s+\(plan_node_id=(\d+)\):/){
    $operator = "$1($2)";
    $opid=$2;
    next;
  }
  if (/(\w+)\s+\(pseudo_plan_node_id=(-\d+)\):/){
    $operator = "$1($2)";
    $opid=$2;
    next;
  }

  if (/(MemoryLimit|PeakMemoryUsage):\s+(\S+)/) {
    $plan->{$fragment}{$instance}{$1}=$2;
    $plan->{$fragment}{$instance}{id}=$instance;
    next;
  }

  if (/DegreeOfParallelism:\s+(\d+)/) {
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{DegreeOfParallelism}=$1+0;
    my $pipeline_id=join "_", ($instance, $pipeline);
    $pipeline_dop->{$pipeline_id}=$1+0;
    next;
  }

  if (/(ActiveTime|PendingTime|OverheadTime|DriverTotalTime|ScheduleTime|InputEmptyTime|FirstInputEmptyTime|FollowupInputEmptyTime|OutputFullTime|PreconditionBlockTime|OverallTime):\s+(\S+)/) {
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{$1}=norm_time($2);
    next;
  }

  if (/(LocalRfWaitingSet|ScheduleAccumulatedChunkMoved|ScheduleAccumulatedRowsPerChunk|ScheduleCounter|ScheduleEffectiveCounter):\s+(\d+)/){
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{$1}=$2;
    next;
  }

  if (!defined($operator)){
    next;
  }

  if (/(PushTotalTime|PullTotalTime|SetFinishingTime|OperatorTotalTime|SetFinishedTime|JoinRuntimeFilterTime|CloseTime):\s+(\S+)/) {
    my $operator_id = join "_", ($instance, $pipeline, $driver, $operator);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{$1}=norm_time($2);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{id}=$operator_id;
    my $pipeline_id=join "_", ($instance, $pipeline);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{pipeline_id}=$pipeline_id;
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{operator_id}=$operator;
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{plan_node_id}=$opid;
    next;
  }

  if (/(PullChunkNum|PushChunkNum|PullRowNum|PushRowNum):\s+(\S+)/) {
    my $operator_id = join "_", ($instance, $pipeline, $driver, $operator);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{$1}=norm_num($2);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{id}=$operator_id;
    next;
  }
  if (/ScanTime:\s+(\S+)/) {
    my $operator_id = join "_", ($instance, $pipeline, $driver, $operator);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{OperatorTotalTime}=norm_time($1);
 
  }
}

my $instances=[map {values %$_} values %$plan];
my $pipelines=[map {values %$_}  map {$_->{pipelines}} values @$instances];
my $drivers=[map {values %$_}  map {$_->{drivers}} @$pipelines];
my $ops=[map {values %$_} map {$_->{operators}} @$drivers];

my @ops=@$ops;
my $op_dop={};
use List::Util qw(max);
for my $op (@ops) {
  my $operator=$op->{operator_id};
  if (!exists $op_dop->{$operator}){
    $op_dop->{$operator}{dop} = 0;
    $op_dop->{$operator}{plan_node_id} = $op->{plan_node_id};
    $op_dop->{$operator}{instance_num} = 0;
    $op_dop->{$operator}{OperatorTotalTime}=$op->{OperatorTotalTime};
    $op_dop->{$operator}{id} = $op->{id};
  }
  $op_dop->{$operator}{dop} += $pipeline_dop->{$op->{pipeline_id}};
  $op_dop->{$operator}{instance_num} += 1;
  if ($op_dop->{$operator}{OperatorTotalTime} < $op->{OperatorTotalTime}) {
    $op_dop->{$operator}{OperatorTotalTime} = $op->{OperatorTotalTime};
    $op_dop->{$operator}{id} = $op->{id};
  }
}
#print Dumper(\@ops);
#print Dumper($op_dop);
my $sort_by = "plan_node_id";
if (exists $ENV{sort_by}){
  $sort_by="OperatorTotalTime";
}
print join "\n", map {
  sprintf "id=%s\tdop=%s\tinst=%s\ttime=%0.3f\t%s\t%s",
  substr("".$op_dop->{$_}{plan_node_id}."    ", 0, 4),
  substr("".$op_dop->{$_}{dop}."   ",0, 3),
  substr("".$op_dop->{$_}{instance_num}."    ", 0, 4),
  $op_dop->{$_}{OperatorTotalTime},
  $_,
  $op_dop->{$_}{id}
} sort{$op_dop->{$a}{$sort_by} <=> $op_dop->{$b}{$sort_by}}  keys %$op_dop;
print "\n";
