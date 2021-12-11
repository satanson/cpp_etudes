#!/usr/bin/perl

use strict;
use warnings;
use Data::Dumper;

use constant {
  SECT_BEGIN=>1,
  SECT_FIRST=>2,
  SECT_REST=>3,
  SECT_END=>4,
  SECT_UNKNOWN=>5,
};

sub recognize($){
  local $_ = shift;
  if (/^thread_name="(.*?)"\s+thread_id=(0x[\da-f]+)/) {
    return (SECT_BEGIN, $1, $2);
  } elsif (/^\s+@\w+(?:\.\w+)*\.(\w+\.\w+)/) {
    return (SECT_FIRST, $1);
  } elsif (/^\s+at\s+\w+(?:\.\w+)*\.(\w+\.\w+)/) {
    return (SECT_REST, $1);
  } elsif (/^\s+$/) {
    return (SECT_END);
  } else {
    return (SECT_UNKNOWN);
  }
}

sub sect_begin(\%@){
  my ($ctx,$threadName, $threadId)=@_;
  $ctx->{partial}=[];
  $ctx->{thread}="$threadName(thread_id=$threadId)";
}

sub sect_body(\%@){
  my ($ctx,@args)=@_;
  unshift @{$ctx->{partial}}, $args[0];
}

sub sect_end($@){
  my ($ctx,@args)=@_;
  my $partial=$ctx->{partial};
  if (!defined($partial) || scalar(@$partial)==0){
    $ctx->{partial}=undef;
    return;
  }
  #print Dumper($ctx);
  #print Dumper($partial);
  unshift @$partial, $ctx->{thread};
  my $result=join ";", @$partial;
  if (!exists $ctx->{stack}{$result}){
    $ctx->{stack}{$result} = 1;
  } else {
    $ctx->{stack}{$result} +=1;
  }
  $ctx->{partial}=undef;
}

sub sect_err($@){
  my $ctx=shift @_;
  $ctx->{thread}=undef;
  $ctx->{stack}={};
  $ctx->{partial}=undef;
  $ctx->{state}=SECT_END;
}

my $stateTrans={
  "".SECT_END   .":".SECT_BEGIN, \&sect_begin,
  "".SECT_BEGIN .":".SECT_FIRST, \&sect_body,
  "".SECT_FIRST .":".SECT_REST, \&sect_body,
  "".SECT_REST  .":".SECT_REST, \&sect_body,
  "".SECT_REST  .":".SECT_END, \&sect_end,
};

#print Dumper($stateTrans);

sub transfer($@){
  my ($ctx, $state, @args)=@_;
  #print Dumper($ctx);
  my $prevState=$ctx->{"state"};
  my $key="".$prevState.":".$state;
  if (!exists $stateTrans->{$key}){
    sect_err($ctx);
  } else {
    $stateTrans->{$key}($ctx, @args);
    $ctx->{state}=$state;
  }
}

my $ctx={
  state=>SECT_END,
  partial=>undef,
  stack=>{},
};

while(<>){
  #print "$.:$_\n";
  my @a=recognize($_);
  transfer($ctx, @a);
}
transfer($ctx, SECT_END);
#print join "\n",map {$_." ".$ctx->{stack}{$_}} keys %{$ctx->{stack}};
print join "\n",map {$_." ".1} keys %{$ctx->{stack}};
