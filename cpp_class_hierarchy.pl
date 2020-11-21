#!/usr/bin/perl
#
# Copyright (c) 2020 Ran Panfeng.  All rights reserved.
# Author: satanson
# Email: ranpanf@gmail.com

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

sub merge_lines(\@) {
  my @lines = @{+shift};
  my @three_parts = map {/^(\S+):(\d+):(.*)$/;
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
      push @result, join(":", $prev_file, $prev_lineno, $prev_line);
      ($prev_file, $prev_lineno, $prev_line) = ($file, $lineno, $line);
      $prev_lineno_adjacent = $prev_lineno;
    }
  }
  push @result, join(":", $prev_file, $prev_lineno, $prev_line);
  return @result;
}

sub all_sub_classes() {
  my $attr_re = "\\[\\[[^\\[\\]]+\\]\\]";
  my $access_specifier_re = "final|private|public|protected";
  my $template_arguments_re = "<([^<>]*(?:<(?1)>|[^<>])[^<>]*)?>";
  my $cls_re = "^\\s*(template\\s*$template_arguments_re)?\\s*\\b(class|struct)\\b\\s*([a-zA-Z]\\w+[a-zA-Z])\\s*[^{};*()=]*?{";
  my $cls_filter_re = "^(\\S+)\\s*:\\s*(?:class|struct)\\s+\\w+(\\s+:\\s+(\\s*[:\\w]+\\s*,\\s*)*[:\\w]+)?s*\$";

  my $class0_re = "(?:class|struct)\\s+(\\w+)";
  my $class1_re = "(?:class|struct)\\s+(\\w+)\\s*:\\s*([:\\w]+)";
  my $class2_re = "(?:class|struct)\\s+(\\w+)\\s*:(?:\\s*[:\\w]+\\s*,){1}\\s*([:\\w]+)";
  my $class3_re = "(?:class|struct)\\s+(\\w+)\\s*:(?:\\s*[:\\w]+\\s*,){2}\\s*([:\\w]+)";
  my $class4_re = "(?:class|struct)\\s+(\\w+)\\s*:(?:\\s*[:\\w]+\\s*,){3}\\s*([:\\w]+)";
  my $class5_re = "(?:class|struct)\\s+(\\w+)\\s*:(?:\\s*[:\\w]+\\s*,){4}\\s*([:\\w]+)";
  my $class6_re = "(?:class|struct)\\s+(\\w+)\\s*:(?:\\s*[:\\w]+\\s*,){5}\\s*([:\\w]+)";

  my $cache_file = ".cpp_class_hierarchy.list";
  my @matches = ();
  if ((-f $cache_file) && (((-M $cache_file) * 3600 * 24) < 600)) {
    @matches = map {chomp;
      $_} qx(cat $cache_file);
    qx(touch $cache_file);
  }
  else {
    @matches = map {chomp;
      $_} qx(ag -G '\.(c|cc|cpp|C|h|hh|hpp|H)\$' --ignore '*test*' --ignore '*benchmark*'  --ignore '*benchmark*' '$cls_re');
    @matches = merge_lines @matches;

    my @file_info_and_line = grep {defined($_)} map {if (/^(\S+)\s*:\s*(.*)/) {[ $1, $2 ]}
    else {undef}} @matches;
    my @file_info = map {$_->[0]} @file_info_and_line;
    my @line = map {$_->[1]} @file_info_and_line;

    my @transform_re = ($attr_re, $access_specifier_re, ($template_arguments_re) x 4, "template", "\\s*{\$", "(?<=\\s)\\s+");
    for my $re (@transform_re) {
      @line = map {s/$re//g;
        $_} @line;
    }

    @matches = map {$file_info[$_] . ":" . $line[$_]} (0 .. $#line);

    @matches = grep {/$cls_filter_re/} @matches;
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
my $verbose = shift;
my $depth = shift;
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

my $level = 0;
sub sub_class($$) {
  my ($file_info, $cls) = @_;
  $level++;
  #print "level=$level, file_info=$file_info, cls=$cls\n";
  my $root = { file_info => $file_info, name => $cls, child => [] };
  if (!exists $tree->{$cls} || $level >= $depth) {
    $level--;
    return $root;
  }

  my $child = $tree->{$cls};

  foreach my $chd (@$child) {
    push @{$root->{child}}, &sub_class($chd->[0], $chd->[1]);
  }
  $level--;
  return $root;
}

sub fuzzy_sub_class($) {
  my ($cls_pattern) = @_;
  my $root = { file_info => undef, name => $cls_pattern };
  my @names = map {my $name = $_;
    map {[ $_, $name ]} @{$table->{$name}}} grep {/$cls_pattern/} (keys %$table);
  #print Dumper(\@names);
  $root->{child} = [ map {&sub_class(@$_)} @names ];
  return $root;
}

sub unified_sub_class($) {
  my ($cls) = @_;
  if (!exists $tree->{$cls}) {
    return &fuzzy_sub_class($cls);
  }
  else {
    return &fuzzy_sub_class("^$cls\$");
  }
}

$tree = remove_all_loops $tree;
#print Dumper(all_sub_classes);
my $hierarchy = unified_sub_class $cls;
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
print join qq//, map {"$_\n"} @lines;
