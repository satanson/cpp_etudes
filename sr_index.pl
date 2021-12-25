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
    die "undefined time format!'$t'";
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

  }
}

my $pipelines=[map {values %$_} map {values %$_}  map {values %$_} values %$plan];
my $drivers=[map {values %{$_->{drivers}}} map {values %$_} map {values %$_}  map {values %$_} values %$plan];
my $ops=[map {values %{$_->{operators}}} map {values %{$_->{drivers}}} map {values %$_} map {values %$_}  map {values %$_} values %$plan];

my $index="ActiveTime";
if (exists $ENV{index}){
   $index=$ENV{index};
}

my @pipelines = grep {exists $_->{$index}} @$pipelines;
my @drivers = grep {exists $_->{$index}} @$drivers;
my @ops= grep {exists $_->{$index}} @$ops;
print join "\n", map {sprintf "%0.3f\t%s\t%s", $_->{$index}, $index, $_->{id}} sort{$a->{$index}<=>$b->{$index}} (@pipelines, @drivers, @ops);
print "\n";
