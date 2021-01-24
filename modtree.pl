#!/usr/bin/perl
#
# Copyright (c) 2020 Ran Panfeng.  All rights reserved.
# Author: satanson
# Email: ranpanf@gmail.com
# Github repository: https://github.com/satanson/cpp_etudes.git

# Usage:  show function call hierarchy of cpp project in the style of Linux utility tree.
#
# Format: 
#   ./calltree.pl <keyword|regex> <filter> <direction(referred(1)|calling)> <verbose(0|1)> <depth(num)>
#   
#    - keyword for exact match, regex for fuzzy match;
#    - subtrees whose leaf nodes does not match filter are pruned, default value is '' means match all;
#    - direction: 1, in default, show functions referred by other functions in callees' perspective; otherwise, show function calling relation in callers' perspective;
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
  nstore [ @result ], $cached_file;
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
  die "Platform '$os' is not supported, run calltree.pl in [$supported_os]" unless exists $supported_os{$os};

  die "Undefined cwd or home" unless defined($cwd) && defined($home);
  die "Never run calltree.pl in HOME directory: '$home'" if $cwd eq $home;
  die "Never run calltree.pl in root directory: '$cwd'" if $cwd eq '/';
  my @comp = split qr'/+', $cwd;
  die "Never run calltree.pl in a directory whose depth <= 2" if scalar(@comp) <= 3;
}

ensure_safe;
ensure_ag_installed;

my $ignore_pattern = join "", map {" --ignore '$_' "}
  qw(*test* *benchmark* *CMakeFiles* *contrib/* *third_party/*
    *thirdparty/* *3rd-[pP]arty/* *3rd[pP]arty/* *deps/*);

my $cpp_filename_pattern = qq/'\\.(c|cc|cpp|C|h|hh|hpp|H)\$'/;

my $RE_WS = "(?:\\s)";
my $RE_WS_NON_NL = "(?:[\\t\\x{20}])";
my $DBL_QUOTE = "\\x{22}";


sub gen_mod_import() {
  my $re_mod_def = "";
  $re_mod_def .= "$RE_WS_NON_NL* # $RE_WS_NON_NL* include $RE_WS_NON_NL*";
  $re_mod_def .= "[$DBL_QUOTE<] $RE_WS_NON_NL* (\\S+) $RE_WS_NON_NL* [$DBL_QUOTE>] $RE_WS_NON_NL*\$";
  return $re_mod_def =~ s/ //gr;
}

my $RE_MOD_IMPORT = gen_mod_import;
my $RE_MOD_IMPORT_AG = "^$RE_MOD_IMPORT";
my $RE_COLON_SEPARATED_TRIPLE = '^([^:]+):(\d+):(.*)$';

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

sub no_path_sep($) {
  +shift !~ m{/}
}

sub extract_all_mods(\%$$) {
  my ($ignored, $trivial_threshold, $length_threshold) = @_;

  my $multiline_break = "";
  if (multiline_break_enabled()) {
    $multiline_break = "--multiline-break";
  }
  print qq(ag -U $multiline_break -G $cpp_filename_pattern $ignore_pattern '$RE_MOD_IMPORT_AG'), "\n";
  my @matches = map {
    chomp;
    $_
  } qx(ag -U $multiline_break -G $cpp_filename_pattern $ignore_pattern '$RE_MOD_IMPORT_AG');

  printf "extract lines: %d\n", scalar(@matches);

  die "Current directory seems not a C/C++ project" if scalar(@matches) == 0;

  my @mod_import_decls = map {
    $_ =~ qr!$RE_COLON_SEPARATED_TRIPLE!;
    my $referer = $1;
    my $lineno = $2;
    my $referee = ($3 =~ qr!$RE_MOD_IMPORT!, $1);
    [ $referer, $lineno, $referee ]
  } grep {$_ =~ qr!$RE_MOD_IMPORT!} @matches;

  printf "function definition after merge: %d\n", scalar(@mod_import_decls);

  my @mod_referee = map {$_->[2]} @mod_import_decls;

  # remove trivial functions;
  my %mod_count = ();
  foreach my $referee (@mod_referee) {
    $mod_count{$referee}++;
  }

  my %trivial = map {$_ => 1} grep {
    no_path_sep($_) && ($mod_count{$_} > $trivial_threshold || length($_) < $length_threshold)
  } (keys %mod_count);

  my %ignored = (%$ignored, %trivial);

  my %referring = ();
  my %referred = ();
  for (my $i = 0; $i < scalar(@mod_import_decls); ++$i) {
    my $referer = $mod_import_decls[$i]->[0];
    my $lineno = $mod_import_decls[$i]->[1];
    my $referee = $mod_import_decls[$i]->[2];
    next if exists $ignored{$referee};

    my $referee_node = {
      referer => $referer,
      referee => $referee,
      lineno  => $lineno,
      id      => $i,
    };

    if (!exists $referring{$referer}) {
      $referring{$referer} = [];
    }
    push @{$referring{$referer}}, $referee_node;

    if (!exists $referred{$referee}) {
      $referred{$referee} = [];
    }
    push @{$referred{$referee}}, $referee_node;
  }
  my $referer_list = [ sort {$a cmp $b} keys %referring ];
  my $referee_list = [ sort {$a cmp $b} keys %referred ];
  return (\%referring, \%referred, $referer_list, $referee_list);
}

sub get_cached_or_extract_all_mods(\%$$) {
  my ($ignored, $trivial_threshold, $length_threshold) = @_;
  $trivial_threshold = int($trivial_threshold);
  $length_threshold = int($length_threshold);

  my $suffix = "$trivial_threshold.$length_threshold";
  my $script_basename = script_basename();
  my $file = ".$script_basename.summary.$suffix.dat";
  my @key = ((sort {$a cmp $b} keys %$ignored), $trivial_threshold, $length_threshold);
  my $do_summary = sub() {
    my @result = extract_all_mods(%$ignored, $trivial_threshold, $length_threshold);
    print "extract_all_mods: end\n";
    return @result;
  };
  my @result = get_cache_or_run_keyed(@key, $file, $do_summary);
  return @result;
}

my @ignored = ();

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

my ($calling, $referred, $calling_names, $referred_names) = (undef, undef, undef, undef);

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

sub multipart_generator($$) {
  my ($s, $sep) = @_;
  $s = $s =~ s/^\s*(\S+)\s*$/$1/gr;
  my @part = split $sep, $s;
  my $part_num = scalar(@part);
  my $i = -1;
  sub {
    ++$i;
    return undef if $i == $part_num;
    return join $sep, @part[$i .. $part_num - 1];
  }
}
sub search_by_multipart_key($$$) {
  my ($hash, $key, $sep) = @_;
  my $multipart = multipart_generator($key, $sep);
  while (defined(my $lookup_key = $multipart->())) {
    if (exists $hash->{$lookup_key}) {
      return $hash->{$lookup_key};
    }
  }
  return undef;
}

sub referred_tree($$$$$) {
  my ($referred_graph, $mod_name, $filter, $files_excluded, $depth) = @_;
  my $get_id_and_child = sub($$) {
    my ($referred_graph, $node) = @_;
    my $referer = $node->{referer};
    my $lineno = $node->{lineno};
    my $unique_id = "$referer:$lineno";

    if ($referer =~ /$files_excluded/) {
      return (undef, undef);
    }

    my $matched = $referer =~ /$filter/;
    my $referer_list = search_by_multipart_key($referred_graph, $referer, "/");
    if (!defined($referer_list)) {
      return ($matched, $unique_id);
    }
    else {
      # deep copy
      my @child = map {
        my $child = { %$_ };
        $child;
      } @$referer_list;
      return ($matched, $unique_id, @child);
    }
  };
  my $install_child = sub($$) {
    my ($node, $child) = @_;
    $node->{child} = $child;
  };

  my $node = {
    referee => "",
    referer => $mod_name,
    lineno  => "",
    id      => $mod_name,
  };

  return &sub_tree($referred_graph, $node, 0, $depth, {}, $get_id_and_child, $install_child);
}

sub fuzzy_referred_tree($$$$$$) {
  my ($referee_list, $referred_graph, $mod_name_pattern, $filter, $files_excluded, $depth) = @_;
  my $root = {
    referer => $mod_name_pattern,
    referee => $mod_name_pattern,
    lineno  => "",
    id      => "",
    leaf    => undef
  };

  my @mod_name = grep {/$mod_name_pattern/} @$referee_list;

  $root->{child} = [
    grep {defined($_)} map {
      &referred_tree($referred_graph, $_, $filter, $files_excluded, $depth);
    } @mod_name
  ];
  return $root;
}

sub unified_referred_tree($$$$$$) {
  my ($referee_list, $referred_graph, $mod_name, $filter, $files_excluded, $depth) = @_;
  if (exists $referred_graph->{$mod_name}) {
    return &referred_tree($referred_graph, $mod_name, $filter, $files_excluded, $depth);
  }
  else {
    return &fuzzy_referred_tree($referee_list, $referred_graph, $mod_name, $filter, $files_excluded, $depth);
  }
}

=pod
sub calling_tree($$$$$$) {
  my ($calling_graph, $name, $filter, $files_excluded, $depth, $uniques) = @_;

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
  my ($calling_names, $calling_graph, $name_pattern, $filter, $files_excluded, $depth) = @_;
  my @names = grep {/$name_pattern/} @$calling_names;
  my @trees = ();
  my $uniques = {};
  for my $name (@names) {
    my $child0 = $calling_graph->{$name}[0];
    my $child0_file_info = $child0->{file_info};
    my $child0_name = $child0->{name};
    my $child0_unique_id = "$child0_file_info.$child0_name";
    next if exists $uniques->{$child0_unique_id};
    my $tree = calling_tree($calling_graph, $name, $filter, $files_excluded, $depth, $uniques);
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
  my ($name, $filter, $files_excluded, $depth) = @_;
  my $root = undef;
  if (exists $calling->{$name}) {
    $root = &calling_tree($calling, $name, $filter, $files_excluded, $depth * 2, {});
  }
  else {
    $root = &fuzzy_calling_tree($calling_names, $calling, $name, $filter, $files_excluded, $depth * 2);
  }
  return &adjust_calling_tree($root);
}
=cut

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

sub get_entry_of_referred_tree($$) {
  my ($node, $verbose) = @_;

  my $referer = $node->{referer};
  my $lineno = $node->{lineno};
  $referer = $isatty ? "\e[33;32;1m$referer\e[m" : $referer;
  if (defined($verbose) && defined($lineno) && length($lineno) > 0) {
    $referer = "$referer\t[$lineno]";
  }
  return $referer;
}

sub get_child_of_referred_tree($) {
  my $node = shift;
  return exists $node->{child} ? @{$node->{child}} : ();
}

sub format_referred_tree($$) {
  my ($root, $verbose) = @_;
  my @lines = format_tree($root, $verbose, &get_entry_of_referred_tree, &get_child_of_referred_tree);
  return map {"  $_"} ("", @lines, "");
}
=pod
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
  my ($name, $files_excluded) = @_;
  my %names = map {
    $_ => 1
  } grep {
    !is_pure_name($_)
  } grep {
    ($_ =~ /$name/)
  } @$calling_names;

  my @names = grep {!exists $referred->{$_}} sort {$a cmp $b} keys %names;
  my @trees = grep {defined($_)} map {calling_tree($calling, $_, "\\w+", $files_excluded, 2, {})} @names;
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
  my ($name, $files_excluded) = @_;
  my %names = map {
    $_ => 1
  } grep {
    ($_ =~ /$name/) && ($_ !~ /^~/);
  } map {
    simple_name($_)
  } @$referred_names;

  my @names = grep {!exists $calling->{$_}} sort {$a cmp $b} keys %names;
  my @trees = grep {defined($_)} map {referred_tree($referred, $_, "\\w+", $files_excluded, 1)} @names;

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
=cut
use Digest::SHA qw(sha256_hex);
sub cached_sha256_file(@) {
  my @data = (@_);
  my $script_basename = script_basename();
  return ".$script_basename.result.cached." . sha256_hex(@data);
}

my @key = (@ARGV, $isatty, $env_trivial_threshold, $env_length_threshold);

my $mod_name = shift || die "missing module name";
my $filter = shift;
my $mode = shift;
my $verbose = shift;
my $depth = shift;
my $files_excluded = shift;

$filter = (defined($filter) && $filter ne "") ? $filter : ".*";
$mode = (defined($mode) && int($mode) >= 0) ? int($mode) : 1;
$verbose = (defined($verbose) && $verbose ne "0") ? "verbose" : undef;
$depth = (defined($depth) && int($depth) > 0) ? int($depth) : 3;
$files_excluded = (defined($files_excluded) && $files_excluded ne "") ? $files_excluded : '^$';

sub show_tree() {
  my ($referring, $referred, $referer_list, $referee_list) =
    get_cached_or_extract_all_mods(%ignored, $env_trivial_threshold, $env_length_threshold);
  my $tree = unified_referred_tree($referee_list, $referred, $mod_name, $filter, $files_excluded, $depth);
  my @lines = format_referred_tree($tree, $verbose);
  return join qq//, map {"$_\n"} @lines;
}

print get_cache_or_run_keyed(@key, cached_sha256_file(@key), \&show_tree);
