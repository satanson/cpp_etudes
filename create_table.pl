#!/usr/bin/perl
use strict;
use warnings;
use Data::Dumper;
use List::Util qw(max sum);

my @UTF8_TABLE=((1) x 128, (-1) x 64, (2) x 32, (3) x 16,(4) x 8, (-5) x 8);

sub validate_utf8($) {
  my $s = shift;
  my @bytes = map {unpack "W", $_} split //, $s;
  my $char_size = 0;
  my @char_index = ();
  for (my $i=0; $i < scalar(@bytes);$i+=$char_size) {
    $char_size = $UTF8_TABLE[$bytes[$i]];
    if ($char_size<=0) {
      die sprintf("Invalid start utf8 char: char_size=%d, i=%d; %s", $char_size, $i, join(",", map {sprintf "%#b",$_} @bytes[0 .. $i]));
    }
    for (my $k = $i+1; $k < $i+$char_size; ++$k) {
      if ($UTF8_TABLE[$bytes[$k]] != -1) {
        die sprintf("Invalid continuation utf8 char: char_size=%d, i=%d; %s", $char_size, $k, join(",", map {sprintf "%#b",$_} @bytes[0 .. $k]));
      }
    }
    push @char_index, $i;
  }
  return (\@char_index, \@bytes);
}

sub utf8_length($) {
  my $s = shift;
  my ($char_index) = validate_utf8($s);
  return scalar(@$char_index);
}

sub utf8_substr($$;$) {
  my ($s,$off,$len) = @_;
  my ($char_index, $bytes) = validate_utf8($s);
  my $char_num = scalar(@$char_index);
  if ($off < 0) {
    $off += $char_num;
  }
  my @sub_bytes = @{$bytes}[$char_index->[$off] .. scalar(@$bytes)-1];
  if ($off + $len < $char_num) {
    @sub_bytes = @{$bytes}[$char_index->[$off] .. $char_index->[$off+$len-1]];
  }
  return join "", map{pack "W",$_} @sub_bytes;
}

my @data=();
while(<>){
  if (/^\s*$/){
    push @data, undef;
  } else {
    s/^\s*(\S.*\S|\S+)\s*$/$1/g;
    push @data, [split /\s*,\s*/, $_];
  }
}
#print Dumper(\@data);

my $column_num = max(map {scalar(@$_)} grep {defined($_)} @data);

@data=map { 
  if (defined($_)) {
    my @row =(@$_, ("") x $column_num);
    [ @row[0 .. ($column_num -1)]]
  } else{
    $_
  } 
} @data;

#print Dumper(\@data);

my @max_len = map{my $col=$_; max(map {utf8_length($_->[$col])} grep {defined($_)} @data)} 0 .. ($column_num-1);

#print Dumper(\@max_len);
@data = map {
  my $row = $_; 
  if (defined($row)){
    [ map {utf8_substr(' '.$row->[$_].(' ' x ($max_len[$_]+1)), 0, $max_len[$_]+2 )}  0 .. ($column_num-1)]
  } else {
    undef
  }} @data;
#print Dumper(\@data);
my @row_length=sum(@max_len)+$column_num+1;

my $thin_hrule='+'.(join qq/+/, map{'-' x ($_+2)} @max_len).'+';
my $thick_hrule='+'.(join qq/+/, map{'=' x ($_+2)} @max_len).'+';


sub print_thin_hrule(){
  print "$thin_hrule\n";
}

sub print_thick_hrule(){
  print "$thick_hrule\n";
}

sub print_data($){
  my $row = shift;
  print "|".(join qq/|/, @$row). "|\n";
}


sub remove_leading_undef(\@) {
  my @array=@{shift;};

  while (1) {
    my $elem = shift @array;
    if (defined($elem)){
      unshift @array, $elem;
      last;
    }
  }
  return @array;
}

sub remove_trailing_undef(\@) {
  my @array=@{shift;};
  while (1) {
    my $elem = pop @array;
    if (defined($elem)){
      push @array, $elem;
      last;
    }
  }
  return @array;
}

sub squeeze_undef(\@) {
  my @array=@{shift;};
  my $k = 0;
  my $i = 1;
  for (; $i <= $#array; ++$i) {
    if (defined($array[$i])) {
      $array[++$k] = $array[$i];
    } else {
      if (defined($array[$k])) {
        $array[++$k] = undef;
      } else {
        # do nothing
      }
    }
  }
  return @array[0 .. $k];
}

@data = remove_leading_undef @data;
my ($hdr, @body) = @data;
@body = remove_leading_undef @body;
@body = remove_trailing_undef @body;
@body = squeeze_undef @body;

#print hdr
print_thick_hrule;
print_data $hdr;
print_thick_hrule;
foreach (0 .. $#body){
  if (defined($body[$_])) {
    print_data $body[$_];
  } else {
    print_thin_hrule;
  }
}
print_thin_hrule;
