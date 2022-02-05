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

  if (/(MemoryLimit|PeakMemoryUsage):\s+(\S+)/) {
    $plan->{$fragment}{$instance}{$1}=$2;
    $plan->{$fragment}{$instance}{id}=$instance;
    next;
  }

  if (/DegreeOfParallelism:\s+(\d+)/) {
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{DegreeOfParallelism}=$1+0;
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
    my $operator_id = join "_", ($fragment, $instance, $pipeline, $driver, $operator);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{$1}=norm_time($2);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{id}=$operator_id;
    next;
  }

  if (/(PullChunkNum|PushChunkNum|PullRowNum|PushRowNum):\s+(\S+)/) {
    my $operator_id = join "_", ($fragment, $instance, $pipeline, $driver, $operator);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{$1}=norm_num($2);
    $plan->{$fragment}{$instance}{pipelines}{$pipeline}{drivers}{$driver}{operators}{$operator}{id}=$operator_id;
    next;
  }
}

my $instances=[map {values %$_} values %$plan];
my $pipelines=[map {values %$_}  map {$_->{pipelines}} values @$instances];
my $drivers=[map {values %$_}  map {$_->{drivers}} @$pipelines];
my $ops=[map {values %$_} map {$_->{operators}} @$drivers];

my $agg="max";
my $index="ActiveTime";
if (exists $ENV{index}){
  $index=$ENV{index};
}

if (exists $ENV{agg}){
  $agg=$ENV{agg};
}

my @instances = grep {exists $_->{$index}} @$instances;
my @pipelines = grep {exists $_->{$index}} @$pipelines;
my @drivers = grep {exists $_->{$index}} @$drivers;
my @ops= grep {exists $_->{$index} && $_->{id}=~/\)_(\w+\(-\d+\))$/} @$ops;

for my $op (@ops) {
  if ($op->{id}=~/\)_(\w+\(-\d+\))$/) {
    $op->{op_id}=$1;
  }
}
#print Dumper(\@ops);
use List::Util qw(max);
my $op_stat={};
for my $op (@ops) {
  if (!exists $op_stat->{$op->{op_id}}) {
    $op_stat->{$op->{op_id}} = 0;
  }
  if ($agg eq "max") {
    $op_stat->{$op->{op_id}} = max($op_stat->{$op->{op_id}}, $op->{$index});
  } else {
    $op_stat->{$op->{op_id}} += $op->{$index};
  }
}
print join "\n", map {sprintf "%0.3f\t%s\t%s", $op_stat->{$_}, $index, $_} sort{$op_stat->{$a}<=>$op_stat->{$b}} keys %$op_stat;
print "\n";
