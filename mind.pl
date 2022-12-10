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

#my @lines = split /\n/, $lines;
#print join "", map{"$_\n"} @lines;
my %subst_tab=(
  "<cs1>"=>"\e[33;93;3m",
  "<cs2>"=>"\e[31;32;1m",
  "<cs3>"=>"\e[31;31;1m",
  "<end>"=>"\e[m",
);
sub subst(@){
  my @lines=(@_);
  return map{
    my $ln=$_; 
    foreach my $pat (keys %subst_tab){
      $ln=~s/$pat/$subst_tab{$pat}/g;
    }
    $ln;
  } @lines;
}
my $tree = mind(subst(map{chop;$_}<ARGV>));
print join qq//, map{"$_\n"} format_mind($tree);
