#!/usr/bin/perl
use strict;
use warnings;
use Data::Dumper;


my ($fragment, $pipeline, $operator, $opid)=(undef)x6;

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
    die "undefined time format!'$_','$t'";
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
    die "undefined number format! '$_', '$n'";
    return undef;
  }
}

sub norm_bytes($) {
  my $b = shift;
  my %unit=(B=>1,KB=>1024,MB=>1024*1024,GB=>1024*1024*1024);
  if ($b=~/^(\d+(?:\.\d+)?)\s+(B|KB|MB|GB)?/) {
    my $u = $2;
    if (!defined($u)) {
      $u="B";
    }
    return ($1+0.0)*$unit{$u};
  } else {
    die "undefined number format! '$b'";
    return undef;
  }
}

while(<>) {
  if (/Fragment\s+(\d+)/){
    $fragment = "Fragment($1)";
    next;
  }

  if (/Pipeline\s+\(id=(\d+)\)/) {
    $pipeline = "Pipeline($1)";
    $plan->{$fragment}{pipelines}{$pipeline}{id}=join "_", ($fragment, $pipeline);
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

  if (/(MemoryLimit|PeakMemoryUsage|InstanceNum):\s+(\S+)/) {
    $plan->{$fragment}{$1}=$2;
    $plan->{$fragment}{id}=$fragment;
    next;
  }

  if (/(InstanceNum):\s+(\S+)/) {
    $plan->{$fragment}{$1}=$2+0;
    $plan->{$fragment}{id}=$fragment;
    next;
  }

  if (/DegreeOfParallelism:\s+(\d+)/) {
    $plan->{$fragment}{pipelines}{$pipeline}{DegreeOfParallelism}=$1+0;
    next;
  }

  if (/(ActiveTime|DriverTotalTime|OverheadTime|PendingTime|InputEmptyTime|FirstInputEmptyTime|FollowupInputEmptyTime|OutputFullTime|PreconditionBlockTime):\s+(\S+)/) {
    $plan->{$fragment}{pipelines}{$pipeline}{$1}=norm_time($2);
    next;
  }

  if (/(LocalRfWaitingSet|ScheduleAccumulatedChunkMoved|ScheduleAccumulatedRowsPerChunk|ScheduleCounter|ScheduleEffectiveCounter):\s+(\d+)/){
    $plan->{$fragment}{pipelines}{$pipeline}{$1}=$2;
    next;
  }

  if (!defined($operator)){
    next;
  }

  if (/\b(PushTotalTime|PullTotalTime|CompressTime|SetFinishingTime|BuildHashTableTime|RuntimeFilterBuildTime|CopyRightTableChunkTime|OtherJoinConjunctEvaluateTime|OutputBuildColumnTimer|OutputProbeColumnTimer|OutputTupleColumnTimer|ProbeConjunctEvaluateTime|__MAX_OF_ProbeConjunctEvaluateTime|__MIN_OF_ProbeConjunctEvaluateTime|SearchHashTableTimer|WhereConjunctEvaluateTime|OperatorTotalTime|SetFinishedTime|JoinRuntimeFilterTime|CloseTime)\b:\s+(\S+)/) {
    my $operator_id = join "_", ($fragment, $pipeline, $operator);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{$1}=norm_time($2);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{id}=$operator_id;
    next;
  }

  if (/(PullChunkNum|PushChunkNum|PullRowNum|DestID|PushRowNum):\s+(\S+)/) {
    my $operator_id = join "_", ($fragment, $pipeline, $operator);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{$1}=norm_num($2);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{id}=$operator_id;
    next;
  }
  if (/(BytesPassThrough|BytesSent|UpcompressedBytes):\s+(.*)/) {
    my $operator_id = join "_", ($fragment, $pipeline, $operator);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{$1}=$2;
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{id}=$operator_id;
    next;
  }
  if (/(RuntimeInFilterNum|RuntimeBloomFilterNum):\s+(.*)/) {
    my $operator_id = join "_", ($fragment, $pipeline, $operator);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{$1}=$2+0;
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{id}=$operator_id;
    next;
  }
}

my $fragments=[values %$plan];
my $pipelines=[map {values %$_}  map {$_->{pipelines}} values @$fragments];
my $ops=[map {values %$_} map {$_->{operators}} @$pipelines];

my $index="ActiveTime";
if (exists $ENV{index}){
   $index=$ENV{index};
}

my @fragments = grep {exists $_->{$index}} @$fragments;
my @pipelines = grep {exists $_->{$index}} @$pipelines;
my @ops= grep {exists $_->{$index}} @$ops;
print join "\n", map {sprintf "%s\t%s\t%s", "".$_->{$index}, $index, $_->{id}} sort{$a->{$index} <=> $b->{$index}} (@fragments, @pipelines, @ops);
print "\n";
