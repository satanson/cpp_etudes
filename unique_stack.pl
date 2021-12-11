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
    return (SECT_BEGIN, $_);
  } elsif (/^\s+@\w+(?:\.\w+)*\.(\w+\.\w+)/) {
    return (SECT_FIRST, $_);
  } elsif (/^\s+at\s+\w+(?:\.\w+)*\.(\w+\.\w+)/) {
    return (SECT_REST, $_);
  } elsif (/^\s+$/) {
    return (SECT_END);
  } else {
    return (SECT_UNKNOWN);
  }
}

sub sect_begin(\%@){
  my ($ctx, $line)=@_;
  $ctx->{partial}=[$line];
}

sub sect_body(\%@){
  my ($ctx, $line)=@_;
  push @{$ctx->{partial}}, $line;
}

sub sect_end($@){
  my ($ctx,@args)=@_;
  my $partial=$ctx->{partial};
  if (!defined($partial) || scalar(@$partial)==0){
    $ctx->{partial}=undef;
    return;
  }
  my $result=join "\0", @$partial;
  if (!exists $ctx->{stacks}{$result}){
    $ctx->{stacks}{$result} = 1;
  } else {
    $ctx->{stacks}{$result} +=1;
  }
  $ctx->{partial}=undef;
}

sub sect_err($@){
  my $ctx=shift @_;
  $ctx->{thread}=undef;
  $ctx->{stacks}={};
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
  stacks=>{},
};

while(<>){
  #print "$.:$_\n";
  my @a=recognize($_);
  transfer($ctx, @a);
}
transfer($ctx, SECT_END);
#print join "\n",map {$_." ".$ctx->{stacks}{$_}} keys %{$ctx->{stacks}};
print join "\n",map {s/\0//gr} keys %{$ctx->{stacks}};
