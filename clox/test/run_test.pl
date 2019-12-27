#!/usr/bin/perl
use strict;
use warnings;
use File::Slurp;
use Term::ANSIColor;

sub read_files {
	opendir my $dir, $_[0] or die "Cannot open $_[0] dir $!";
	my @files = readdir $dir;
	closedir $dir;
	return @files;
}

sub read_files_as_hash {
	my %hash;
	my $base_path = $_[0];
	foreach(read_files($base_path)) {
		$hash{$_} = "$base_path/$_";
	}
	delete $hash{"."};
	delete $hash{".."};
	return %hash;
}

sub clean_entry {
	$_[0] =~ s/^\s+|\s+$//g;
	return $_[0];
}

my $prog_dir = "./programs";
my $test_dir = "./test/cases";
my $clox_bin = "./build/clox";

my %prog = read_files_as_hash($prog_dir);
my %tests = read_files_as_hash($test_dir);
if($ARGV[0]) {
	print "\n======================\n";
	print "Loaded tests:\n";
	print map { "$_ => $tests{$_}\n" } keys %tests;
	print "\n";
	print "Loaded programs\n";
	print map { "$_ => $prog{$_}\n" } keys %prog;
	print "======================\n\n";
}

my $have_err = 0;

while(my ($key, $value) = each(%tests)) {
	if($prog{$key}) {
		print("RUNNING TEST: $key... ");
		my $result = `$clox_bin $prog{$key}`;
		if(!$result) {
			$result = '';
		}
		$result = clean_entry($result);
		my $expected = clean_entry(read_file($value));
		if($expected eq $result) {
			print color('bold green');
			print "OK\n";
			print color('reset');
		} else {
			print color('bold red');
			print "FAIL\n";
			print "EXPECTED:\n$expected\nBUT HAVE:\n$result\n";
			print color('reset');
			$have_err = 1;
		}
	}
}

exit($have_err);