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

sub ensure_ag_installed() {
  my ($ag_path) = map {chomp;
    $_} qx(which ag 2>/dev/null);
  if (!defined($ag_path) || (!-e $ag_path)) {
    printf STDERR "ag is missing, please install ag at first, refer to https://github.com/ggreer/the_silver_searcher\n";
    exit 1;
  }
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

my $ignore_pattern = join "", map {" --ignore '$_' "} qw(*test* *benchmark* *CMakeFiles* *contrib/* *thirdparty/* *3rdparty/*);
my $cpp_filename_pattern = qq/'\\.(c|cc|cpp|C|h|hh|hpp|H)\$'/;

my $NAME = "\\b[A-Za-z_]\\w*\\b";
my $WS = "(?:\\s)";
my $SPACES = "(?:[\t ])";
my $TWO_COLON = "(?::{2})";
my $SCOPED_NAME = "$TWO_COLON? (?:$NAME $TWO_COLON)* [~]?$NAME" =~ s/ //gr;
my $TRAILING_WS = "(?:$WS*(?://|/\\*)?.*)?\$";
sub gen_nested_pair_re($$$) {
  my ($L, $R, $others) = @_;
  my $simple_case = "$others $L $others $R $others";
  my $recursive_case = "$others $L(?-1)*$R $others";
  my $nested = "($recursive_case|$simple_case)";
  return "(?:$L $others $nested* $others $R)" =~ s/\s+//gr;
}

my $NESTED_PARENTHESES = gen_nested_pair_re("\\(", "\\)", "[^()]*");
my $NESTED_BRACES = gen_nested_pair_re("{", "}", "[^{}]*");
my $NESTED_ANGLES = gen_nested_pair_re("\\<", "\\>", "[^><]*");
my $TEMPLATE_ARGS = "\\btemplate $WS* $NESTED_ANGLES" =~ s/ //gr;
my $FUNC_MODIFIER = "\\b(?:static|virtual|inline|static $WS+ inline|inline $WS+ static)\\b" =~ s/ //gr;
my $CV_QUALIFIER = "\\b(?:const|volatile|const $WS+ volatile|volatile $WS+ const)\\b" =~ s/ //gr;
my $REF_PTR = "(?:[*&]+)";
my $FUNC_RETURN_VALUE = "(?:$CV_QUALIFIER $WS+)? $SCOPED_NAME (?:$WS* $NESTED_ANGLES)? (?: $WS* $CV_QUALIFIER)? (?: $WS*  $REF_PTR $WS* )?" =~ s/ //gr;

sub gen_initializer_list_of_ctor() {
  my $initializer = "$NAME $WS* $NESTED_PARENTHESES";
  my $initializer_list = "$WS* : (?:$WS* $initializer $WS*,$WS*)* $WS* $initializer $WS*";
  return $initializer_list =~ s/ //gr;
}

my $INITIALIZER_LIST = gen_initializer_list_of_ctor();

sub gen_func_def_re() {
  my $func_def_re = "";
  $func_def_re .= "^.*?($SCOPED_NAME) $WS* $NESTED_PARENTHESES";
  $func_def_re .= "$WS* $NESTED_BRACES";
  $func_def_re =~ s/ //g;
  return $func_def_re;
}

my $FUNC_DEF_RE = gen_func_def_re;

sub gen_func_call_re() {
  my $func_call_re = "($NAME) $WS* [(]";
  return $func_call_re =~ s/ //gr;
}

my $FUNC_CALL_RE = gen_func_call_re;

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
  } qx(ag -G $cpp_filename_pattern $ignore_pattern -l);
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
  } qx(ag -G '.+\\.saved_by_calltree\$' $ignore_pattern -l);

  foreach my $f (@saved_files) {
    my $original_f = substr($f, 0, length($f) - length(".saved_by_calltree"));
    rename $f => $original_f;
  }

  my @tmp_files = grep {
    defined($_) && length($_) > 0 && (-f $_)
  } map {
    chomp;
    $_
  } qx(ag -G '\\.tmp\\.created_by_calltree\$' $ignore_pattern -l);

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
  my @sigs = qw/__DIE__ QUIT INT TERM ABRT/;
  @SIG{@sigs} = ($abnormal_handler) x scalar(@sigs);
}

sub merge_lines(\@) {
  my @lines = @{+shift};
  my @three_parts = map {/^([^:]+):(\d+):(.*)$/;
    [ $1, $2, $3 ]} @lines;
  my @line_contents = map {$_->[2]} @three_parts;

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

sub all_callee($$) {
  my ($line, $func_call_re) = @_;
  my @calls = ();
  my @names = ();
  # print "\n\n\nline=$line\n";
  while ($line =~ /$func_call_re/g) {
    if (defined($1) && defined($2)) {
      # print "calling=$1, func_name=$2\n";
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

  print qq(ag -G $cpp_filename_pattern $ignore_pattern '$FUNC_DEF_RE'), "\n";
  my @matches = map {
    chomp;
    $_
  } qx(ag -G $cpp_filename_pattern $ignore_pattern '$FUNC_DEF_RE');

  printf "extract lines: %d\n", scalar(@matches);

  die "Current directory seems not a C/C++ project" if scalar(@matches) == 0;

  my @func_file_line_def = merge_lines @matches;

  printf "function definition after merge: %d\n", scalar(@func_file_line_def);

  my $func_def_re = qr!$FUNC_DEF_RE!;
  my $func_call_re = qr!$FUNC_CALL_RE!;
  my @func_def = map {$_->[2]} @func_file_line_def;
  my @func_name = map {$_ =~ $func_def_re;
    $1} @func_def;

  my $func_call_re_enclosed_by_parentheses = qr!($FUNC_CALL_RE)!;
  printf "process callees: begin\n";
  my @func_callees = map {
    my ($first, @rest) = all_callee($_, $func_call_re_enclosed_by_parentheses);
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

  my %trivials = map {$_ => 1} grep {$func_count{$_} > $trivial_threshold || length($_) < $length_threshold} (keys %func_count);
  my %ignored = (%$ignored, %trivials);
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


sub any(\&;@) {
  my ($pred, @values) = @_;
  my $n = () = grep {$_} map {$pred->($_)} @values;
  return $n > 0;
}

sub all(\&;@) {
  my ($pred, @values) = @_;
  return !&any(sub {!$pred->($_)}, @values);
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

    if (&all(sub {-f $_}, (values %cached_files))) {
      my ($calling, $called, $calling_names, $called_names) = (undef, undef, undef, undef);
      foreach my $cached_file (values %cached_files) {
        eval(read_content($cached_file));
      }
      my @cached_vars = ($calling, $called, $calling_names, $called_names);
      if (&any(sub {!defined($_)}, @cached_vars)) {
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

my %ignored = map {$_ => 1} @ignored;

my ($calling, $called, $calling_names, $called_names) = cache_or_extract_all_funcs(%ignored, 50, 3);


sub sub_tree($$$$$$$) {
  my ($graph, $node, $level, $depth, $path, $get_id_and_child, $install_child) = @_;

  my ($matched, $node_id, @child) = $get_id_and_child->($graph, $node);
  return undef unless defined($node_id);

  if (scalar(@child) == 0 || ($level + 1) >= $depth || exists $path->{$node_id}) {
    if (scalar(@child) == 0) {
      $node->{leaf} = "outmost";
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
    my ($called, $node) = @_;
    my $name = $node->{name};
    my $simple_name = simple_name($name);
    my $file_info = $node->{file_info};
    my $unique_id = "$file_info:$name";

    if ($file_info ne "" && $file_info =~ /$files_excluded/) {
      return (undef, undef);
    }

    my $matched = $simple_name =~ /$filter/;
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
    my ($graph, $node) = @_;
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
      if (!exists $calling_graph->{$simple_name}) {
        return ($matched, $unique_id);
      }
      else {
        my @variant_nodes = map {
          $new_variant_node->($_)
        } @{$calling_graph->{$simple_name}};
        return ($matched, $unique_id, @variant_nodes);
      }
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

sub get_entry_of_called_tree($$) {
  my ($node, $verbose) = @_;

  my $name = $node->{name};
  my $file_info = $node->{file_info};
  $name = "\e[33;32;1m$name\e[m";
  if (defined($verbose) && defined($file_info) && length($file_info) > 0) {
    $name = $name . "\t[" . $file_info . "]";
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

  if ($branch_type eq "matches") {
    $name = "\e[97;35;1m$name\e[m";
  }
  elsif ($branch_type eq "variants") {
    $name = "\e[91;33;1m+$name\e[m";
  }
  elsif ($branch_type eq "callees") {
    $name = "\e[33;32;1m$name\e[m";
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

if ($backtrace) {
  my $tree = unified_called_tree($func, $filter, $files_excluded, $depth);
  my @lines = format_called_tree($tree, $verbose);
  print join qq//, map {"$_\n"} @lines;
}
else {
  my $tree = unified_calling_tree($func, $filter, $files_excluded, $depth);
  my @lines = format_calling_tree($tree, $verbose);
  print join qq//, map {"$_\n"} @lines;
}
