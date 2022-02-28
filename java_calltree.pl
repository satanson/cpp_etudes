#!/usr/bin/perl
#
# Copyright (c) 2020 Ran Panfeng.  All rights reserved.
# Author: satanson
# Email: ranpanf@gmail.com
# Github repository: https://github.com/satanson/cpp_etudes.git

# Usage:  show function call hierarchy of java project in the style of Linux utility tree.
#
# Format: 
#   ./java_calltree.pl <keyword|regex> <filter> <direction(called(1)|calling)> <verbose(0|1)> <depth(num)>
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
# ./java_calltree.pl '\w+' '' 1 1 1
#
# # show functions calling fdatasync in a backtrace way with depth 3;
# ./java_calltree.pl 'fdatasync' '' 1 1 3
#
# # show functions calling sync_file_range in a backtrace way with depth 3;
# ./java_calltree.pl 'sync_file_range' '' 1 1 3
#
# # show functions calling fsync in a backtrace way with depth 4;
# /java_calltree.pl 'fsync' '' 1 1 4
#

use warnings;
use strict;
use Data::Dumper;
use Storable qw/freeze thaw nstore retrieve/;

sub red_color($) {
  my ($msg) = @_;
  "\e[95;31;1m$msg\e[m"
}

sub any(&;@) {
  my ($pred, @values) = @_;
  my $n = () = grep {$_} map {$pred->($_)} @values;
  $n > 0;
}

sub all(&;@) {
  my ($pred, @values) = @_;
  !(any {!$pred->($_)} @values);
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
  file_newer_than(+shift, get_path_of_script());
}

sub get_cached_or_run(&$$;@) {
  my ($func, $validate_func, $cached_file, @args) = @_;
  if (file_newer_than_script($cached_file)) {
    my $result = retrieve($cached_file);
    if (defined($result) && ref($result) eq ref([]) && $validate_func->(@$result)) {
      return @$result;
    }
  }
  my @result = $func->(@args);
  nstore [@result], $cached_file;
  return @result;
}

sub get_cache_or_run_keyed(\@$$;@) {
  my ($key, $file, $func, @args) = @_;

  die "Invalid data" unless defined($key) && ref($key) eq ref([]);
  die "Invalid func" unless defined($func);

  my @key = @$key;
  my $expect_key = join "\0", @key;

  my $check_key = sub(\%) {
    my $data = shift;
    return exists $data->{cached_key} && $data->{cached_key} eq $expect_key;
  };
  my ($data) = get_cached_or_run {
    +{ cached_key => $expect_key, cached_data => [ $func->(@args) ] }
  } $check_key, $file;
  return @{$data->{cached_data}};
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
  die "Platform '$os' is not supported, run java_calltree.pl in [$supported_os]" unless exists $supported_os{$os};

  die "Undefined cwd or home" unless defined($cwd) && defined($home);
  die "Never run java_calltree.pl in HOME directory: '$home'" if $cwd eq $home;
  die "Never run java_calltree.pl in root directory: '$cwd'" if $cwd eq '/';
  my @comp = split qr'/+', $cwd;
  die "Never run java_calltree.pl in a directory whose depth <= 2" if scalar(@comp) <= 3;
}

ensure_safe;
ensure_ag_installed;

my $ignore_pattern = join "", map {" --ignore '$_' "}
  qw(*test* *benchmark* *CMakeFiles* *contrib/* *third_party/*
    *thirdparty/* *3rd-[pP]arty/* *3rd[pP]arty/* *deps/*);

my $java_filename_pattern = qq/'\\.java\$'/;

my $RE_IDENTIFIER = "\\b[A-Za-z_]\\w*\\b";
my $RE_WS = "(?:\\s)";
my $RE_TWO_COLON = "(?::{2})";
my $RE_SCOPED_IDENTIFIER = "(?:$RE_TWO_COLON $RE_WS*)? (?: $RE_IDENTIFIER $RE_WS* $RE_TWO_COLON $RE_WS*)* [~]?$RE_IDENTIFIER" =~ s/ //gr;
sub gen_nested_pair_re($$$) {
  my ($L, $R, $others) = @_;
  my $simple_case = "$others $L $others $R $others";
  my $recursive_case = "$others $L(?-1)*$R $others";
  my $nested = "($recursive_case|$simple_case)";
  return "(?:$L $others $nested* $others $R)" =~ s/\s+//gr;
}

my $RE_NESTED_PARENTHESES = gen_nested_pair_re("\\(", "\\)", "[^()]*");
my $RE_NESTED_BRACES = gen_nested_pair_re("{", "}", "[^{}]*");

sub gen_re_list($$$) {
  my ($re_delimiter, $re_item, $optional) = @_;
  my $re = "(?: $re_item (?: $RE_WS* $re_delimiter $RE_WS* $re_item)*? ) $optional";
  return $re =~ s/ //gr;
}

sub gen_re_initializer_list_of_ctor() {
  my $re_csv = gen_re_list(",", "(?:[^,]+?)", "??");
  my $initializer = "(?: $RE_IDENTIFIER  $RE_WS*  (?: (?: \\( $RE_WS*  $re_csv  $RE_WS* \\) ) | (?: \\{ $RE_WS*  $re_csv  $RE_WS* \\} ) ) )";
  my $re_csv_initializer = gen_re_list(",", "(?: $initializer )", "");
  my $initializer_list = "(?: (?<=\\) ) $RE_WS* : $RE_WS* $re_csv_initializer $RE_WS* (?={) )";

  return $initializer_list =~ s/ //gr;

}
my $RE_INITIALIZER_LIST = gen_re_initializer_list_of_ctor();
sub gen_re_overload_operator() {
  my $operators = "[-+*/%^&|~!=<>]=?|(?:(?:<<|>>|\\|\\||&&)=?)|<=>|->\\*|->|\\(\\s*\\)|\\[\\s*\\]|\\+\\+|--|,";
  my $re = "(?:operator \\s* (?:$operators)\\s*(?=\\())";
  return $re =~ s/ //gr;
}
my $RE_OVERLOAD_OPERATOR = gen_re_overload_operator();
sub gen_re_func_def() {
  my $re_func_def = "";
  $re_func_def .= "^.*?($RE_SCOPED_IDENTIFIER|$RE_OVERLOAD_OPERATOR) $RE_WS* $RE_NESTED_PARENTHESES";
  $re_func_def .= "(?:$RE_INITIALIZER_LIST)?";
  $re_func_def .= "$RE_WS* $RE_NESTED_BRACES";
  $re_func_def =~ s/ //g;
  return $re_func_def;
}

sub gen_re_func_def_name() {
  my $re_func_def = "";
  $re_func_def .= "^.*?($RE_SCOPED_IDENTIFIER|$RE_OVERLOAD_OPERATOR) $RE_WS* $RE_NESTED_PARENTHESES";
  $re_func_def =~ s/ //g;
  return $re_func_def;
}

my $RE_FUNC_DEFINITION = gen_re_func_def;
my $RE_FUNC_DEFINITION_NAME = gen_re_func_def_name;

sub gen_re_func_call() {
  my $cs_tokens = "$RE_WS* (?:(?: $RE_SCOPED_IDENTIFIER $RE_WS* , $RE_WS*)* $RE_SCOPED_IDENTIFIER $RE_WS*)?";
  #my $re_func_call = "((?:($RE_SCOPED_IDENTIFIER) $RE_WS *(?:\\($cs_tokens\\))? $RE_WS* (?: \\. | -> | :: ) $RE_WS* )? ($RE_SCOPED_IDENTIFIER)) $RE_WS* [(]";
  my $re_func_call = "((?:($RE_SCOPED_IDENTIFIER) $RE_WS* (?: \\. | -> | :: ) $RE_WS* )? ($RE_SCOPED_IDENTIFIER)) $RE_WS* [(]";
  return $re_func_call =~ s/ //gr;
}

my $RE_FUNC_CALL = gen_re_func_call;

sub gen_re_gcc_attribute() {
  my $attr = "(?: $RE_IDENTIFIER (?: $RE_WS* \\([^()]+\\) )? )";
  my $attr_list = "(?: $attr (?: $RE_WS* , $RE_WS* $attr $RE_WS*)*)";
  my $gcc_attribute = "__attribute__ $RE_WS* \\( \\(? $RE_WS*  $attr_list $RE_WS* \\)? \\)";
  return $gcc_attribute =~ s/ //gr;
}

my $RE_GCC_ATTRIBUTE = gen_re_gcc_attribute();

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
my $RE_TEMPLATE_ARGS_1LAYER = qr'((\.?)<\s*((((\w+[\.:]+)*\w+\s*,\s*)*(\w+[\.:]+)*\w+\s*)|\?)\s*>)';
my $RE_CSV_TOKEN = gen_re_list(",", $RE_SCOPED_IDENTIFIER, "??");
my $RE_NOEXCEPT_THROW = qr'(\bthrows\b\s*(((\w+[\.:]+)*\w+\s*,\s*)*(\w+[\.:]+)*\w+\s*)\s*)';
my $RE_MACRO_DEF = qr/(#define([^\n\r]*\\(\n\r?|\r\n?))*([^\n\r]*[^\n\r\\])?((\n\r?)|(\r\n?)|$))/;

sub empty_string_with_blank_lines($) {
  q/""/ . (join "\n", map {""} split "\n", $_[0]);
}

sub blank_lines($) {
  $_[0] =~ tr/\n\r//cdr;
}

sub replace_single_char($) {
  $_[0] =~ s/$RE_SINGLE_CHAR/'x'/gr;
}

sub replace_quoted_string($) {
  return $_[0] =~ s/$RE_QUOTED_STRING/&empty_string_with_blank_lines($1)/gemr;
}

sub replace_slash_star_comment($) {
  $_[0] =~ s/$RE_SLASH_STAR_COMMENT/&blank_lines($1)/gemr;
}

sub replace_nested_char($) {
  $_[0] =~ s/$RE_NESTED_CHARS_IN_SINGLE_QUOTES/'x'/gr;
}

sub replace_single_line_comment($) {
  $_[0] =~ s/$RE_SINGLE_LINE_COMMENT//gr;
}

sub replace_left_angles($) {
  $_[0] =~ s/$RE_LEFT_ANGLES/++/gr;
}

sub replace_lt($) {
  $_[0] =~ s/\s+<\s+/ + /gr;
}

sub replace_template_args_1layer($) {
  ($_[0] =~ s/$RE_TEMPLATE_ARGS_1LAYER/&blank_lines($1)/gemr, $1);
}

my $RE_DECORATOR=qr/\@\w+(\([^()]+\))?/;
sub remove_decorator($) {
  $_[0] =~ s/$RE_DECORATOR/ /gr;
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
  $_[0] =~ s/(\b(volatile|const|final|override|public|private|protected|static|synchronized)\b)|\[\[\w+\]\]//gr;
}

sub remove_noexcept_and_throw($) {
  $_[0] =~ s/$RE_NOEXCEPT_THROW/&blank_lines($1)/gemr;
}

sub replace_template_args_4layers($) {
  repeat_apply(4, &replace_template_args_1layer, $_[0]);
}

#sub remove_gcc_attributes($) {
#  $_[0] =~ s/$RE_GCC_ATTRIBUTE//gr;
#}
#
#sub replace_macro_defs($) {
#  $_[0] =~ s/$RE_MACRO_DEF/&blank_lines($1)/gemr;
#}

sub preprocess_one_java_file($) {
  my $file = shift;
  return unless -f $file;
  my $content = read_content($file);
  return unless defined($content) && length($content) > 0;
  $content = replace_quoted_string(replace_single_char(replace_slash_star_comment($content)));

  $content = join qq/\n/, map {
    replace_lt(replace_left_angles(replace_nested_char(replace_single_line_comment($_))))
  } split qq/\n/, $content;

  $content = remove_keywords_and_attributes($content);
  #$content = remove_gcc_attributes($content);
  $content = replace_template_args_4layers($content);
  $content = remove_noexcept_and_throw($content);
  $content = remove_decorator($content);
  #$content = replace_macro_defs($content);

  my $tmp_file = "$file.tmp.created_by_call_tree";
  write_content($tmp_file, $content);
  rename $tmp_file => $file;
}

sub get_all_java_files() {
  return grep {
    defined($_) && length($_) > 0 && (-f $_)
  } map {
    chomp;
    $_
  } qx(ag -U -G $java_filename_pattern $ignore_pattern -l);
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

sub preprocess_java_files(\@) {
  my @files = grep {defined($_) && length($_) > 0 && (-f $_) && (-T $_)} @{$_[0]};
  foreach my $f (@files) {
    my $saved_f = "$f.saved_by_java_calltree";
    rename $f => $saved_f;
    write_content($f, read_content($saved_f));
    preprocess_one_java_file($f);
  }
}

sub preprocess_all_java_files() {
  my @files = get_all_java_files();

  my @groups = group_files(10, @files);
  my $num_groups = scalar(@groups);
  return if $num_groups < 1;
  my @pids = (undef) x $num_groups;
  for (my $i = 0; $i < $num_groups; ++$i) {
    my @group = @{$groups[$i]};
    my $pid = fork();
    if ($pid == 0) {
      preprocess_java_files(@group);
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
  } qx(ag -U -G '.+\\.saved_by_java_calltree\$' $ignore_pattern -l);

  foreach my $f (@saved_files) {
    my $original_f = substr($f, 0, length($f) - length(".saved_by_java_calltree"));
    rename $f => $original_f;
  }

  my @tmp_files = grep {
    defined($_) && length($_) > 0 && (-f $_)
  } map {
    chomp;
    $_
  } qx(ag -U -G '\\.tmp\\.created_by_java_calltree\$' $ignore_pattern -l);

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
        push @result, [ $prev_file, $prev_lineno, $three_parts[$i-1][1], $prev_line ];
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
    push @result, [ $prev_file, $prev_lineno, $three_parts[-1][1], $prev_line ];
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
      push @result, [ $prev_file, $prev_lineno, $three_parts[$i][1], $prev_line ];
      ($prev_file, $prev_lineno, $prev_line) = ($file, $lineno, $line);
      $prev_lineno_adjacent = $prev_lineno;
    }
  }
  push @result, [ $prev_file, $prev_lineno, $three_parts[-1][1], $prev_line ];
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

sub extract_all_callees($$) {
  my ($line, $re_func_call) = @_;
  my @callees = ();
  while ($line =~ /$re_func_call/g) {
    push @callees, { call => $1, prefix => $2, name => $3 };
  }
  return @callees;
}

sub simple_name($) {
  my $name = shift;
  $name =~ s/ //g;
  $name =~ /(~?\w+\b)$/ ? $1 : $name;
}

sub scope($) {
  $_[0] =~ /\b(\w+)\b::\s*(~?\w+\b)$/;
  $1
}

sub filename($) {
  $_[0] =~ m{/([^/]+)\.\w+:\d+};
  $1;
}

sub script_basename() {
  get_path_of_script() =~ m{/([^/]+?)(?:\.\w+)?$};
  $1
}

sub is_pure_name($) {
  all {'a' le $_ && $_ le 'z'}  split //, +shift
}

sub extract_all_funcs(\%$$) {
  my ($ignored, $trivial_threshold, $length_threshold) = @_;

  my $multiline_break = "";
  if (multiline_break_enabled()) {
    $multiline_break = "--multiline-break";
  }
  print qq(ag -U $multiline_break -G $java_filename_pattern $ignore_pattern '$RE_FUNC_DEFINITION'), "\n";
  my @matches = map {
    chomp;
    $_
  } qx(ag -U $multiline_break -G $java_filename_pattern $ignore_pattern '$RE_FUNC_DEFINITION');

  printf "extract lines: %d\n", scalar(@matches);

  die "Current directory seems not a Java project" if scalar(@matches) == 0;

  my @func_file_line_def = merge_lines @matches;

  printf "function definition after merge: %d\n", scalar(@func_file_line_def);

  my $re_func_def_name = qr!$RE_FUNC_DEFINITION_NAME!;
  my @func_def = map {$_->[3]} @func_file_line_def;
  my @func_name = map {$_ =~ $re_func_def_name;
    $1} @func_def;

  my $re_func_call = qr!$RE_FUNC_CALL!;
  printf "process callees: begin\n";
  my @func_callees = map {
    my (undef, @rest) = extract_all_callees($_, $re_func_call);
    [ @rest ]
  } @func_def;
  printf "process callees: end\n";

  # remove trivial functions;
  my %func_count = ();
  foreach my $callees (@func_callees) {
    foreach my $callee (@$callees) {
      $func_count{$callee->{name}}++;
    }
  }

  my %trivial = map {$_ => 1} grep {
    is_pure_name($_) && ($func_count{$_} > $trivial_threshold || length($_) < $length_threshold)
  } (keys %func_count);

  my %ignored = (%$ignored, %trivial);
  my %reserved = map {$_ => simple_name($_)} grep {!exists $ignored{$_}} (keys %func_count);

  my @func_file_line = map {$_->[0] . ":" . $_->[1]} @func_file_line_def;
  my @func_simple_name = map {simple_name($_)} @func_name;

  my %calling = ();
  my %called = ();
  for (my $i = 0; $i < scalar(@func_name); ++$i) {
    my $file_info = $func_file_line[$i];
    my $caller_name = $func_name[$i];
    my $caller_simple_name = $func_simple_name[$i];
    my $scope = scope($caller_name);
    my $filename = filename($file_info);
    my ($file, $start_lineno, $end_lineno) = @{$func_file_line_def[$i]}[0..2];

    my @callees = @{$func_callees[$i]};
    foreach my $seq (0 .. $#callees) {$callees[$seq]{seq} = $seq}

    my %callees = map {
      if (defined($_->{prefix})) {
        $_->{prefix} . "/" . $_->{name} => $_
      }
      else {
        $_->{name} => $_
      }
    } grep {exists $reserved{$_->{name}}} @callees;

    @callees = values %callees;
    @callees = sort {$a->{seq} <=> $b->{seq}} @callees;
    my %callee_name2simple = map {$_->{name} => simple_name($_->{name})} @callees;

    my $caller_node = {
      name         => $caller_name,
      simple_name  => $caller_simple_name,
      scope        => $scope,
      file_info    => $file_info,
      file         => $file,
      start_lineno => $start_lineno,
      end_lineno   => $end_lineno,
      filename     => $filename,
      callees      => [ @callees ],
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
    for my $callee (@callees) {
      my $callee_name = $callee->{name};
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
  my $calling_names = [ sort {$a cmp $b} keys %calling ];
  my $called_names = [ sort {$a cmp $b} keys %called ];
  return (\%calling, \%called, $calling_names, $called_names);
}

sub get_cached_or_extract_all_funcs(\%$$) {
  my ($ignored, $trivial_threshold, $length_threshold) = @_;
  restore_saved_files();
  $trivial_threshold = int($trivial_threshold);
  $length_threshold = int($length_threshold);

  my $suffix = "$trivial_threshold.$length_threshold";
  my $script_basename = script_basename();
  my $file = ".$script_basename.summary.$suffix.dat";
  my @key = ((sort {$a cmp $b} keys %$ignored), $trivial_threshold, $length_threshold);
  my $do_summary = sub() {
    print "preprocess_all_java_files\n";
    preprocess_all_java_files();
    print "register_abnormal_shutdown_hook\n";
    register_abnormal_shutdown_hook();
    print "extract_all_funcs: begin\n";
    my @result =
      extract_all_funcs(%$ignored, $trivial_threshold, $length_threshold);
    print "extract_all_funcs: end\n";
    @SIG{keys %SIG} = qw/DEFAULT/ x (keys %SIG);
    restore_saved_files();
    return @result;
  };
  my @result = get_cache_or_run_keyed(@key, $file, $do_summary);
  return @result;
}

my @ignored = (
  qw(for if while switch catch),
  qw(VLOG LOG log warn log trace debug defined warn error fatal),
  qw(static_cast reinterpret_cast const_cast dynamic_cast),
  qw(return assert sizeof alignas),
  qw(constexpr),
  qw(set get),
  qw(printf assert ASSERT),
  qw(CHECK DCHECK_LT DCHECK DCHECK_EQ DCHECK_GT DCHECK_NE),
  qw(UNLIKELY LIKELY unlikely likely)
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
  die "Invariant 'java_calltree_trivial_threshold($_) > 0' is violated" unless int($_) > 0;
} qw/java_calltree_trivial_threshold 50/;

my $env_length_threshold = get_env_or_default {
  die "Invalid 'java_calltree_length_threshold($_) > 1' is violated" unless int($_) > 1;
} qw/java_calltree_length_threshold 3/;

my %ignored = map {$_ => 1} @ignored;

my ($calling, $called, $calling_names, $called_names) = (undef, undef, undef, undef);

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

use List::Util qw/min max/;
sub lev_dist($$) {
  my ($a, $b) = @_;
  my ($a_len, $b_len) = (0, 0);
  $a_len = length($a) if defined($a);
  $b_len = length($b) if defined($b);
  return $b_len if $a_len == 0;
  return $a_len if $b_len == 0;

  my @a = split //, lc $a;
  my @b = split //, lc $b;

  my ($ii, $jj) = ($a_len + 1, $b_len + 1);
  my @d = map {[ (0) x $jj ]} 1 .. $ii;

  for (my $i = 0; $i < $ii; ++$i) {
    $d[$i][0] = $i;
  }

  for (my $j = 0; $j < $jj; ++$j) {
    $d[0][$j] = $j;
  }

  for (my $i = 1; $i < $ii; ++$i) {
    for (my $j = 1; $j < $jj; ++$j) {
      my ($ci, $cj) = ($a[$i - 1], $b[$j - 1]);
      if ($ci eq $cj) {
        $d[$i][$j] = $d[$i - 1][$j - 1];
      }
      else {
        $d[$i][$j] = 1 + min($d[$i - 1][$j], $d[$i][$j - 1], $d[$i - 1][$j - 1]);
      }
    }
  }
  return $d[-1][-1];
}

sub lev_dist_score($$) {
  my ($a, $b) = @_;
  return 0 unless defined($a) && defined($b) && length($a) > 0 && length($b) > 0;
  my $dist = lev_dist($a, $b);
  return $dist == -1 ? 0 : 1000 - $dist;
}

sub substr_score($$) {
  my ($a, $b) = @_;
  return 0 unless defined($a) && defined($b) && length($a) > 0 && length($b) > 0;
  $a = lc $a;
  $b = lc $b;
  if ((index $b, $a) != -1) {
    return 1000;
  }
  else {
    return 0;
  }
}

sub abbr_score($$) {
  my ($a, $b) = @_;
  return 0 unless defined($a) && defined($b) && length($a) > 0 && length($b) > 0;
  $a = lc $a;
  my $b0 = lc join "", grep {"A" le $_ && $_ le "Z"} split //, $b;
  my $b1 = lc join "", grep {my $chr = $_;
    all {$chr ne $_} qw/a e i o u/} split //, $b;
  my $score0 = 0;
  my $score1 = 0;
  $score0 = substr_score($a, $b0) if length($b0) > 1;
  $score1 = substr_score($a, $b1) if length($b1) > 1;
  return max($score0, $score1);
}

sub score(\%;@) {
  my ($data, @rules) = (@_, sub {0});
  max(map {$_->($data)} @rules);
}

sub default_score(\%) {
  my $data = shift;

  my %rules = (
    rule_prefix_scope_abbr        => sub($) {
      my $d = shift;
      abbr_score($d->{prefix}, $d->{scope});
    },
    rule_prefix_scope_substr      => sub($) {
      my $d = shift;
      substr_score($d->{prefix}, $d->{scope});
    },
    rule_prefix_scope_lev_dist    => sub($) {
      my $d = shift;
      lev_dist_score($d->{prefix}, $d->{scope});
    },
    rule_prefix_filename_abbr     => sub($) {
      my $d = shift;
      abbr_score($d->{prefix}, $d->{filename});
    },
    rule_prefix_filename_substr   => sub($) {
      my $d = shift;
      substr_score($d->{prefix}, $d->{filename});
    },
    rule_prefix_filename_lev_dist => sub($) {
      my $d = shift;
      lev_dist_score($d->{prefix}, $d->{filename});
    },
  );
  return score(%$data, values %rules);
}

sub called_tree($$$$$) {
  my ($called_graph, $name, $filter, $files_included, $depth) = @_;

  my $deduplicate = 0;
  if ($depth < 0) {
    $depth = abs($depth);
    $deduplicate = 1;
  }

  my $get_id_and_child = sub($$) {
    my ($called_graph, $node) = @_;
    my $name = $node->{name};
    my $simple_name = simple_name($name);
    my $file_info = $node->{file_info};
    my $unique_id = "$file_info";

    if ($file_info ne "" && !$files_included->($file_info)) {
      return (undef, undef);
    }

    my $matched = $name =~ /$filter/;
    if (!exists $called_graph->{$simple_name}) {
      return ($matched, $unique_id);
    }
    else {
      # deep copy
      my @child = grep {
        if ($deduplicate) {
          !exists $node->{simple_name} || $_->{simple_name} ne $node->{simple_name}
        }
        else {
          !exists $node->{file_info} || $_->{file_info} ne $node->{file_info}
        }
      } map {
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
  my ($called_names, $called, $name_pattern, $filter, $files_included, $depth) = @_;
  my $root = { file_info => "", name => $name_pattern, leaf => undef };
  my @names = grep {/$name_pattern/} @$called_names;

  $root->{child} = [
    grep {defined($_)} map {
      &called_tree($called, $_, $filter, $files_included, $depth);
    } @names
  ];
  return $root;
}

sub search_called_tree($$$$$$) {
  my ($called, $name_pattern, $match_lines, $filter, $files_included, $depth) = @_;
  my %nodes = map {($_, $_)} map {@$_} values %$called;
  my %match_lines = map {($_->[0], [])} @$match_lines;
  foreach (@$match_lines) {push @{$match_lines{$_->[0]}}, $_->[1]}
  my @nodes = values %nodes;
  @nodes = grep {
    my $n = $_;
    if (!exists $match_lines{$n->{file}} || !$files_included->($n->{file})) {
      undef;
    }
    else {
      my @lineno = @{$match_lines{$n->{file}}};
      any {$n->{start_lineno} <= $_ && $_ <= $n->{end_lineno}} @lineno;
    }
  } @nodes;

  my $root = { file_info => "", name => $name_pattern, leaf => undef };
  $root->{child} = [
    map {
      my $n = $_;
      my @child = map {
        my $subtree = $_;

        if (exists $subtree->{child}) {
          (@{$subtree->{child}});
        }
        else {
          ();
        }
      } &called_tree($called, $n->{name}, $filter, $files_included, $depth);

      my $node = {
        name        => $n->{name},
        simple_name => $n->{simple_name},
        file_info   => $n->{file_info},
        leaf        => "internal",
        child       => [ @child ]
      };
      $node;
    } @nodes
  ];
  return $root;
}

sub unified_called_tree($$$$) {
  my ($name, $filter, $files_included, $depth) = @_;
  if (exists $called->{$name}) {
    return &called_tree($called, $name, $filter, $files_included, $depth);
  }
  else {
    return &fuzzy_called_tree($called_names, $called, $name, $filter, $files_included, $depth);
  }
}

sub calling_tree($$$$$$) {
  my ($calling_graph, $name, $filter, $files_included, $depth, $uniques) = @_;

  my $new_variant_node = sub($) {
    my ($node) = @_;
    my $call = $node->{name};
    my $callees = [ map {+{ %$_ }} @{$node->{callees}} ];
    my $clone_node = +{
      (%$node),
      branch_type => "callees",
      call        => $call,
      callees     => $callees,
    };
    return $clone_node;
  };

  my $new_callee_or_match_node = sub($) {
    my ($callee) = @_;
    my $name = $callee->{name};
    my $call = $callee->{call};
    my $simple_name = simple_name($name);

    unless (is_pure_name($name)) {
      if (exists $calling_graph->{$name} && scalar(@{$calling_graph->{$name}}) == 1) {
        my $node = $new_variant_node->($calling->{$name}[0]);
        $node->{origin_call} = $call;
        return $node;
      }

      if (exists $calling_graph->{$simple_name} && scalar(@{$calling_graph->{$simple_name}}) == 1) {
        my $node = $new_variant_node->($calling->{$simple_name}[0]);
        $node->{origin_call} = $call;
      }
    }

    my $node = +{
      (%$callee),
      simple_name => $simple_name,
      file_info   => "",
      branch_type => 'variants',
    };
    return $node;
  };

  my $get_id_and_child = sub($$) {
    my ($calling_graph, $node) = @_;
    my $name = $node->{name};
    my $call = $node->{call};
    my $simple_name = simple_name($name);
    my $branch_type = $node->{branch_type};
    my $file_info = $node->{file_info};
    my $unique_id = "$file_info:$call";
    $uniques->{"$file_info.$name"} = 1;

    if ($file_info ne "" && !$files_included->($file_info)) {
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

      unless (defined($variant_nodes) && scalar(@$variant_nodes) > 0) {
        return ($matched, $unique_id);
      }

      my @variant_nodes = map {
        $new_variant_node->($_)
      } @$variant_nodes;

      if (exists $node->{prefix}) {
        my $prefix = $node->{prefix};
        my @variant_nodes_and_scores =
          sort {
            $b->[0] <=> $a->[0]
          } map {
            my %d = (
              prefix   => $prefix,
              scope    =>, $_->{scope},
              filename => $_->{filename},
            );
            [ default_score(%d), $_ ]
          } @variant_nodes;
        # exact match
        my @exact_variant_nodes = map {$_->[1]} grep {$_->[0] >= 999} @variant_nodes_and_scores;
        if (scalar(@exact_variant_nodes) > 0) {
          return ($matched, $unique_id, @exact_variant_nodes);
        }
        # approximate match
        my @approx_variant_nodes = map {$_->[1]} grep {$_->[0] >= 995} @variant_nodes_and_scores;
        if (scalar(@approx_variant_nodes) > 0) {
          return ($matched, $unique_id, @approx_variant_nodes);
        }
        # no match
        return ($matched, $unique_id, @variant_nodes);
      }
      else {
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

  my $node = $new_callee_or_match_node->({ name => $name, call => $name, simple_name => $name });

  return &sub_tree($calling_graph, $node, 0, $depth, {}, $get_id_and_child, $install_child);
}

sub adjust_calling_tree($) {
  my ($root) = @_;
  return undef unless defined($root);
  return $root unless exists $root->{child};
  return $root if is_pure_name($root->{name});
  my @child = map {&adjust_calling_tree($_)} @{$root->{child}};
  if (($root->{branch_type} eq "variants") && (scalar(@child) == 1)) {
    my $node = $child[0];
    $node->{origin_call} = $root->{call};
    return $node;
  }
  else {
    $root->{child} = [ @child ];
    return $root;
  }
}

sub fuzzy_calling_tree($$$$$$) {
  my ($calling_names, $calling_graph, $name_pattern, $filter, $files_included, $depth) = @_;
  my @names = grep {/$name_pattern/} @$calling_names;
  my @trees = ();
  my $uniques = {};
  for my $name (@names) {
    my $child0 = $calling_graph->{$name}[0];
    my $child0_file_info = $child0->{file_info};
    my $child0_name = $child0->{name};
    my $child0_unique_id = "$child0_file_info.$child0_name";
    next if exists $uniques->{$child0_unique_id};
    my $tree = calling_tree($calling_graph, $name, $filter, $files_included, $depth, $uniques);
    push @trees, $tree if defined($tree);
  }
  return {
    name        => $name_pattern,
    simple_name => $name_pattern,
    file_info   => "",
    branch_type => "matches",
    child       => [ @trees ],
  };
}

sub unified_calling_tree($$$$) {
  my ($name, $filter, $files_included, $depth) = @_;
  my $root = undef;
  if (exists $calling->{$name}) {
    $root = &calling_tree($calling, $name, $filter, $files_included, $depth * 2, {});
  }
  else {
    $root = &fuzzy_calling_tree($calling_names, $calling, $name, $filter, $files_included, $depth * 2);
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
  if ($file_info) {
    $file_info =~ s/:/ +/g;
    $file_info = "vim $file_info";
  }

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
  my @lines = format_tree($root, $verbose, &get_entry_of_called_tree, &get_child_of_called_tree);
  return map {"  $_"} ("", @lines, "");
}

sub get_entry_of_calling_tree($$) {
  my ($node, $verbose) = @_;

  my $name = $node->{name};
  my $branch_type = $node->{branch_type};
  my $file_info = $node->{file_info};
  my $leaf = $node->{leaf};

  if ($file_info) {
    $file_info =~ s/:/ +/g;
    $file_info = "vim $file_info";
  }

  if ($isatty) {
    if ($branch_type eq "matches") {
      $name = "\e[97;35;1m$name\e[m";
    }
    elsif ($branch_type eq "variants") {
      my $call = $node->{call};
      if ($leaf eq "internal") {
        $name = "\e[91;33;1m+ $call\e[m";
      }
      elsif ($leaf eq "outermost") {
        #$name = "\e[95;31;1m$call\e[m\e[91;38;2m\t[out-of-tree]\e[m";
        $name = "\e[95;31;1m$call\e[m";
      }
      elsif ($leaf eq "recursive") {
        $name = "\e[32;36;1m$name\t[recursive]\e[m";
      }
      else {
        $name = "\e[33;32;1m$call\e[m";
      }
    }
    elsif ($branch_type eq "callees") {
      my $before_at = "";
      if (exists $node->{origin_call} && defined($node->{origin_call})) {
        my $origin_call = $node->{origin_call};
        if ($origin_call ne $name) {
          $before_at = "\e[91;33;1m$origin_call\e[m @ ";
        }
      }
      if ($leaf eq "recursive") {
        $name = "$before_at\e[32;36;1m$name\t[recursive]\e[m";
      }
      else {
        $name = "$before_at\e[33;32;1m$name\e[m";
      }
    }
  }
  else {
    if ($branch_type eq "variants" && $leaf eq "outermost") {
      $name = "$name\t[OUT-OF-TREE]";
    }
    elsif ($branch_type eq "variants" && $leaf eq "internal") {
      $name = "$name [+]";
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

sub group_by(&;@) {
  my ($part_func, @values) = @_;
  my @pairs = map {[ $part_func->($_), $_ ]} @values;
  my %group = ();
  for my $p (@pairs) {
    my ($key, $val) = ($p->[0], $p->[1]);
    if (!exists $group{$key}) {
      $group{$key} = [];
    }
    push @{$group{$key}}, $val;
  }
  return %group;
}

sub outermost_tree($$) {
  my ($name, $files_included) = @_;
  my %names = map {
    $_ => 1
  } grep {
    !is_pure_name($_)
  } grep {
    ($_ =~ /$name/)
  } @$calling_names;

  my @names = grep {!exists $called->{$_}} sort {$a cmp $b} keys %names;
  my @trees = grep {defined($_)} map {calling_tree($calling, $_, "\\w+", $files_included, 2, {})} @names;
  @trees = map {
    ($_->{branch_type} eq "variants" ? @{$_->{child}} : $_);
  } @trees;

  @trees = map {
    $_->{child} = [];
    $_->{leaf} = "internal";
    $_
  } @trees;

  my %trees = group_by {$_->{file_info}} @trees;
  @trees = sort {
    $a->{name} cmp $b->{name}
  } map {
    $_->[0];
  } values %trees;
  return {
    name        => $name,
    simple_name => $name,
    file_info   => "",
    branch_type => "matches",
    child       => [ @trees ],
  };
}

sub innermost_tree($$) {
  my ($name, $files_included) = @_;
  my %names = map {
    $_ => 1
  } grep {
    ($_ =~ /$name/) && ($_ !~ /^~/);
  } map {
    simple_name($_)
  } @$called_names;

  my @names = grep {!exists $calling->{$_}} sort {$a cmp $b} keys %names;
  my @trees = grep {defined($_)} map {called_tree($called, $_, "\\w+", $files_included, 1)} @names;

  @trees = map {
    $_->{child} = [];
    $_
  } @trees;

  return {
    name        => $name,
    simple_name => $name,
    file_info   => "",
    child       => [ @trees ],
  };
}

sub get_child_of_calling_tree($) {
  my $node = shift;
  return exists $node->{child} ? @{$node->{child}} : ();
}
sub format_calling_tree($$) {
  my ($root, $verbose) = @_;
  my @lines = format_tree($root, $verbose, &get_entry_of_calling_tree, &get_child_of_calling_tree);
  return map {"  $_"} ("", @lines, "");
}

use Digest::SHA qw(sha256_hex);
sub cached_sha256_file(@) {
  my @data = (@_);
  my $script_basename = script_basename();
  return ".$script_basename.result.cached." . sha256_hex(@data);
}

my @key = (@ARGV, $isatty, $env_trivial_threshold, $env_length_threshold);

my $func = shift || die "missing function name";
my $filter = shift;
my $mode = shift;
my $verbose = shift;
my $depth = shift;
my $files_included = shift;

$filter = (defined($filter) && $filter ne "") ? $filter : ".*";
$mode = (defined($mode) && int($mode) >= 0) ? int($mode) : 1;
$verbose = (defined($verbose) && $verbose ne "0") ? "verbose" : undef;
$depth = defined($depth) ? int($depth) : 3;
$files_included = (defined($files_included) && $files_included ne "") ? $files_included : '.*';

my $files_included_gen = sub($) {
  my $re = shift;
  if ($re =~ /^-/) {
    $re = substr $re, 1;
    sub($) {
      my $a = shift;
      $a !~ /$re/;
    }
  }
  else {
    sub($) {
      my $a = shift;
      $a =~ /$re/;
    }
  }
};

$files_included = $files_included_gen->($files_included);

sub search_matched_lines($) {
  my $re = shift;
  grep {defined($_)} map {
    if (/^([^:]+):(\d+):.*$/) {
      [ $1, $2 ]
    }
    else {
      undef
    }
  } map {chomp;
    $_} qx(ag -U -G $java_filename_pattern  $ignore_pattern '$re');
}

sub show_tree() {
  ($calling, $called, $calling_names, $called_names) =
    get_cached_or_extract_all_funcs(%ignored, $env_trivial_threshold, $env_length_threshold);
  if ($mode == 0) {
    my $tree = unified_calling_tree($func, $filter, $files_included, $depth);
    my @lines = format_calling_tree($tree, $verbose);
    return join qq//, map {"$_\n"} @lines;
  }
  elsif ($mode == 1) {
    my $tree = unified_called_tree($func, $filter, $files_included, $depth);
    my @lines = format_called_tree($tree, $verbose);
    return join qq//, map {"$_\n"} @lines;
  }
  elsif ($mode == 2) {
    my $tree = outermost_tree($func, $files_included);
    my @lines = format_calling_tree($tree, $verbose);
    return join qq//, map {"$_\n"} @lines;
  }
  elsif ($mode == 3) {
    my $tree = innermost_tree($func, $files_included);
    my @lines = format_called_tree($tree, $verbose);
    return join qq//, map {"$_\n"} @lines;
  }
  elsif ($mode == 4) {
    my @match_lines = search_matched_lines($func);
    my $tree = search_called_tree($called, $func, \@match_lines, $filter, $files_included, $depth);
    my @lines = format_called_tree($tree, $verbose);
    return join qq//, map {"$_\n"} @lines;
  }
}

print get_cache_or_run_keyed(@key, cached_sha256_file(@key), \&show_tree);
