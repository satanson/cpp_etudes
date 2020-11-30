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

sub red_color($) {
  my ($msg) = @_;
  return "\e[95;31;1m$msg\e[m";
}

use Cwd qw/abs_path/;
sub get_path_of_script() {
  if ($0 !~ qr'/') {
    my ($path) = map {chomp;
      $_} qx(which $0 2>/dev/null);
    return $path;
  }
  else {
    return abs_path($0);
  }
}

sub file_newer_than($$) {
  my ($file_a, $file_b) = @_;
  return 0 unless ((-f $file_a) && (-f $file_b));
  return (-M $file_a) < (-M $file_b);
}

sub file_newer_than_script($) {
  return file_newer_than(+shift, get_path_of_script());
}

sub ensure_ag_installed() {
  my ($ag_path) = map {chomp;
    $_} qx(which ag 2>/dev/null);
  if (!defined($ag_path) || (!-e $ag_path)) {
    printf STDERR "ag is missing, please install ag at first, refer to https://github.com/ggreer/the_silver_searcher; 
    the fork https://github.com/satanson/the_silver_searcher provides a --multiline-break options will generate exact
    result because the modified ag insert break(--) between two function definitions that are not separated by blank 
    lines.";
    exit 1;
  }
}

sub multiline_break_enabled() {
  my ($enabled) = map {chomp;
    $_} qx(echo enabled|ag --multiline-break enabled 2>/dev/null);
  return defined($enabled) && $enabled eq "enabled";
}

sub ensure_safe() {
  my ($cwd, $home) = @ENV{qw/PWD HOME/};
  my @supported_os = qw/Linux Darwin/;
  my %supported_os = map {$_ => 1} @supported_os;

  my ($os) = map {
    if (defined($_)) {
      chomp;
      $_
    }
    else {"UNKNOWN"}
  } qx'uname -s 2>/dev/null';

  my $supported_os = join "|", @supported_os;
  die "Platform '$os' is not supported, run calltree.pl in [$supported_os]" unless exists $supported_os{$os};

  return unless defined($cwd) && defined($home);
  die "Never run calltree.pl in HOME directory: '$home'" if $cwd eq $home;
  die "Never run calltree.pl in root directory: '$cwd'" if $cwd eq '/';
  my @comp = split qr'/+', $cwd;
  die "Never run calltree.pl in a directory whose depth <= 2" if scalar(@comp) <= 3;
}

ensure_safe;
ensure_ag_installed;

my $ignore_pattern = join "", map {" --ignore '$_' "} qw(*test* *benchmark* *CMakeFiles* *contrib/* *thirdparty/* *3rd-[pP]arty/* *3rd[pP]arty/*);
my $cpp_filename_pattern = qq/'\\.(c|cc|cpp|C|h|hh|hpp|H)\$'/;

my $RE_IDENTIFIER = "\\b[A-Za-z_]\\w*\\b";
my $RE_WS = "(?:\\s)";
my $RE_TWO_COLON = "(?::{2})";
my $RE_SCOPED_IDENTIFIER = "$RE_TWO_COLON? (?:$RE_IDENTIFIER $RE_TWO_COLON)* [~]?$RE_IDENTIFIER" =~ s/ //gr;
sub gen_nested_pair_re($$$) {
  my ($L, $R, $others) = @_;
  my $simple_case = "$others $L $others $R $others";
  my $recursive_case = "$others $L(?-1)*$R $others";
  my $nested = "($recursive_case|$simple_case)";
  return "(?:$L $others $nested* $others $R)" =~ s/\s+//gr;
}

my $RE_NESTED_PARENTHESES = gen_nested_pair_re("\\(", "\\)", "[^()]*");
my $RE_NESTED_BRACES = gen_nested_pair_re("{", "}", "[^{}]*");

sub gen_initializer_list_of_ctor() {
  my $initializer = "$RE_IDENTIFIER $RE_WS* $RE_NESTED_PARENTHESES";
  my $initializer_list = "$RE_WS* : (?:$RE_WS* $initializer $RE_WS*,$RE_WS*)* $RE_WS* $initializer $RE_WS*";
  return $initializer_list =~ s/ //gr;
}

sub gen_func_def_re() {
  my $func_def_re = "";
  $func_def_re .= "^.*?($RE_SCOPED_IDENTIFIER) $RE_WS* $RE_NESTED_PARENTHESES";
  $func_def_re .= "$RE_WS* $RE_NESTED_BRACES";
  $func_def_re =~ s/ //g;
  return $func_def_re;
}

my $RE_FUNC_DEFINITION = gen_func_def_re;

sub gen_func_call_re() {
  my $func_call_re = "($RE_SCOPED_IDENTIFIER) $RE_WS* [(]";
  return $func_call_re =~ s/ //gr;
}

my $RE_FUNC_CALL = gen_func_call_re;

sub read_content($) {
  my $file_name = shift;
  if (-f $file_name) {
    open my $handle, "<", $file_name or die "Fail to open '$file_name' for reading: $!";
    local $/;
    my $content = <$handle>;
    close $handle;
    return $content;
  }
  return undef;
}

sub write_content($@) {
  my ($file_name, @lines) = @_;
  open my $handle, "+>", $file_name or die "Fail to open '$file_name' for writing: $!";
  print $handle join("\n", @lines);
  close $handle;
}

sub read_lines($) {
  my $file_name = shift;
  if (-f $file_name) {
    open my $handle, "<", $file_name or die "Fail to open '$file_name' for reading: $!";
    my @lines = <$handle>;
    close $handle;
    return \@lines;
  }
  return undef;
}
## preprocess source file

my $RE_QUOTED_STRING = qr/("([^"]*\\")*[^"]*")/;
my $RE_SINGLE_CHAR = qr/'[\\]?.'/;
my $RE_SLASH_STAR_COMMENT = qr"(/[*]([^/*]*(([*]+|[/]+)[^/*]+)*([*]+|[/]+)?)[*]/)";
my $RE_NESTED_CHARS_IN_SINGLE_QUOTES = qr/'[{}<>()]'/;
my $RE_SINGLE_LINE_COMMENT = qr'/[/\\].*';
my $RE_LEFT_ANGLES = qr'<[<=]+';
my $RE_TEMPLATE_ARGS_1LAYER = qr'(<\s*(((::)?(\w+::)*\w+\s*,\s*)*(::)?(\w+::)*\w+\s*)>)';

sub empty_string_with_blank_lines($) {
  return q/""/ . (join "\n", map {""} split "\n", $_[0]);
}

sub blank_lines($) {
  return join "\n", map {""} split "\n", $_[0];
}

sub replace_single_char($) {
  return $_[0] =~ s/$RE_SINGLE_CHAR/'x'/gr;
}

sub replace_quoted_string($) {
  return $_[0] =~ s/$RE_QUOTED_STRING/&empty_string_with_blank_lines($1)/gemr;
}

sub replace_slash_star_comment($) {
  return $_[0] =~ s/$RE_SLASH_STAR_COMMENT/&blank_lines($1)/gemr;
}

sub replace_nested_char($) {
  return $_[0] =~ s/$RE_NESTED_CHARS_IN_SINGLE_QUOTES/'x'/gr;
}

sub replace_single_line_comment($) {
  return $_[0] =~ s/$RE_SINGLE_LINE_COMMENT//gr;
}

sub replace_left_angles($) {
  return $_[0] =~ s/$RE_LEFT_ANGLES/++/gr;
}

sub replace_lt($) {
  return $_[0] =~ s/\s+<\s+/ + /gr;
}

sub replace_template_args_1layer($) {
  return ($_[0] =~ s/$RE_TEMPLATE_ARGS_1LAYER/&blank_lines($1)/gemr, $1);
}

sub repeat_apply($\&$) {
  my ($times, $fun, $arg) = @_;
  my ($result, $continue) = $fun->($arg);
  if (defined($continue) && $times > 1) {
    return &repeat_apply($times - 1, $fun, $result);
  }
  return $result;
}

sub remove_keywords_and_attributes($) {
  return $_[0] =~ s/(\b(const|final|override|noexcept)\b)|\[\[\w+\]\]//gr;
}

sub replace_template_args_4layers($) {
  return repeat_apply(4, &replace_template_args_1layer, $_[0]);
}

sub preprocess_one_cpp_file($) {
  my $file = shift;
  return unless -f $file;
  my $content = read_content($file);
  return unless defined($content) && length($content) > 0;
  $content = replace_quoted_string(replace_single_char(replace_slash_star_comment($content)));

  $content = join qq/\n/, map {
    replace_lt(replace_left_angles(replace_nested_char(replace_single_line_comment($_))))
  } split qq/\n/, $content;

  $content = remove_keywords_and_attributes($content);
  $content = replace_template_args_4layers($content);

  my $tmp_file = "$file.tmp.created_by_call_tree";
  write_content($tmp_file, $content);
  rename $tmp_file => $file;
}

sub get_all_cpp_files() {
  return grep {
    defined($_) && length($_) > 0 && (-f $_)
  } map {
    chomp;
    $_
  } qx(ag -U -G $cpp_filename_pattern $ignore_pattern -l);
}

sub group_files($@) {
  my ($num_groups, @files) = @_;
  my $num_files = scalar(@files);
  die "Illegal num_groups($num_groups)" if $num_groups < 1;
  if ($num_files == 0) {
    return;
  }
  $num_groups = $num_files if $num_files < $num_groups;
  my @groups = map {[]} (1 .. $num_groups);
  foreach my $i (0 .. ($num_files - 1)) {
    push @{$groups[$i % $num_groups]}, $files[$i];
  }
  return @groups;
}

sub preprocess_cpp_files(\@) {
  my @files = grep {defined($_) && length($_) > 0 && (-f $_) && (-T $_)} @{$_[0]};
  foreach my $f (@files) {
    my $saved_f = "$f.saved_by_calltree";
    rename $f => $saved_f;
    write_content($f, read_content($saved_f));
    preprocess_one_cpp_file($f);
  }
}

sub preprocess_all_cpp_files() {
  my @files = get_all_cpp_files();

  my @groups = group_files(10, @files);
  my $num_groups = scalar(@groups);
  return if $num_groups < 1;
  my @pids = (undef) x $num_groups;
  for (my $i = 0; $i < $num_groups; ++$i) {
    my @group = @{$groups[$i]};
    my $pid = fork();
    if ($pid == 0) {
      preprocess_cpp_files(@group);
      exit 0;
    }
    elsif ($pid > 0) {
      $pids[$i] = $pid;
    }
    else {
      die "Fail to fork a process: $!";
    }
  }

  for (my $i = 0; $i < $num_groups; ++$i) {
    wait;
  }
}

sub restore_saved_files() {
  my @saved_files = grep {
    defined($_) && length($_) > 0 && (-f $_)
  } map {
    chomp;
    $_
  } qx(ag -U -G '.+\\.saved_by_calltree\$' $ignore_pattern -l);

  foreach my $f (@saved_files) {
    my $original_f = substr($f, 0, length($f) - length(".saved_by_calltree"));
    rename $f => $original_f;
  }

  my @tmp_files = grep {
    defined($_) && length($_) > 0 && (-f $_)
  } map {
    chomp;
    $_
  } qx(ag -U -G '\\.tmp\\.created_by_calltree\$' $ignore_pattern -l);

  foreach my $f (@tmp_files) {
    unlink $f;
  }
}


sub register_abnormal_shutdown_hook() {
  my $abnormal_handler = sub {
    my $cause = shift;
    $cause = red_color($cause);
    print qq/Abnormal exit caused by $cause\n/;
    @SIG{keys %SIG} = qw/DEFAULT/ x (keys %SIG);
    restore_saved_files();
    exit 0;
  };
  my @sig = qw/__DIE__ QUIT INT TERM ABRT/;
  @SIG{@sig} = ($abnormal_handler) x scalar(@sig);
}

sub merge_lines_multiline_break_enabled(\@) {
  my @lines = @{+shift};
  my @three_parts = map {
    if (/^([^:]+):(\d+):(.*)$/) {
      [ $1, $2, $3 ]
    }
    else {
      undef
    }
  } @lines;

  my ($prev_file, $prev_lineno, $prev_line) = (undef, undef, undef);
  my @result = ();
  for (my $i = 0; $i < scalar(@three_parts); ++$i) {
    if (!defined($three_parts[$i])) {
      if (defined($prev_file)) {
        # $i-1 is last line of a match block
        push @result, [ $prev_file, $prev_lineno, $prev_line ];
        ($prev_file, $prev_lineno, $prev_line) = (undef, undef, undef);
      }
      else {
        # deplicate multiline-break.
      }
    }
    else {
      if (defined($prev_file)) {
        # non-first lines of a match block
        $prev_line = $prev_line . $three_parts[$i][2];
      }
      else {
        # first line of a match block
        ($prev_file, $prev_lineno, $prev_line) = @{$three_parts[$i]};
      }
    }
  }

  if (defined($prev_file)) {
    push @result, [ $prev_file, $prev_lineno, $prev_line ];
  }
  return @result;
}

sub merge_lines_multiline_break_disabled(\@) {
  my @lines = @{+shift};
  my @three_parts = map {/^([^:]+):(\d+):(.*)$/;
    [ $1, $2, $3 ]} @lines;

  my ($prev_file, $prev_lineno, $prev_line) = @{$three_parts[0]};
  my $prev_lineno_adjacent = $prev_lineno;
  my @result = ();

  for (my $i = 1; $i < scalar(@three_parts); ++$i) {
    my ($file, $lineno, $line) = @{$three_parts[$i]};

    if (($file eq $prev_file) && ($prev_lineno_adjacent + 1 == $lineno)) {
      $prev_line = $prev_line . $line;
      $prev_lineno_adjacent += 1;
    }
    else {
      push @result, [ $prev_file, $prev_lineno, $prev_line ];
      ($prev_file, $prev_lineno, $prev_line) = ($file, $lineno, $line);
      $prev_lineno_adjacent = $prev_lineno;
    }
  }
  push @result, [ $prev_file, $prev_lineno, $prev_line ];
  return @result;
}

sub merge_lines(\@) {
  my @lines = @{+shift};
  if (multiline_break_enabled()) {
    print "multiline-break is enabled in ag\n";
    return merge_lines_multiline_break_enabled(@lines);
  }
  else {
    return merge_lines_multiline_break_disabled(@lines);
  }
}

sub all_callee($$) {
  my ($line, $func_call_re) = @_;
  my @calls = ();
  my @names = ();
  while ($line =~ /$func_call_re/g) {
    if (defined($1) && defined($2)) {
      push @calls, $1;
      push @names, $2;
    }
  }
  if (scalar(@calls) == 0) {
    return ();
  }
  my @nested_names = map {&all_callee($_, $func_call_re)} grep {defined($_) && length($_) >= 4} map {substr($calls[$_], length($names[$_]))} (0 .. $#calls);
  push @names, @nested_names;
  return @names;
}

sub simple_name($) {
  $_[0] =~ /(~?\w+\b)$/;
  $1
}

sub extract_all_funcs(\%$$) {
  my ($ignored, $trivial_threshold, $length_threshold) = @_;

  my $multiline_break = "";
  if (multiline_break_enabled()) {
    $multiline_break = "--multiline-break";
  }
  print qq(ag -U $multiline_break -G $cpp_filename_pattern $ignore_pattern '$RE_FUNC_DEFINITION'), "\n";
  my @matches = map {
    chomp;
    $_
  } qx(ag -U $multiline_break -G $cpp_filename_pattern $ignore_pattern '$RE_FUNC_DEFINITION');

  printf "extract lines: %d\n", scalar(@matches);

  die "Current directory seems not a C/C++ project" if scalar(@matches) == 0;

  my @func_file_line_def = merge_lines @matches;

  printf "function definition after merge: %d\n", scalar(@func_file_line_def);

  my $func_def_re = qr!$RE_FUNC_DEFINITION!;
  my @func_def = map {$_->[2]} @func_file_line_def;
  my @func_name = map {$_ =~ $func_def_re;
    $1} @func_def;

  my $func_call_re_enclosed_by_parentheses = qr!($RE_FUNC_CALL)!;
  printf "process callees: begin\n";
  my @func_callees = map {
    my (undef, @rest) = all_callee($_, $func_call_re_enclosed_by_parentheses);
    [ @rest ]
  } @func_def;
  printf "process callees: end\n";

  # remove trivial functions;
  my %func_count = ();
  foreach my $callees (@func_callees) {
    foreach my $callee (@$callees) {
      $func_count{$callee}++;
    }
  }

  my %trivial = map {$_ => 1} grep {$func_count{$_} > $trivial_threshold || length($_) < $length_threshold} (keys %func_count);
  my %ignored = (%$ignored, %trivial);
  my %reserved = map {$_ => simple_name($_)} grep {!exists $ignored{$_}} (keys %func_count);

  my @idx = grep {
    my $name = $func_name[$_];
    defined($name) && !exists $ignored{$name}
  } (0 .. $#func_name);

  @func_file_line_def = map {$func_file_line_def[$_]} @idx;
  my @func_file_line = map {$_->[0] . ":" . $_->[1]} @func_file_line_def;
  @func_name = map {$func_name[$_]} @idx;
  my @func_simple_name = map {simple_name($_)} @func_name;
  @func_callees = map {$func_callees[$_]} @idx;

  my %calling = ();
  my %called = ();
  for (my $i = 0; $i < scalar(@func_name); ++$i) {
    my $file_info = $func_file_line[$i];
    my $caller_name = $func_name[$i];
    my $caller_simple_name = $func_simple_name[$i];

    my %callee_names = map {$_ => 1} grep {exists $reserved{$_}} @{$func_callees[$i]};
    my @callee_names = keys %callee_names;
    my %callee_name2simple = map {$_ => simple_name($_)} @callee_names;

    my %callee_simple_names = map {$_ => 1} grep {exists $reserved{$_}} (values %callee_name2simple);
    my @callee_simple_names = sort {$a cmp $b} keys %callee_simple_names;

    my $caller_node = {
      name         => $caller_name,
      simple_name  => $caller_simple_name,
      file_info    => $file_info,
      callee_names => [ @callee_simple_names ],
    };

    if (!exists $calling{$caller_name}) {
      $calling{$caller_name} = [];
    }
    push @{$calling{$caller_name}}, $caller_node;

    if ($caller_name ne $caller_simple_name) {
      if (!exists $calling{$caller_simple_name}) {
        $calling{$caller_simple_name} = [];
      }
      push @{$calling{$caller_simple_name}}, $caller_node;
    }

    my %processed_callee_names = ();
    for my $callee_name (@callee_names) {
      my $callee_simple_name = $callee_name2simple{$callee_name};
      if (!exists $called{$callee_name}) {
        $called{$callee_name} = [];
      }
      push @{$called{$callee_name}}, $caller_node;
      $processed_callee_names{$callee_name} = 1;
      if (($callee_name ne $callee_simple_name) && !exists $processed_callee_names{$callee_simple_name}) {
        if (!exists $called{$callee_simple_name}) {
          $called{$callee_simple_name} = [];
        }
        push @{$called{$callee_simple_name}}, $caller_node;
        $processed_callee_names{$callee_simple_name} = 1;
      }
    }
  }
  return (\%calling, \%called);
}


sub any(&;@) {
  my ($pred, @values) = @_;
  my $n = () = grep {$_} map {$pred->($_)} @values;
  return $n > 0;
}

sub all(&;@) {
  my ($pred, @values) = @_;
  return !(any {!$pred->($_)} @values);
}

sub cache_or_extract_all_funcs(\%$$) {
  my ($ignored, $trivial_threshold, $length_threshold) = @_;

  restore_saved_files();

  $trivial_threshold = int($trivial_threshold);
  $length_threshold = int($length_threshold);
  my $suffix = "$trivial_threshold.$length_threshold";

  my $ignored_file = ".calltree.ignored.$suffix";
  my %cached_files = map {$_ => ".calltree.$_.$suffix"} qw/calling called calling_names called_names/;

  my $ignored_string = join ",", sort {$a cmp $b} (keys %$ignored);
  my $saved_ignored_string = read_content($ignored_file);

  if (defined($saved_ignored_string) && ($saved_ignored_string eq $ignored_string)) {

    if (all {-f $_ && file_newer_than_script($_)} (values %cached_files)) {
      my ($calling, $called, $calling_names, $called_names) = (undef, undef, undef, undef);
      foreach my $cached_file (values %cached_files) {
        eval(read_content($cached_file));
      }
      my @cached_vars = ($calling, $called, $calling_names, $called_names);
      if (any {!defined($_)} @cached_vars) {
        my $args = join " or ", map {'$_'} keys %cached_files;
        die "Fail to parse $args";
      }
      return @cached_vars;
    }
  }

  print "preprocess_all_cpp_files\n";
  preprocess_all_cpp_files();
  print "register_abnormal_shutdown_hook\n";
  register_abnormal_shutdown_hook();
  print "extract_all_funcs: begin\n";
  my ($calling, $called) = extract_all_funcs(%$ignored, $trivial_threshold, $length_threshold);
  print "extract_all_funcs: end\n";
  @SIG{keys %SIG} = qw/DEFAULT/ x (keys %SIG);
  restore_saved_files();

  my $calling_names = [ sort {$a cmp $b} keys %$calling ];
  my $called_names = [ sort {$a cmp $b} keys %$called ];

  my @keyed_cached_vars = (
    [ calling => $calling ],
    [ called => $called ],
    [ calling_names => $calling_names ],
    [ called_names => $called_names ],
  );

  write_content $ignored_file, $ignored_string;

  local $Data::Dumper::Purity = 1;
  foreach my $e (@keyed_cached_vars) {
    my ($key, $cached_var) = @$e;
    write_content($cached_files{$key}, Data::Dumper->Dump([ $cached_var ], [ $key ]));
  }
  return map {$_->[1]} @keyed_cached_vars;
}


my @ignored = (
  qw(for if while switch catch),
  qw(log warn log trace debug defined warn error fatal),
  qw(static_cast reinterpret_cast const_cast dynamic_cast),
  qw(return assert sizeof alignas),
  qw(constexpr),
  qw(set get),
);

sub get_env_or_default(&$$) {
  my ($chk, $key, $default) = @_;
  if (exists $ENV{$key}) {
    my $value = $ENV{$key};
    map {$chk->($_)} ($value);
    return $value;
  }
  else {
    return $default;
  }
}

my $env_trivial_threshold = get_env_or_default {
  die "Invariant 'calltree_trivial_threshold($_) > 0' is violated" unless int($_) > 0;
} qw/calltree_trivial_threshold 50/;

my $env_length_threshold = get_env_or_default {
  die "Invalid 'calltree_length_threshold($_) > 1' is violated" unless int($_) > 1;
} qw/calltree_length_threshold 3/;

my %ignored = map {$_ => 1} @ignored;
my ($calling, $called, $calling_names, $called_names)
  = cache_or_extract_all_funcs(%ignored, $env_trivial_threshold, $env_length_threshold);

sub sub_tree($$$$$$$) {
  my ($graph, $node, $level, $depth, $path, $get_id_and_child, $install_child) = @_;

  my ($matched, $node_id, @child) = $get_id_and_child->($graph, $node);
  return undef unless defined($node_id);
  $node->{leaf} = "internal";
  if (scalar(@child) == 0 || ($level + 1) >= $depth || exists $path->{$node_id}) {
    if (scalar(@child) == 0) {
      $node->{leaf} = "outermost";
    }
    elsif ($level >= $depth) {
      $node->{leaf} = "deep";
    }
    elsif (exists $path->{$node_id}) {
      $node->{leaf} = "recursive";
    }
    else {
      # never reach here;
    }
    return $matched ? $node : undef;
  }

  $level++;

  # add node_id to path;
  $path->{$node_id} = 1;

  my @child_nodes = ();
  foreach my $chd (@child) {
    push @child_nodes, &sub_tree($graph, $chd, $level, $depth, $path, $get_id_and_child, $install_child);
  }

  # delete node_id from path;
  delete $path->{$node_id};

  @child_nodes = grep {defined($_)} @child_nodes;
  $level--;

  if (@child_nodes) {
    $install_child->($node, [ @child_nodes ]);
    return $node;
  }
  else {
    $install_child->($node, []);
    return $matched ? $node : undef;
  }
}

sub called_tree($$$$) {
  my ($called_graph, $name, $filter, $files_excluded, $depth) = @_;
  my $get_id_and_child = sub($$) {
    my ($called_graph, $node) = @_;
    my $name = $node->{name};
    my $simple_name = simple_name($name);
    my $file_info = $node->{file_info};
    my $unique_id = "$file_info:$name";

    if ($file_info ne "" && $file_info =~ /$files_excluded/) {
      return (undef, undef);
    }

    my $matched = $name =~ /$filter/;
    if (!exists $called_graph->{$simple_name}) {
      return ($matched, $unique_id);
    }
    else {
      # deep copy
      my @child = map {
        my $child = {
          name        => $_->{name},
          simple_name => $_->{simple_name},
          file_info   => $_->{file_info},
        };
        $child;
      } @{$called_graph->{$simple_name}};
      return ($matched, $unique_id, @child);
    }
  };
  my $install_child = sub($$) {
    my ($node, $child) = @_;
    $node->{child} = $child;
  };

  my $node = {
    name        => $name,
    simple_name => $name,
    file_info   => "",
  };

  return &sub_tree($called_graph, $node, 0, $depth, {}, $get_id_and_child, $install_child);
}

sub fuzzy_called_tree($$$$$$) {
  my ($called_names, $called, $name_pattern, $filter, $files_excluded, $depth) = @_;
  my $root = { file_info => "", name => $name_pattern, leaf => undef };
  my @names = grep {/$name_pattern/} @$called_names;

  $root->{child} = [
    grep {defined($_)} map {
      &called_tree($called, $_, $filter, $files_excluded, $depth);
    } @names
  ];
  return $root;
}

sub unified_called_tree($$$$) {
  my ($name, $filter, $files_excluded, $depth) = @_;
  if (exists $called->{$name}) {
    return &called_tree($called, $name, $filter, $files_excluded, $depth);
  }
  else {
    return &fuzzy_called_tree($called_names, $called, $name, $filter, $files_excluded, $depth);
  }
}

sub calling_tree($$$$$) {
  my ($calling_graph, $name, $filter, $files_excluded, $depth) = @_;

  my $new_variant_node = sub($) {
    my ($node) = @_;
    my %clone_node = map {$_ => $node->{$_}} qw/name simple_name file_info/;
    $clone_node{branch_type} = 'callees';
    $clone_node{callees} = [ @{$node->{callee_names}} ];
    \%clone_node;
  };

  my $new_callee_or_match_node = sub($) {
    my ($name) = @_;
    my $simple_name = simple_name($name);

    if (exists $calling_graph->{$simple_name} && scalar(@{$calling_graph->{$simple_name}}) == 1) {
      return $new_variant_node->($calling->{$simple_name}[0]);
    }

    my $node = {
      name        => $name,
      simple_name => $simple_name,
      file_info   => "",
      branch_type => 'variants',
    };

    return $node;
  };

  my $get_id_and_child = sub($$) {
    my ($calling_graph, $node) = @_;
    my $name = $node->{name};
    my $simple_name = simple_name($name);
    my $branch_type = $node->{branch_type};
    my $file_info = $node->{file_info};
    my $unique_id = "$file_info:$name";

    if ($file_info ne "" && $file_info =~ /$files_excluded/) {
      return (undef, undef);
    }

    my $matched = $simple_name =~ /$filter/;
    if ($branch_type eq "variants") {
      my $variant_nodes = undef;
      if (exists $calling_graph->{$name}) {
        $variant_nodes = $calling_graph->{$name};
      }
      elsif (exists $calling_graph->{$simple_name}) {
        $variant_nodes = $calling_graph->{$simple_name};
      }

      unless (defined($variant_nodes)) {
        return ($matched, $unique_id);
      }

      my @variant_nodes = map {
        $new_variant_node->($_)
      } @$variant_nodes;
      return ($matched, $unique_id, @variant_nodes);
    }
    else {
      my @callee_nodes = map {
        $new_callee_or_match_node->($_)
      } @{$node->{callees}};
      return ($matched, $unique_id, @callee_nodes);
    }
  };

  my $install_child = sub($$) {
    my ($node, $child) = @_;
    $node->{child} = $child;
  };

  my $node = $new_callee_or_match_node->($name);

  return &sub_tree($calling_graph, $node, 0, $depth, {}, $get_id_and_child, $install_child);
}

sub adjust_calling_tree($) {
  my ($root) = @_;
  return undef unless defined($root);
  return $root unless exists $root->{child};
  my @child = map {&adjust_calling_tree($_)} @{$root->{child}};
  if (($root->{branch_type} eq "variants") && (scalar(@child) == 1)) {
    return $child[0];
  }
  else {
    $root->{child} = [ @child ];
    return $root;
  }
}

sub fuzzy_calling_tree($$$$$$) {
  my ($calling_names, $calling_graph, $name_pattern, $filter, $files_excluded, $depth) = @_;
  my @names = grep {/$name_pattern/} @$calling_names;
  my @trees = grep {defined($_)} map {&calling_tree($calling_graph, $_, $filter, $files_excluded, $depth)} @names;
  return {
    name        => $name_pattern,
    simple_name => $name_pattern,
    file_info   => "",
    branch_type => "matches",
    child       => [ @trees ],
  };
}

sub unified_calling_tree($$$$) {
  my ($name, $filter, $files_excluded, $depth) = @_;
  my $root = undef;
  if (exists $calling->{$name}) {
    $root = &calling_tree($calling, $name, $filter, $files_excluded, $depth * 2);
  }
  else {
    $root = &fuzzy_calling_tree($calling_names, $calling, $name, $filter, $files_excluded, $depth * 2);
  }
  return &adjust_calling_tree($root);
}

sub format_tree($$\&\&) {
  my ($root, $verbose, $get_entry, $get_child) = @_;
  unless (defined($root) && %$root) {
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

my $isatty = -t STDOUT;

sub get_entry_of_called_tree($$) {
  my ($node, $verbose) = @_;

  my $name = $node->{name};
  my $file_info = $node->{file_info};
  $name = $isatty ? "\e[33;32;1m$name\e[m" : $name;
  if (defined($verbose) && defined($file_info) && length($file_info) > 0) {
    $name = "$name\t[$file_info]";
  }
  return $name;
}

sub get_child_of_called_tree($) {
  my $node = shift;
  return exists $node->{child} ? @{$node->{child}} : ();
}

sub format_called_tree($$) {
  my ($root, $verbose) = @_;
  return format_tree($root, $verbose, &get_entry_of_called_tree, &get_child_of_called_tree);
}

sub get_entry_of_calling_tree($$) {
  my ($node, $verbose) = @_;

  my $name = $node->{name};
  my $branch_type = $node->{branch_type};
  my $file_info = $node->{file_info};
  my $leaf = $node->{leaf};

  if ($isatty) {
    if ($branch_type eq "matches") {
      $name = "\e[97;35;1m$name\e[m";
    }
    elsif ($branch_type eq "variants") {
      if ($leaf eq "internal") {
        $name = "\e[91;33;1m+$name\e[m";
      }
      elsif ($leaf eq "outermost") {
        $name = "\e[95;31;1m$name\e[m\e[91;38;2m\t[out-of-tree]\e[m";
      }
      else {
        $name = "\e[33;32;1m$name\e[m";
      }
    }
    elsif ($branch_type eq "callees") {
      if ($leaf eq "recursive") {
        $name = "\e[32;36;1m$name\t[recursive]\e[m";
      }
      else {
        $name = "\e[33;32;1m$name\e[m";
      }
    }
  }
  else {
    if ($branch_type eq "variants" && $leaf eq "outermost") {
      $name = "$name\t[OUT-OF-TREE]";
    }
    elsif ($branch_type eq "variants" && $leaf eq "internal") {
      $name = "+$name";
    }
    elsif ($branch_type eq "callees" && $leaf eq "recursive") {
      $name = "$name\t[RECURSIVE]";
    }
  }

  if (defined($verbose) && defined($file_info) && length($file_info) > 0) {
    $name = $name . "\t[" . $file_info . "]";
  }
  return $name;
}

sub get_child_of_calling_tree($) {
  my $node = shift;
  return exists $node->{child} ? @{$node->{child}} : ();
}
sub format_calling_tree($$) {
  my ($root, $verbose) = @_;
  return format_tree($root, $verbose, &get_entry_of_calling_tree, &get_child_of_calling_tree);
}

use Digest::SHA qw(sha256_hex);
sub cached_sha256_file(@) {
  my @data = (@_);
  return ".calltree.result.cached." . sha256_hex(@data);
}

sub cache_or_run(\@\&;@) {
  my ($key, $func, @args) = @_;
  die "Invalid data" unless defined($key) && ref($key) eq ref([]);
  die "Invalid func" unless defined($func);
  my @key = @$key;
  my $file = cached_sha256_file(@key);
  my $expect_key = join "\0", @key;

  if (file_newer_than_script($file)) {
    my ($cached_key, $cached_data) = (undef, undef);
    my $content = read_content($file);
    eval($content) if defined($content);
    if (defined($cached_key) && defined($cached_data) && $expect_key eq $cached_key) {
      return $cached_data;
    }
  }

  my $data = $func->(@args);
  write_content($file, Data::Dumper->Dump([ $expect_key, $data ], [ qw/cached_key cached_data/ ]));
  return $data;
}

my @key = (@ARGV, $isatty, $env_trivial_threshold, $env_length_threshold);

my $func = shift || die "missing function name";
my $filter = shift;
my $backtrace = shift;
my $verbose = shift;
my $depth = shift;
my $files_excluded = shift;

$filter = (defined($filter) && $filter ne "") ? $filter : ".*";
$backtrace = (defined($backtrace) && $backtrace eq "0") ? undef : "backtrace";
$verbose = (defined($verbose) && $verbose ne "0") ? "verbose" : undef;
$depth = (defined($depth) && int($depth) > 0) ? int($depth) : 3;
$files_excluded = (defined($files_excluded) && $files_excluded ne "") ? $files_excluded : '^$';

sub show_tree() {
  if (defined $backtrace) {
    my $tree = unified_called_tree($func, $filter, $files_excluded, $depth);
    my @lines = format_called_tree($tree, $verbose);
    return join qq//, map {"$_\n"} @lines;
  }
  else {
    my $tree = unified_calling_tree($func, $filter, $files_excluded, $depth);
    my @lines = format_calling_tree($tree, $verbose);
    return join qq//, map {"$_\n"} @lines;
  }
}

print cache_or_run(@key, &show_tree);
