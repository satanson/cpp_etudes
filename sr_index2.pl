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
  elsif ($t=~/^(\d+)m(\d+)s$/) {
    return (($1+0)*60+$2)*1000;
  }
  elsif($t=~/^(\d+)m$/) {
    return ($1+0)*60*1000;
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
  my %unit=(B=>1000000000,M=>1000000, K=>1000);
  if ($n=~/^(\d+(?:\.\d+)?)(B|M|K)$/) {
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
    return ($1+0.0)*$unit{$u}/1024/1024/1024;
  } else {
    die "undefined number format! '$b'";
    return undef;
  }
}

while(<>) {
  if (/Fragment\s+(\d+)/){
    $fragment = "Fragment($1)";
    $pipeline = undef;
    $operator = undef;
    next;
  }


  if (/Pipeline\s+\(id=(\d+)\)/) {
    $pipeline = "Pipeline($1)";
    $operator = undef;
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

  if (/\b(InstanceNum):\s+(.*)/) {
    $plan->{$fragment}{$1}=$2;
    $plan->{$fragment}{id}=$fragment;
    next;
  }

  if (/\b(InstancePeakMemoryUsage|__MAX_OF_InstancePeakMemoryUsage|InstanceNum):\s+(.*)/) {
    $plan->{$fragment}{$1}=norm_bytes($2);
    $plan->{$fragment}{id}=$fragment;
    next;
  }

  if (/\b(MemoryLimit|PeakMemoryUsage|__MAX_OF_PeakMemoryUsage):\s+(.*)/) {
    if (defined($operator)){
      my $operator_id = join "_", ($fragment, $pipeline, $operator);
      $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{$1}=norm_bytes($2);
      $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{id}=$operator_id;
    } elsif(defined($pipeline)) {
      $plan->{$fragment}{pipelines}{$pipeline}{$1}=norm_bytes($2);
    } else {
      $plan->{$fragment}{$1}=norm_bytes($2);
      $plan->{$fragment}{id}=$fragment;
    }
    next;
  }


  if (/\b(InstanceNum):\s+(\S+)/) {
    $plan->{$fragment}{$1}=$2+0;
    $plan->{$fragment}{id}=$fragment;
    next;
  }

  if (/\b(FragmentInstancePrepareTime|__MAX_OF_FragmentInstancePrepareTime):\s+(\S+)/) {
    $plan->{$fragment}{$1}=norm_time($2);
    $plan->{$fragment}{id}=$fragment;
    next;
  }

  if (/\b(DegreeOfParallelism|TotalDegreeOfParallelism|PeakDriverQueueSize|__MAX_OF_PeakDriverQueueSize):\s+(\d+(?:(?:\.\d+)\w+)?)/) {
    $plan->{$fragment}{pipelines}{$pipeline}{$1}=norm_num($2);
    next;
  }

  if (/\b(PendingTime|__MAX_OF_PendingTime):\s+(\S+)/) {
    $plan->{$fragment}{pipelines}{$pipeline}{$1}=norm_time($2);
    next;
  }

  if (/\b(ActiveTime|__MAX_OF_DriverPrepareTime|DriverPrepareTime|DriverTotalTime|OverheadTime|ScheduleTime|PendingTime|InputEmptyTime|FirstInputEmptyTime|FollowupInputEmptyTime|OutputFullTime|PreconditionBlockTime):\s+(\S+)/) {
    $plan->{$fragment}{pipelines}{$pipeline}{$1}=norm_time($2);
    next;
  }

  if (/\b(LocalRfWaitingSet|ScheduleAccumulatedChunkMoved|ScheduleAccumulatedRowsPerChunk|ScheduleCounter|ScheduleEffectiveCounter):\s+(\d+)/){
    $plan->{$fragment}{pipelines}{$pipeline}{$1}=$2;
    next;
  }

  if (!defined($operator)){
    next;
  }

  if (/\b(PrepareTime|__MAX_OF_PrepareTime|__MIN_OF_PrepareTime|PushTotalTime|SortingTime|__MAX_OF_SortingTime|MergingTime|IOTaskExecTime|__MAX_OF_IOTaskExecTime|IOTaskWaitTime|__MAX_OF_IOTaskWaitTime|ScanTime|__MAX_OF_ScanTime|__MAX_OF_MergingTime|PullTotalTime|CompressTime|SetFinishingTime|BuildHashTableTime|RuntimeFilterBuildTime|CopyRightTableChunkTime|OtherJoinConjunctEvaluateTime|OutputBuildColumnTimer|OutputProbeColumnTimer|OutputTupleColumnTimer|ProbeConjunctEvaluateTime|__MAX_OF_ProbeConjunctEvaluateTime|__MIN_OF_ProbeConjunctEvaluateTime|SearchHashTableTimer|WhereConjunctEvaluateTime|OperatorTotalTime|SetFinishedTime|JoinRuntimeFilterTime|CloseTime)\b:\s+(\S+)/) {
    my $operator_id = join "_", ($fragment, $pipeline, $operator);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{$1}=norm_time($2);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{id}=$operator_id;
    next;
  }

  if (/\b(RowsRead|SubmitTaskCount|__MAX_OF_SubmitTaskCount|MorselsCount|__MAX_OF_MorselsCount|IOCounter|__MAX_OF_IOCounter|RawRowsRead|PullChunkNum|PushChunkNum|PullRowNum|DestID|PushRowNum|BlockCacheReadCounter|__MAX_OF_BlockCacheReadCounter|BlockCacheWriteCounter|__MAX_OF_BlockCacheWriteCounter):\s+(\S+)/) {
    my $operator_id = join "_", ($fragment, $pipeline, $operator);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{$1}=norm_num($2);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{id}=$operator_id;
    next;
  }
  if (/\b(BytesPassThrough|BytesSent|BlockCacheWriteBytes|__MAX_OF_BlockCacheWriteBytes|BlockCacheReadBytes|__MAX_OF_BlockCacheReadBytes|InputRequiredMemory|__MAX_OF_InputRequiredMemory|MergeUnsortedPeakMemoryUsage|__MAX_OF_MergeUnsortedPeakMemoryUsage|MergeSortedPeakMemoryUsage|__MAX_OF_MergeSortedPeakMemoryUsage|SortPartialPeakMemoryUsage|__MAX_OF_SortPartialPeakMemoryUsage|UpcompressedBytes|OperatorPeakMemoryUsage|__MAX_OF_OperatorPeakMemoryUsage):\s+(.*)/) {
    my $operator_id = join "_", ($fragment, $pipeline, $operator);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{$1}=norm_bytes($2);
    $plan->{$fragment}{pipelines}{$pipeline}{operators}{$operator}{id}=$operator_id;
    next;
  }
  if (/\b(RuntimeInFilterNum|RuntimeBloomFilterNum):\s+(.*)/) {
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
#print Dumper([@fragments, @pipelines, @ops]);
print join "\n", map {sprintf "%s\t\t%s\t%s", "".$_->{$index}, $index, $_->{id}} sort{$a->{$index} <=> $b->{$index}} (@fragments, @pipelines, @ops);
print "\n";
