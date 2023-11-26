#!/bin/perl
use strict;
use warnings;
use Data::Dumper;

sub urlDecode($) {
  my @chars=split //, $_[0];
  my $s = "";
  for (my $i=0;$i < scalar(@chars); ++$i) {
    my $ch = $chars[$i];
    if ($ch eq "%") {
      my $hexCode = join "", @chars[($i+1)..($i+2)];
      $i+=2;
      $s .= chr(hex("0x".$hexCode));
    } else {
      $s .= $ch;
    }
  }
  return $s;
}

sub parseUrlParams($) {
  my $url = shift;
  my $paramStr = $1 if $url =~ /[^?]*\?(.*)/;
  return {} unless defined($paramStr);
  return {map{urlDecode($_)} map{split/=/} split /&/, $paramStr}
}

sub getParamKey($$) {
  my ($url,$key) = (@_);
  my $params=parseUrlParams($url);
  if (exists $params->{$key}) {
    return join "_", map {qq/\u$_/} split /\W+/, $params->{$key};
  } else {
    return undef;
  }
}

my ($dir)=map{chomp;$_} qx(cd \$(dirname $0);pwd);
print qq/chdir $dir\n/;
chdir $dir;

my @logs=map{chomp;$_} qx(ls *.log);
@logs=grep{ my $complete=()=qx(grep "100%" $_);my $saved=()=qx(grep "saved" $_); $complete>0 && $saved>0;} @logs;
for my $log (@logs) {
	if ($log =~ /(.*)(\.log)/) {
		my $fileName=$1;
		my $incompleteFileName=$fileName . ".incomplete";
		if (-e $incompleteFileName) {
			rename $incompleteFileName => $fileName;
		}
		unlink $log;
	}
}

my $downloadListFile="download.list";
unless (-f $downloadListFile) {
	print "$downloadListFile not exists\n";
	exit 0;
}

my @downloadList=grep {/(http|https|ftp|ftps)/} map{chomp;$_} qx(cat $downloadListFile);
print Dumper(\@downloadList);
my ($ts) = map {chomp;$_} qx(date +"%Y%m%d_%H%M%S");
rename $downloadListFile => $downloadListFile . ".backup." . $ts;

for my $e (@downloadList) {
	my ($fileName,$url)=(undef, undef);
	if ($e =~ m{^\s*((?:http|https|ftp|ftps)://\S+)}) {
		$url = $1;
		$fileName=getParamKey($url, "title");
	} elsif ($e =~ m{\s*(.+?)\s*((?:https|http|ftp|ftps)://.*)}) {
		$url = $2;
		$fileName = join "_", map {qq/\u$_/} split /\W+/, $1;
	} else {
		next;
	}
	next unless (defined($url) && defined($fileName) && $url ne "" && $fileName ne "");
	my $incompleteFileName = $fileName . ".incomplete";
	next if (-e $fileName || -e $incompleteFileName);

	print qq(env https_proxy=127.0.0.1:7777 nohup wget -O ${incompleteFileName} '${url}' >${fileName}.log 2>&1 &),"\n";
	qx(env https_proxy=127.0.0.1:7777 nohup wget -O ${incompleteFileName} '${url}' >${fileName}.log 2>&1 &);
}
