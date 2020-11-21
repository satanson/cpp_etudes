# Copyright (c) 2020 Ran Panfeng.  All rights reserved.
# Author: satanson
# Email: ranpanf@gmail.com

#!/usr/bin/perl
use warnings;
use strict;
use Data::Dumper;

sub ensure_ag_installed() {
    my ($ag_path) = map {chomp;
        $_} qx(which ag 2>/dev/null);
    if (!defined($ag_path) || (!-e $ag_path)) {
        printf STDERR "ag is missing, please install ag at first\n";
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
            $_} qx(ag -G '\.(c|cc|cpp|C|h|hh|hpp|H)\$' '$cls_re');
        @matches = merge_lines @matches;
        @matches = map {s/$attr_re//g;
            $_} @matches;
        @matches = map {s/$access_specifier_re//g;
            $_} @matches;
        @matches = map {s/$template_arguments_re//g;
            $_} @matches;
        @matches = map {s/$template_arguments_re//g;
            $_} @matches;
        @matches = map {s/$template_arguments_re//g;
            $_} @matches;
        @matches = map {s/$template_arguments_re//g;
            $_} @matches;
        @matches = map {s/template//g;
            $_} @matches;
        @matches = map {s/^(.+?)\s*{/$1/g;
            $_} @matches;
        @matches = map {s/\s+/ /g;
            $_} @matches;
        @matches = grep {/$cls_filter_re/} @matches;
        open my $cache_file_handle, "+>", $cache_file or die "$!";
        print $cache_file_handle join("\n", @matches);
        close($cache_file_handle);
        #qx(echo -n > $cache_file);
        #foreach my $line (@matches) {
        #  #print qq(echo "$line" >>$cache_file), "\n";
        #  qx(echo "$line" >>$cache_file);
        #}
    }

    #print Dumper(\@matches);

    my %table = map {$_->[1] => $_} grep {defined($_)} map {if (/^(\S+)\s*:\s*$class0_re/) {[ $1, $2 ]}
    else {
        print "undefined:$_\n";
        undef
    }} @matches;
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
            push @{$tree->{$parent}}, $e;
        }
    }
    return $tree, \%table;
}

my $cls = shift || die "missing class name";
my $verbose = shift;
my ($tree, $table) = all_sub_classes();

sub sub_class($) {
    my ($cls) = @_;
    my $root = { name => $cls, child => [] };
    if (!exists $tree->{$cls}) {
        return $root;
    }

    my $child = $tree->{$cls};

    foreach my $chd (@$child) {
        push @{$root->{child}}, &sub_class($chd->[1]);
    }
    return $root;
}

sub fuzzy_sub_class($) {
    my ($cls_pattern) = @_;
    my $root = { name => $cls_pattern };
    my @names = grep {/$cls_pattern/} (keys %$table);
    #print Dumper(\@names);
    $root->{child} = [ map {&sub_class($_)} @names ];
    return $root;
}

sub unified_sub_class($) {
    my ($cls) = @_;
    if (!exists $tree->{$cls}) {
        return &fuzzy_sub_class($cls);
    }
    else {
        return &sub_class($cls);
    }
}

#print Dumper(all_sub_classes);
my $hierarchy = unified_sub_class $cls;
#print Dumper($hierarchy);
#
sub format_tree($;$) {
    my ($root, $verbose) = @_;
    unless (%$root) {
        return ();
    }
    my $name = $root->{name};
    my @child = @{$root->{child}};

    my $line = $name;
    if (defined($verbose) && exists $table->{$name}) {
        $line = $name . "\t[" . $table->{$name}[0] . "]";
    }
    my @result = ($line);

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
