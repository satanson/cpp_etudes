#!/usr/bin/perl
#
# Copyright (c) 2020 Ran Panfeng.  All rights reserved.
# Author: satanson
# Email: ranpanf@gmail.com

# Usage:  show function call hierarchy of cpp project in the style of Linux utility tree.
#
# Format: 
#   ./calltree.pl <keyword|regex> <filter> <direction(called(1)|calling)> <verbose(0|1)> <depth(num)>
#   
#    - keyword for exact match, regex for fuzzy match;
#    - subtrees whose leaf nodes does not match filter are pruned, default value is '' means match all;
#    - direction: 1, in default, show functions called by other functions in callees' perspective; otherwise, show function calling relation in callers' perspective;
#    - verbose=0, no file locations output; otherwise succinctly output;
#    - depth=num, print max derivation depth.
#
# Examples: 
#
# # show all functions (set depth=1 preventing output from overwhelming).
# ./calltree.pl '\w+' '' 1 1 1
#
# # show functions calling fdatasync in a backtrace way with depth 3;
# ./calltree.pl 'fdatasync' '' 1 1 3
#
# # show functions calling sync_file_range in a backtrace way with depth 3;
# ./calltree.pl 'sync_file_range' '' 1 1 3
#
# # show functions calling fsync in a backtrace way with depth 4;
# /calltree.pl 'fsync' '' 1 1 4
#

use warnings;
use strict;
use Data::Dumper;

sub ensure_ag_installed() {
  my ($ag_path) = map {chomp;
    $_} qx(which ag 2>/dev/null);
  if (!defined($ag_path) || (!-e $ag_path)) {
    printf STDERR "ag is missing, please install ag at first, refer to https://github.com/ggreer/the_silver_searcher\n";
    exit 1;
  }
}


ensure_ag_installed;

my $NAME = "\\b\\w+\\b";
my $WS = "(?:\\s)";
my $SPACES = "(?:[\t ])";
my $TWO_COLON="(?::{2})";
my $SCOPED_NAME = "$TWO_COLON? (?:$NAME $TWO_COLON)* $NAME" =~ s/ //gr;
my $TRAILING_WS = "(?:$WS*(?://|/\\*)?.*)?\$";
sub gen_nested_pair_re($$$){
  my ($L,$R, $others)=@_;
  my $simple_case="$others $L $others $R $others";
  my $recursive_case="$others $L(?-1)*$R $others";
  my $nested="($recursive_case|$simple_case)";
  return "(?:$L $others $nested* $others $R)" =~ s/\s+//gr;
}

my $NESTED_PARENTHESES = gen_nested_pair_re("\\(", "\\)", "[^()]*");
my $NESTED_BRACES = gen_nested_pair_re("{", "}", "[^{}]*");
my $NESTED_ANGLES = gen_nested_pair_re("\\<", "\\>", "[^><]*");
my $TEMPLATE_ARGS = "\\btemplate $WS* $NESTED_ANGLES" =~ s/ //gr;
my $FUNC_MODIFIER = "\\b(?:static|virtual|inline|static $WS+ inline|inline $WS+ static)\\b" =~ s/ //gr;
my $CV_QUALIFIER= "\\b(?:const|volatile|const $WS+ volatile|volatile $WS+ const)\\b" =~ s/ //gr;
my $REF_PTR= "(?:[*&]+)";
my $FUNC_RETURN_VALUE = "(?:$CV_QUALIFIER $WS+)? $SCOPED_NAME (?:$WS* $NESTED_ANGLES)? (?: $WS* $CV_QUALIFIER)? (?: $WS*  $REF_PTR $WS* )?" =~ s/ //gr;

sub gen_initializer_list_of_ctor(){
  my $initializer = "$NAME $WS* (?:$NESTED_PARENTHESES|$NESTED_BRACES)";
  my $initializer_list = "$WS* : $WS* (?:$initializer $WS*,)* $WS* $initializer $WS*";
  return $initializer_list =~ s/ //gr;
}

my $INITIALIZER_LIST = gen_initializer_list_of_ctor();
sub gen_func_def_re() {
  my $func_def_re = "";
  #$func_def_re .= "(?:$TEMPLATE_ARGS)?(?:$WS+ $FUNC_MODIFIER $WS+)? $FUNC_RETURN_VALUE";
  #$func_def_re .= "$WS+ ($SCOPED_NAME) $WS* $NESTED_PARENTHESES (?:$INITIALIZER_LIST)?";
  $func_def_re .= "^.*?($SCOPED_NAME) $WS* $NESTED_PARENTHESES (?:$INITIALIZER_LIST)?";
  $func_def_re .= "$WS* $NESTED_BRACES";
  $func_def_re =~ s/ //g; 
  return $func_def_re;
}

sub gen_func_def_capture_name_re(){
  my $func_def_re = "";
  $func_def_re .= "($SCOPED_NAME) $WS* $NESTED_PARENTHESES (?:$INITIALIZER_LIST)?";
  $func_def_re .= "$WS* $NESTED_BRACES";
  $func_def_re =~ s/ //g; 
  return $func_def_re;
}

my $FUNC_DEF_TRAILING_RE = "^.*}$TRAILING_WS";

my $FUNC_DEF_RE = gen_func_def_re;
my $FUNC_DEF_CAPTURE_NAME_RE = gen_func_def_capture_name_re;

sub gen_func_call_re() {
  my $func_call_re = "($NAME) (?:$WS* <.*?>)? $WS* $NESTED_PARENTHESES";
  return $func_call_re =~ s/ //gr;
}

my $FUNC_CALL_RE = gen_func_call_re;


sub merge_lines(\@) {
  my @lines = @{+shift};
  my @three_parts = map {/^(\S+?):(\d+):(.*)$/; [ $1, $2, $3 ]} @lines;
  my @line_contents = map {$_->[2]} @three_parts;
  my $func_def_re = qr!$FUNC_DEF_RE!;

  #my $func_def_trailing_re = qr!$FUNC_DEF_TRAILING_RE!;
  # my %maybe_func_def_trailing = map {$_=>1} grep{$line_contents[$_] =~ $func_def_trailing_re} (0 .. $#line_contents);
  # print "trailing=", join ",", (keys %maybe_func_def_trailing), "\n";

  my ($prev_file, $prev_lineno, $prev_line) = @{$three_parts[0]};
  my $prev_lineno_adjacent = $prev_lineno;
  my @result = ();

  for (my $i = 1; $i < scalar(@three_parts); ++$i) {
    my ($file, $lineno, $line) = @{$three_parts[$i]};

    $line =~ s{//.*$}{}g;
    $line =~ s/'[{}<>()]'/'x'/g;
    $line =~ s{/\\.*$}{}g;
    $line =~ s{/[*](([^/*]*|[^/]+|[^*]+|[*][^&/]+/)+)[*]/}{}g;
    $line =~ s{"([^\\"]|\\.)*"}{""}g;

    # print "i=$i, file=$file, lineno=$lineno, line=$line\n";
    # sequential lines may be concatenated into a long line;
    #
    if (($file eq $prev_file) && ($prev_lineno_adjacent + 1 == $lineno)) {
      $prev_line = $prev_line . $line;
      $prev_lineno_adjacent += 1;
    } else {
      push @result, [$prev_file, $prev_lineno, $prev_line];
      ($prev_file, $prev_lineno, $prev_line) = ($file, $lineno, $line);
      $prev_lineno_adjacent = $prev_lineno;
    }
  }
  push @result, [$prev_file, $prev_lineno, $prev_line];
  return @result;
}

sub all_callee($$){
  my ($line, $func_call_re) = @_;
  my @calls=();
  my @names=();
  while ($line =~ /$func_call_re/g){
    if (defined($1) && defined($2)){
      #print "calling=$1, func_name=$2\n";
      push @calls, $1; 
      push @names, $2;
    }
  }
  if (scalar(@calls)==0){
    return (); 
  }
  my @nested_names = map{&all_callee($_, $func_call_re)} grep{defined($_) && length($_)>=4} map{substr($calls[$_], length($names[$_]))} (0 .. $#calls); 
  push @names, @nested_names; 
  return @names;
}

sub extract_all_funcs(\%$$) {
  my ($ignored, $trivial_threshold, $length_threshold)=@_;

  my @matches = map {
    chomp;
    $_
  } qx(ag -G '\.(c|cc|cpp|C|h|hh|hpp|H)\$' --ignore '*test*' --ignore '*benchmark*'  --ignore '*benchmark*' '$FUNC_DEF_RE');

  my @func_file_line_def = merge_lines @matches;

  my $left_angles_re = qr/(?:<\s*){2,}/;
  my @func_def = map{s/$left_angles_re/ /g; $_} map {$_->[2]}@func_file_line_def;

  @func_def = map{s/\s+<\s+/ lt /g; $_} @func_def;
  @func_def = map{s/<=/ le /g; $_} @func_def;
  
  my $func_def_re = qr!$FUNC_DEF_RE!;
  my $func_def_capture_name_re = qr!$FUNC_DEF_CAPTURE_NAME_RE!;
  my $func_call_re = qr!$FUNC_CALL_RE!;

  my @func_name = map {
    if($_=~$func_def_capture_name_re){
      $1
    }else{
      undef
    }
  } @func_def;

  my $func_call_re_enclosed_by_parentheses = qr!($FUNC_CALL_RE)!;
  my @func_callees = map {
    print "def=$_\n";
    my ($first, @rest) = all_callee($_, $func_call_re_enclosed_by_parentheses);
    [@rest]
  } @func_def;

  # remove trivial functions;
  my %func_count = ();
  foreach my $callees (@func_callees){
    foreach my $callee (@$callees) {
      $func_count{$callee}++; 
    }
  }

  my %trivials = map{$_=>1}grep{$func_count{$_} > $trivial_threshold || length($_) < $length_threshold } (keys %func_count);
  my %reserved = map{/(\b\w+\b)$/; $_=>$1}grep{$func_count{$_} <= $trivial_threshold && length($_) > $length_threshold} (keys %func_count);
  my %ignored = (%$ignored, %trivials);

  my @idx = grep {
    my $name = $func_name[$_]; 
    defined($name) && !exists $ignored{$name}
  } (0 .. $#func_name);

  @func_file_line_def = map{$func_file_line_def[$_]} @idx;
  my @func_file_line = map {$_->[0] . ":" . $_->[1]} @func_file_line_def;
  @func_name = map{$func_name[$_]} @idx;
  my @func_simple_name = map {$_=~/(\b\w+\b)$/; $1} @func_name;
  @func_callees = map {$func_callees[$_]} @idx;

  my %calling=();
  my %called=();
  for (my $i=0; $i < scalar(@func_name); ++$i) {
    my $file_info = $func_file_line[$i];
    my $caller_name = $func_name[$i];
    my $caller_simple_name = $func_simple_name[$i];
    my %callee_names = map {$_=>1} grep {exists $reserved{$_}} @{$func_callees[$i]};
    my @callee_names = keys %callee_names;
    my %callee_simple_names = map {/(\b\w+\b)$/; $1 => 1} @callee_names;
    my %callee_name2simple = map {/(\b\w+\b)$/; $_ => $1 } @callee_names;
    # remove simple name that equals to its original name
    my @callee_simple_names = grep {!exists $callee_names{$_}} keys %callee_simple_names;
    my $caller_node = {
      name=>$caller_name, 
      simple_name=>$caller_simple_name,
      file_info=>$file_info,
      callee_names=>[@callee_names],
      callee_simple_names=> [@callee_simple_names],
    };

    if (!exists $calling{$caller_name}) {
      $calling{$caller_name}=[];
    }
    push @{$calling{$caller_name}}, $caller_node;

    if ($caller_name ne $caller_simple_name) {
      if (!exists $calling{$caller_simple_name}) {
        $calling{$caller_simple_name}=[];
      }
      push @{$calling{$caller_simple_name}}, $caller_node;
    }

    my %processed_callee_names=();
    for my $callee_name (@callee_names) {
      my $callee_simple_name = $callee_name2simple{$callee_name};
      if (!exists $called{$callee_name}){
        $called{$callee_name}=[];
      }
      push @{$called{$callee_name}}, $caller_node;
      $processed_callee_names{$callee_name} = 1;
      if (($callee_name ne $callee_simple_name) && !exists $processed_callee_names{$callee_simple_name}) {
        if (!exists $called{$callee_simple_name}){
          $called{$callee_simple_name}=[];
        }
        push @{$called{$callee_simple_name}}, $caller_node;
        $processed_callee_names{$callee_simple_name} = 1;
      }
    }
  }
  return (\%calling, \%called);
}

sub read_content($){
  my $file_name = shift;
  if (-f $file_name){
    open my $handle, "<", $file_name or die "Fail to open '$file_name' for reading: $!";
    local $/;
    my $content = <$handle>;
    close $handle;
    return $content;
  }
  return undef;
}

sub write_content($@){
  my ($file_name, @lines) = @_;
  open my $handle, "+>", $file_name or die "Fail to open '$file_name' for writing: $!";
  print $handle join("\n",@lines);
  close $handle;
}

sub read_lines($){
  my $file_name = shift;
  if (-f $file_name){
    open my $handle, "<", $file_name or die "Fail to open '$file_name' for reading: $!";
    my @lines = <$handle>;
    close $handle;
    return \@lines ;
  }
  return undef;
}

sub cache_or_extract_all_funcs(\%$$) {
  my ($ignored, $trivial_threshold, $length_threshold)=@_;
  $trivial_threshold = int($trivial_threshold);
  $length_threshold = int($length_threshold);
  my $suffix = ".$trivial_threshold.$length_threshold";
  my $ignored_file = ".calltree_ignored$suffix";
  my $calling_file = ".calltree_calling$suffix";
  my $called_file = ".calltree_called$suffix";

  my $ignored_string = join ",", sort{$a cmp $b} (keys %$ignored);
  my $saved_ignored_string=read_content($ignored_file);

  if (defined($saved_ignored_string) && ($saved_ignored_string eq $ignored_string)) {
    if ((-f $calling_file) && (-f $called_file)) { 
      my ($calling, $called)=(undef, undef);
      eval(read_content($calling_file));
      eval(read_content($called_file));
      unless (defined($calling) && defined($called)) {
        die "Fail to parse '$calling_file' or '$called_file'";
      }
      qx(touch $ignored_file);
      qx(touch $calling_file);
      qx(touch $called_file);
      return ($calling, $called);
    }
  }
  my ($calling, $called) = extract_all_funcs(%$ignored, $trivial_threshold, $length_threshold);
  write_content $ignored_file, $ignored_string;
  local $Data::Dumper::Purity = 1;
  write_content($calling_file, Data::Dumper->Dump([$calling], [qw(calling)]));
  write_content($called_file, Data::Dumper->Dump([$called], [qw(called)]));
  return ($calling, $called);
}

my $func = shift || die "missing function name";
my $filter = shift;
my $backtrace = shift;
my $verbose = shift;
my $depth = shift;

$filter = ".*"  unless (defined($filter) && $filter ne "");
$backtrace = 0 unless (defined($backtrace) && $backtrace ne "0");
$verbose = undef if (defined($verbose) && $verbose == 0);
$depth = 100000 unless defined($depth);

my @ignored=(
  qw(for if while switch catch),
  qw(log  warn log trace debug defined warn  error fatal),
  qw(static_cast reinterpret_cast const_cast dynamic_cast),
  qw(constexpr),
  qw(set get),
);

my %ignored=map{$_=>1}@ignored;

my ($calling, $called) = cache_or_extract_all_funcs(%ignored, 50, 3);

sub called_tree($$$$$$) {
  my ($called, $name, $file_info, $level, $filter, $path) = @_;
  $level++;

  my $node = { file_info => $file_info, name => $name, callers => [], leaf=>undef};
  my $simple_name = ($name=~/\b(\w+)\b$/, $1);

  if (!exists $called->{$simple_name} || $level >= $depth || exists $path->{$simple_name}) {
    $level--;
    if (!exists $called->{$simple_name}) {
      $node->{leaf}="outmost";
    } elsif ($level >= $depth) {
      $node->{leaf}="deep";
    } elsif (exists $path->{$simple_name}) {
      $node->{leaf}="recursive";
    }
    $level--;
    return $name =~ /$filter/?$node:undef;
  }

  my $child = $called->{$simple_name};
  my @child_nodes=();

  $path->{$simple_name}=1;
  foreach my $chd (@$child) {
    push @child_nodes, &called_tree($called, $chd->{name}, $chd->{file_info}, $level, $filter, $path);
  }
  delete $path->{$simple_name};

  @child_nodes = grep{defined($_)} @child_nodes;
  $level--;

  if (@child_nodes){
    $node->{callers} = [@child_nodes];
    return $node;
  } else {
    return undef;
  }
}

sub fuzzy_called_tree($$$) {
  my ($called, $name_pattern, $filter) = @_;
  my $root = {file_info => "", name => $name_pattern, callers=>[], leaf=>undef};
  my @names = grep {/$name_pattern/} sort {$a cmp $b} (keys %$called);

  $root->{callers} = [ 
    grep {defined($_)} map {
      &called_tree($called, $_, "", 0, $filter, {})
    } @names
  ];
  return $root;
}

sub unified_called_tree($$$) {
  my ($called, $name, $filter) = @_;
  if (!exists $called->{$name}) {
    return &fuzzy_called_tree($called, $name, $filter);
  }
  else {
    return &fuzzy_called_tree($called, "^$name\$", $filter);
  }
}
sub get_entry_of_called_tree($$){
  my ($node, $verbose)=@_;

  my $name = $node->{name};
  my $file_info = $node->{file_info};
  $name = "\e[33;32;1m$name\e[m";
  if (defined($verbose) && defined($file_info) && length($file_info)>0) {
    $name = $name . "\t[" . $file_info . "]";
  }
  return $name;
}

sub get_child_of_called_tree($){
  my $node = shift;
  return @{$node->{callers}};
}

sub format_tree($$\&\&) {
  my ($root, $verbose, $get_entry, $get_child) = @_;
  unless (%$root) {
    return ();
  }
  my $entry = $get_entry->($root, $verbose);
  my @child = $get_child->($root);

  my @result = ($entry);

  if (scalar(@child) == 0) {
    return @result;
  }

  my $last_child = pop @child;

  foreach my $chd (@child) {
    my ($first, @rest) = &format_tree($chd, $verbose, $get_entry, $get_child);
    push @result, "├── $first";
    push @result, map {"│   $_"} @rest;
  }

  my ($first, @rest) = &format_tree($last_child, $verbose, $get_entry, $get_child);
  push @result, "└── $first";
  push @result, map {"    $_"} @rest;
  return @result;
}

sub format_called_tree($$){
  return format_tree($_[0], $_[1], &get_entry_of_called_tree, &get_child_of_called_tree);
}

my @lines = format_called_tree(unified_called_tree($called, $func, $filter), $verbose);
print join qq//, map {"$_\n"} @lines;
