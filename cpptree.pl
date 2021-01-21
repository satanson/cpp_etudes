#!/usr/bin/perl
#
# Copyright (c) 2020 Ran Panfeng.  All rights reserved.
# Author: satanson
# Email: ranpanf@gmail.com
# Github repository: https://github.com/satanson/cpp_etudes.git

# Usage:  show class hierarchy of cpp project in the style of Linux utility tree.
#
# Format: 
#   ./cpptree.pl <keyword|regex> <filter> <verbose(0|1)> <depth(num)>
#   
#   - keyword for exact match, regex for fuzzy match;
#    - subtrees whose leaf nodes does not match filter are pruned, default value is '' means match all;
#    - verbose=0, no file locations output; otherwise succinctly output;
#    - depth=num, print max derivation depth.
#
# Examples: 
#
# # show all classes
# ./cpptree.pl '\w+'
#
# # show all classes with file locations.
# ./cpptree.pl '\w+' '' 1
#
# # show all classes exact-match ExecNode if ExecNode class exists
# ./cpptree.pl 'ExecNode' '' 1
#
# # show all classes fuzzy-match regex '.*Node$' if the literal class name not exists.
# ./cpptree.pl '.*Node$' '' 1
#
# # show all classes and depth of derivation relationship is less than 3
# ./cpptree.pl '\w+' '' 1 3
#
# # show all classes whose ancestor class matches 'Node' and itself or its offsprings matches 'Scan'
# /cpptree.pl 'Node' 'Scan'
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

my $ignore_pattern = join "", map {" --ignore '$_' "} qw(*test* *benchmark* *CMakeFiles* *contrib/* *thirdparty/* *3rd-[pP]arty/* *3rd[pP]arty/*);
my $cpp_filename_pattern = qq/'\\.(c|cc|cpp|C|h|hh|hpp|H)\$'/;

sub multiline_break_enabled() {
  my ($enabled) = map {chomp;
    $_} qx(echo enabled|ag --multiline-break enabled 2>/dev/null);
  return defined($enabled) && $enabled eq "enabled";
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

sub file_newer_than_script($) {
  my ($file) = @_;
  my $script_path = get_path_of_script();
  return 0 unless ((-f $file) && (-f $script_path));
  return (-M $file) < (-M $script_path);
}

sub all_sub_classes() {
  my $attr_re = "\\[\\[[^\\[\\]]+\\]\\]";
  my $access_specifier_re = "final|private|public|protected";
  my $template_arguments_re = "<([^<>]*(?:<(?1)>|[^<>])[^<>]*)?>";
  my $cls_re = "^\\s*(template\\s*$template_arguments_re)?(?:\\s*typedef)?\\s*\\b(class|struct)\\b\\s*([a-zA-Z_]\\w*)\\s*[^{};*()=]*?{";
  my $cls_filter_re = "^(\\S+)\\s*:\\s*(?:class|struct)\\s+\\w+(\\s+:\\s+(\\s*[:\\w]+\\s*,\\s*)*[:\\w]+)?s*";

  my $class0_re = "(?:class|struct)\\s+(\\w+)";
  my $class1_re = "(?:class|struct)\\s+(\\w+)\\s*:\\s*([:\\w]+)";
  my $class2_re = "(?:class|struct)\\s+(\\w+)\\s*:(?:\\s*[:\\w]+\\s*,){1}\\s*([:\\w]+)";
  my $class3_re = "(?:class|struct)\\s+(\\w+)\\s*:(?:\\s*[:\\w]+\\s*,){2}\\s*([:\\w]+)";
  my $class4_re = "(?:class|struct)\\s+(\\w+)\\s*:(?:\\s*[:\\w]+\\s*,){3}\\s*([:\\w]+)";
  my $class5_re = "(?:class|struct)\\s+(\\w+)\\s*:(?:\\s*[:\\w]+\\s*,){4}\\s*([:\\w]+)";
  my $class6_re = "(?:class|struct)\\s+(\\w+)\\s*:(?:\\s*[:\\w]+\\s*,){5}\\s*([:\\w]+)";

  my $cache_file = ".cpptree.list";
  my @matches = ();

  my $multiline_break = "";
  if (multiline_break_enabled()) {
    $multiline_break = "--multiline-break";
  }

  if ((-f $cache_file) && file_newer_than_script($cache_file)) {
    @matches = map {chomp;
      $_} qx(cat $cache_file);
    qx(touch $cache_file);
  }
  else {

    print qq(ag $multiline_break -U -G $cpp_filename_pattern $ignore_pattern '$cls_re'), "\n";
    @matches = map {
      chomp;
      $_
    } qx(ag $multiline_break -U -G $cpp_filename_pattern $ignore_pattern '$cls_re');

    die "Current directory seems not a C/C++ project" if scalar(@matches) == 0;
    printf "Extract lines: %s\n", scalar(@matches);

    @matches = map {join ":", @$_} merge_lines @matches;
    @matches = map {s/\btypedef\b//gr} @matches;
    printf "Merged into lines: %s\n", scalar(@matches);

    my @file_info_and_line = grep {
      defined($_)
    } map {
      if (/^(\S+)\s*:\s*(.*)/) {[ $1, $2 ]}
      else {undef}
    } @matches;
    my @file_info = map {$_->[0]} @file_info_and_line;
    my @line = map {$_->[1]} @file_info_and_line;

    my @transform_re = ($attr_re, $access_specifier_re, ($template_arguments_re) x 4, "template", "\\s*{\$", "(?<=\\s)\\s+");
    for my $re (@transform_re) {
      @line = map {s/$re//g;
        $_} @line;
    }
    printf "Transformed lines:%s\n", scalar(@line);

    @matches = map {$file_info[$_] . ":" . $line[$_]} (0 .. $#line);

    @matches = grep {/$cls_filter_re/} @matches;

    printf "Matches after filtering: %s\n", scalar(@matches);
    open my $cache_file_handle, "+>", $cache_file or die "$!";
    print $cache_file_handle join("\n", @matches);
    close($cache_file_handle);
  }

  #print Dumper(\@matches);

  my @table = grep {defined($_)} map {if (/^(\S+)\s*:\s*$class0_re/) {[ $1, $2 ]}
  else {
    undef
  }} @matches;

  my %table = ();

  for my $e (@table) {
    my ($file_info, $name) = @$e;
    if (!exists $table{$name}) {
      $table{$name} = [];
    }
    push @{$table{$name}}, $file_info;
  }

  #print Dumper(\%table);

  my $tree = {};
  my @class_re_list = ($class1_re, $class2_re, $class3_re, $class4_re, $class5_re, $class6_re);

  @matches = map {$_->[1]} sort {$a->[0] cmp $b->[0]} map {/^(\S+)\s*:\s*$class0_re/;
    [ $2, $_ ]} @matches;
  for my $re (@class_re_list) {
    my @parent_matches = grep {defined($_)} map {if (/^(\S+)\s*:\s*$re\s*$/) {[ $1, $2, $3 ]}
    else {undef}} @matches;
    if (scalar(@parent_matches) == 0) {
      last;
    }
    for my $e (@parent_matches) {
      my ($fileline, $child, $parent) = @$e;
      if (!defined($tree->{$parent})) {
        $tree->{$parent} = [];
      }

      if ($child ne $parent) {
        push @{$tree->{$parent}}, $e;
      }
    }
  }
  return $tree, \%table;
}

my $cls = shift || die "missing class name";
my $filter = shift;
my $verbose = shift;
my $depth = shift;

$filter = ".*" unless (defined($filter) && $filter ne "");
$verbose = undef if (defined($verbose) && $verbose == 0);
$depth = 100000 unless defined($depth);

my ($tree, $table) = all_sub_classes();


sub remove_loop($$$) {
  my ($tree, $name, $path) = @_;
  if (!exists $tree->{$name}) {
    return;
  }
  my @child = @{$tree->{$name}};
  @child = grep {!exists $path->{$_->[1]}} @child;
  $tree->{$name} = [ @child ];
  foreach my $chd (@child) {
    my $chd_name = $chd->[1];
    $path->{$chd_name} = 1;
    #my $csv_path = join ", ", keys %$path;
    #print "name=$name, chd_name=$chd_name, path=[$csv_path]\n";
    &remove_loop($tree, $chd_name, $path);
    delete $path->{$chd_name};
  }
}

sub remove_all_loops($) {
  my $tree = shift;
  foreach my $name (keys %$tree) {
    &remove_loop($tree, $name, { $name => 1 });
  }
  return $tree;
}

use List::Util qw/max/;
my $level = 0;
sub sub_class($$;$) {
  my ($file_info, $cls, $filter) = @_;
  $filter = ".*" unless defined($filter);
  $level++;
  #print "level=$level, file_info=$file_info, cls=$cls\n";
  my $root = { file_info => $file_info, name => $cls, child => [], tall => 1 };
  if (!exists $tree->{$cls} || $level >= $depth) {
    $level--;
    return $cls =~ /$filter/ ? $root : undef;
  }

  my $child = $tree->{$cls};
  my @child_nodes = ();

  foreach my $chd (@$child) {
    push @child_nodes, &sub_class($chd->[0], $chd->[1], $filter);
  }

  @child_nodes = grep {defined($_)} @child_nodes;
  $level--;

  if (@child_nodes) {
    $root->{child} = [ @child_nodes ];
    $root->{tall} = 1 + max(map{$_->{tall}} @child_nodes);
    return $root;
  }
  else {
    return undef;
  }
}

sub fuzzy_sub_class($;$) {
  my ($cls_pattern, $filter) = @_;
  $filter = ".*" unless defined($filter);
  my $root = { file_info => undef, name => $cls_pattern };
  my @names = map {my $name = $_;
    map {[ $_, $name ]} @{$table->{$name}}} grep {/$cls_pattern/} (keys %$table);
  #print Dumper(\@names);
  my @child = grep {defined($_)} map {&sub_class(@$_, $filter)} @names;
  my $tallest = max(map{$_->{tall}} @child); 
  @child = grep {$_->{tall} == $tallest } @child;
  $root->{child} = [@child];
  return $root;
}

sub unified_sub_class($;$) {
  my ($cls, $filter) = @_;
  $filter = ".*" unless defined($filter);
  if (!exists $tree->{$cls}) {
    return &fuzzy_sub_class($cls, $filter);
  }
  else {
    return &fuzzy_sub_class("^$cls\$", $filter);
  }
}

$tree = remove_all_loops $tree;
#print Dumper(all_sub_classes);
my $hierarchy = unified_sub_class $cls, $filter;
#print Dumper($hierarchy);
#
sub format_tree($;$) {
  my ($root, $verbose) = @_;
  unless (%$root) {
    return ();
  }
  my $file_info = $root->{file_info};
  my $name = $root->{name};
  my @child = @{$root->{child}};

  my @result = ();
  $name = "\e[33;32;1m$name\e[m";
  if (defined($verbose) && defined($file_info)) {
    my $full_name = $name . "\t[" . $file_info . "]";
    push @result, $full_name;
  }
  else {
    push @result, $name;
  }

  if (scalar(@child) == 0) {
    return @result;
  }

  my $last_child = pop @child;
  foreach my $chd (@child) {
    my ($first, @rest) = &format_tree($chd, $verbose);
    push @result, "├── $first";
    push @result, map {"│   $_"} @rest;
  }
  my ($first, @rest) = &format_tree($last_child, $verbose);
  push @result, "└── $first";
  push @result, map {"    $_"} @rest;
  return @result;
}

my @lines = format_tree $hierarchy, $verbose;
@lines = map {"  $_"} ("", @lines, "");
print join qq//, map {"$_\n"} @lines;
