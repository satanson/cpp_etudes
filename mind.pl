#!/usr/bin/perl
use strict;
use warnings;
use Data::Dumper;

sub mind(@) {
  my ($title, @lines) = @_;
  # [level, content, children]
  my @stk=([0, $title, []]);
  foreach my $ln (@lines) {
    if ($ln =~/^([-]*)\s*(.*)/) {
      my ($dashes, $content) = ($1, $2);
      my $level = length($dashes);
      while (scalar(@stk) > 0 && $stk[-1][0] + 1 != $level) {
        pop @stk;
      }
      die "Ill-formed mind input" if (scalar(@stk) == 0);
      my $top = $stk[-1];
      my $curr = [$level, $content, []];
      push @stk, $curr;
      my ($top_level,$parent_content,$top_children) = @$top;
      push @$top_children, $curr;
    }
  }
  return $stk[0];
}

my $lines =<<DONE
calltree.pl
- prerequisites
-- perl 5.10+: Most Linux distributions and MacOS have intalled perl 5.10+.
-- ag(the_silver_searcher): 
--- Must be installed,
--- Although calltree.pl can leverage official https://github.com/ggreer/the_silver_searcher, but not the best choice.
--- Suggest to install customized https://github.com/satanson/the_silver_searcher that extract function body precisely.
-- calltree.pl/java_calltree.pl/cpptree.pl/javatree.pl/deptree.pl are standalone scripts with no others dependencies.
-
- format: calltree.pl '<src_regex>' '[[-]<dst_regex>]' <mode(0|1|2|3|4)> <verbose> <[-]depth> [[-]file_name_regex]
- REGEX in calltree.pl are perl regex, and must be enclosed by quotes.
- 
- arguments: 
-- <src_regex>: (required) Search since functions match `src_regex`, these functions will be root of subtrees.
--- demos
---- calltree.pl '(\\bfsync|fdatasync|sync_file_range\\b)' 'Write'  1 1 3 # rocksdb
---- calltree.pl '((?i)\\b\\w*compaction\\w*\\b)' ''  1 1 3 # rocksdb
-- [[-]<dst_regex>]: (required) Search till functions match `dst_regex`, these functions will be leaf of subtrees.
--- demos
---- calltree.pl '(\\bfsync|fdatasync|sync_file_range\\b)' ''  1 1 3 # empty '' means match any functions 
---- calltree.pl '(\\bfsync|fdatasync|sync_file_range\\b)' '-Write'  1 1 3 # '-<regex>' means interested in functions not match the regex
---- calltree.pl '(\\bfsync|fdatasync|sync_file_range\\b)' '(?i)compaction'  1 1 3 # '<regex>' means interested in functions match the regex
-- 
-- <mode>: (required)
--- 0: show calling tree(caller is parent node, callee is child node).
---- demos
----- calltree.pl 'parsePassPipeline' '(Pass|Analysis|Pipeline)\$' 0 1 2 # llvm
----- calltree.pl 'main' 'Pipeline' 0 1 3 # llvm
--- 1: show called tree(callee is parent node, caller is child node).
---- demos
----- 
----- 
--- 2: show outermost functions that call others and have no callers.
--- 3: show intermost functions that are called by others and have no callees.
--- 4: show called tree and used to find functions/files that enclosing the contents matches <src_regex>.
--
-- <verbose>: (required)
--- 0: not show file-lineno infos, take effect on all modes.
--- 1: show file-lineno infos, take effect on all modes.
--- qhcv or hcv: a four-digit or three-digit number, take effect only on mode 0.
---- q: quiet, 0 for non-quiet(default) and 1 for quiet, not show common subtrees when quiet.
---- h: height of common subtrees, >=3(default), subtrees height than `h` will be extracted as common subtrees.
---- c: repetition count of common subtrees, >=2(default), subtrees ocurrs at least for `c` times will be considered as common subtrees.
---- v: verbose, 0 for not showing file-lineno infos and 1 for showing file-lineno infos.
---- motivation: mode 0(show calling tree) leverages qhcv and hcv to speed up searching and collapse duplicate calling patterns.
---- demos:
-----  a
-----  b
-----  c
--
-- [-]<depth>: (required) the depth of the tree output, a larger depth means cost more times to generate trees.
--- depth > 0: take effect on all modes.
--- depth < 0: only take effect on mode 1 (show called tree),
---- It is quite similar to |depth|, but adjacent functions with the name will be collapsed to only one.
---- It is very useful when your are reading codes that designed in Visitor or Iterator pattern.
--- demos:
---- a
---- b
---- c
--
-- [[-]<file_name_regex>]: (optional) Sometimes, your are only interested in functions in files whose names match the regex or not match.
--- demos
---- a
---- b
-
- How to reach me: 
-- E-Mail: ranpanf\@gmail.com
DONE
;

sub should_prune_subtree($$) {
}

sub format_tree($$$$\&\&) {
  my ($root, $level, $verbose, $enable_prune, $get_entry, $get_child) = @_;
  $root->{level} = $level;
  unless (defined($root) && %$root) {
    return ();
  }

  my ($common_idx) = should_prune_subtree($root, $enable_prune);
  my $entry = $get_entry->($root, $verbose, $common_idx);
  my @result = ($entry);

  my @child = $get_child->($root);
  if (!scalar(@child)) {
    return @result;
  }

  my $last_child = pop @child;

  foreach my $chd (@child) {
    my ($first, @rest) = &format_tree($chd, $level + 1, $verbose, $enable_prune, $get_entry, $get_child);
    if ((!defined($first) || $first =~/^\s*$/) && scalar(@rest)==0) {
      push @result, "│";
      next;
    }
    push @result, "├── $first";
    push @result, map {"│   $_"} @rest;
  }

  my ($first, @rest) = &format_tree($last_child, $level + 1, $verbose, $enable_prune, $get_entry, $get_child);
  push @result, "└── $first";
  push @result, map {"    $_"} @rest;
  return @result;
}
sub format_mind($) {
  my $root = shift;
  my $get_entry = sub($$$) {
    my $root = shift;
    return $root->{name};
  };
  my $get_child = sub($) {
    my $root = shift;
    return map {+{level=>$_->[0], name=>$_->[1], child=>$_->[2]}} @{$root->{child}};
  };
  my $tree_root = {
    level=>$root->[0],
    name=>$root->[1],
    child=>$root->[2],
  };

  &format_tree($tree_root, 0, 0, 0, $get_entry, $get_child);
}

my @lines = split /\n/, $lines;
print join qq//, map{"$_\n"} format_mind(mind(@lines));
