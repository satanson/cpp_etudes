#!/usr/bin/bash

find | perl -lne '$count->{$1}+=1 if m{/[^/]+\.(\w+)$}}{print join "\n", map {$_->[0] .":". $_->[1]} sort{$b->[1] <=> $a->[1]} map{[$_, $count->{$_}]} grep {length($_)<16}keys %$count'
