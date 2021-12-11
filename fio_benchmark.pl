#!/usr/bin/perl

use strict;
use warnings;
use Getopt::Long;
use Carp;

our ($OPT_concurrencyInit, $OPT_concurrencyLinearVarying, $OPT_concurrencyExponentialVarying) = (1, 0, 2);
our ($OPT_ioSizeInit, $OPT_ioSizeLinearVarying, $OPT_ioSizeExponentialVarying) = (256, 0, 1);
our ($OPT_fileSize, $OPT_directory, $OPT_timeout, $OPT_stopOnSaturation) = (1*2**19, "", 7200, "true");
our ($OPT_concurrencyMax, $OPT_ioSizeMax) = (500, 64*2**10);

sub options(){ map {/^OPT_(\w+)\b$/; ("$1=s" => eval "*${_}{SCALAR}") } grep {/^OPT_\w+\b$/} keys %:: }

sub usage(){
	my $name = qx(basename $0); chomp $name;
	"USAGE:\n\t" . "$name " . join " ", map{/^OPT_(\w+)$/; "--$1"} grep {/^OPT_\w+\b$/} keys %::;
}
sub show(){
	print join "\n", map {/^OPT_(\w+)\b$/; ("--$1=" . eval "\$$_" ) } grep {/^OPT_\w+\b$/} keys %::;
	print "\n";
}

GetOptions(options()) or die usage();
show();
if (!$OPT_directory){
  die "missing OPT_directory";
}

sub workloadGenerator{
	my ($C, $C_lvary, $C_evary, $IO, $IO_lvary, $IO_evary) = @_;
	sub(){
		my ($c, $io) = ($C, $IO);
		$C += $C_lvary;
		$C *= $C_evary;
		$IO += $IO_lvary;
		$IO *= $IO_evary;
		($c, $io);
	}
}


my $startup = time();
sub since{my $start=shift; time()-$start}

my $WLGen = workloadGenerator(
	$OPT_concurrencyInit, $OPT_concurrencyLinearVarying, $OPT_concurrencyExponentialVarying,
	$OPT_ioSizeInit, $OPT_ioSizeLinearVarying, $OPT_ioSizeExponentialVarying,
);
=pod
for (1..100){
my @a=$WLGen->();
print "@a\n";
}
=cut

sub normbw{
	my ($num, $unit)=split ",", shift;
  if (!defined($num)){
    croak "\$num is undef";
  }
	my %conv=(""=>1, "B"=>1, "KB"=>2**10, "KiB"=>2**10, "MB"=>2**20, "MiB"=>2**20, "GB"=>2**30);
	my $n=$num*$conv{$unit}/1024;
  return $n;
}
sub normlat{
	my ($num, $unit)=split ",", shift;
  printf "\$num=$num, \$unit=$unit\n";
	my %conv=("nsec"=>0.000001, "usec"=>0.001, "msec"=>1, "sec"=>1000, "min"=>60000);
	$num*$conv{$unit};
}
qx(echo -n '' > result.dat);
qx(echo "con\tioSize\tbw\tIOPS\tlat" >> result.dat);
qx(mkdir -p $OPT_directory);
my $fio_args="--ioengine=psync --sync=1 --direct=1 --group_reporting --unlink=1 --rw=write --directory=$OPT_directory";
my $count=0;
while(1){
	if (since($startup) > $OPT_timeout) { print "timeout:\n"; exit 0; }
	my ($concurrency, $ioSize) = $WLGen->();
	if ($concurrency > $OPT_concurrencyMax || $ioSize > $OPT_ioSizeMax) { 
		print "concurrency=$concurrency;ioSize=$ioSize\n";
		exit 0;
	}

	print qq(fio $fio_args --numjobs=$concurrency --name=bs${ioSize}K --bs=${ioSize}K --size=${OPT_fileSize}K > stdout),"\n";
	qx(fio $fio_args --numjobs=$concurrency --name=bs${ioSize}K --bs=${ioSize}K --size=${OPT_fileSize}K > stdout);
	die $! if $?;
	qx(mv stdout stdout.${count});

	my $curr="stdout." . (${count}-0);
	my $curr_bw_unit = qx(perl -ne 'print "\$1,\$2" if/^\\s+write:.*BW=\\b(\\d+(?:\\.\\d+)?)\\s*(\\w+)\\b/' $curr);chomp $curr_bw_unit;
	my $curr_bw=normbw($curr_bw_unit);
	my $curr_iops = qx(perl -ne 'print "\$1" if/^\\s+write:.*IOPS=\\b(\\d+)\\b/' $curr);chomp $curr_iops;
	my $curr_lat_unit= qx(perl -ne 'print "\$2,\$1" if/^\\s+lat\\s*\\((\\w+)\\).*avg=\\b(\\d+(\\.\\d+)?)\\b/' $curr);chomp $curr_lat_unit;
	my $curr_lat=normlat($curr_lat_unit);
	qx(echo "$concurrency\t$ioSize\t$curr_bw\t$curr_iops\t$curr_lat" >> result.dat);
	if ($OPT_stopOnSaturation eq "true" && $count > 0) {
		my $prev="stdout." . (${count}-1);
		my $prev_bw_unit = qx(perl -ne 'print "\$1,\$2" if/^\\s+write:.*BW=\\b(\\d+(?:\\.\\d+)?)\\s*(\\w+)\\b/' $prev);chomp $prev_bw_unit;
		my $prev_bw=normbw($prev_bw_unit);
		my $prev_iops = qx(perl -ne 'print "\$1" if/^\\s+write:.*IOPS=\\b(\\d+)\\b/' $prev);chomp $prev_iops;
		my $prev_lat_unit= qx(perl -ne 'print "\$2,\$1" if/^\\s+lat\\s*\\((\\w+)\\).*avg=\\b(\\d+(\\.\\d+)?)\\b/' $prev);chomp $prev_lat_unit;
		my $prev_lat = normlat($prev_lat_unit);

		my $delta=abs(($curr_bw - $prev_bw)/$curr_bw);
		if ($delta < 0.0005) {
			print "curr_bw=$curr_bw; prev_bw=$prev_bw; delta=$delta\n";
			exit 0;
		}
	}
	$count++;
}
