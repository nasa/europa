#!/usr/bin/perl -w

use strict;

# Use: Convert old style constraint tests to new XML constraint tests.
# Warning: This script may contain content not suitable for Good Programmers(TM).
#          Should you attempt to use this script with bad or oddly formatted tests,
#          it will likely break.  Consider yourself warned.

my %symbols;

sub getValueType
{
	my ($value) = $_[0] =~ /(?:[[{]\s*)?([^ \t}\]]+)/;
	if(!defined $value or $value =~ /^\s*$/)  { return undef;}
	if($value =~ /^([-+])?\d+$/
	|| $value =~ /^([-+])?[Ii]nf(inity)?$/)   { return "int";}
	if($value =~ /^([-+])?\d+\.(\d+)?$/
	|| $value =~ /^([-+])?[Ii]nf(inity)?\.$/) { return "float";}
	if(lc($value) =~ /^(true|false)$/)        { return "boolean";}
	if($value =~ /^\".*\"$/)                  { return "string";}
	if(defined $symbols{$value})              { return $symbols{$value};}
	#this allows us to add type to symbol domains.
	print "What is the type of '$value': ";
	my $type = undef;
	chomp($type = <STDIN>);
	$symbols{$value} = $type;
	return $type;
}

sub getValue {
	my $value = $_[0];
	if($value =~ /^([-+])?\d+(\.(\d+)?)?$/)    { return $value;}
	if($value =~ /^(\+)?[Ii]nf(inity)?(\.)?$/) { return "+inf";}
	if($value =~ /^-[Ii]nf(inity)?(\.)?$/)     { return "-inf";}
	if(lc($value) =~ /^(true|false)$/)         { return lc($value);}
	if($value =~ /^\"(.*)\"$/)                   { return $1;}
	return $value;
}

sub getSet {
	my @set = split /\s+/, $_[0];
	my $toRet = "";
	my $type = undef;
	foreach my $element (@set) {
		if(!defined $type) {$type = getValueType($element);}
		elsif($type ne getValueType($element)) {die "Enumerated sets must contain only one type of element \"". getValue($element) ."\" is not a " . getValueType($element);}
		$toRet .= "<element value=\"" . getValue($element) . "\"/>";
	}
	return $toRet;
}

sub getDomain {
	my $domain = $_[0];
	my $type = $_[1];
	if(!defined $type) { $type = getValueType($domain);}
	if($domain =~ /\[\]/)               { return "<IntervalDomain/>";}
	if($domain =~ /\[(\S+)\]/) {
		return "<IntervalIntDomain lb=\"" . getValue($1) . "\" ub=\"" . getValue($1) . "\"/>" if($type eq "int");
		return "<IntervalDomain lb=\"" . getValue($1) . "\" ub=\"" . getValue($1) . "\"/>";
	}
	if($domain =~ /\[(\S+) (\S+)\]/) {
		return "<IntervalIntDomain lb=\"" . getValue($1) . "\" ub=\"" . getValue($2) . "\"/>" if($type eq "int");
		return "<IntervalDomain lb=\"" . getValue($1) . "\" ub=\"" . getValue($2) . "\"/>";
	}
	if($domain =~ /\{(.*?)}/) {
		warn "input and output domain types must match" unless (!defined $_[1] || $type eq $_[1]);
		if($1 ne "")
		{
			return "<BoolDomain>".getSet($1)."</BoolDomain>" if($type eq "boolean");
			return "<NumericDomain>".getSet($1)."</NumericDomain>" if($type eq "int" or $type eq "float");
			return "<StringDomain>".getSet($1)."</StringDomain>" if($type eq "string");
			return "<SymbolDomain type=\"$type\">".getSet($1)."</SymbolDomain>";
		}
		else
		{
			return "<EmptyDomain/>" if(!defined $type);
			return "<BoolDomain/>" if($type eq "boolean");
			return "<NumericDomain/>" if($type eq "numeric");
			return "<StringDomain/>" if($type eq "string");
			return "<SymbolDomain type=\"$type\"/>";
		}
	}
	die "could not convert domain $domain";
}

sub usage
{
	print "./converter.pl <Test constraint table file> <XML file>\n";
}

################################################################################
# Main body of code begins here.                                               #
################################################################################

usage and die "Must be exactly 2 arguments" if($#ARGV != 2);
usage and die "Could not find file \"$ARGV[0]\"." if(!-e $ARGV[0]);
open my $file, "< $ARGV[0]" or die "Can't open $ARGV[0]: $!";
open my $xml, "> $ARGV[1]" or die "Can't open $ARGV[1]: $!";

print $xml "<?xml version=\"1.0\"?>\n";
print $xml "<ConstraintTests>\n";

my $case = 1;
foreach my $line (<$file>)
{
	chomp $line;
	my ($constraint, $in, $out, $comment) = $line =~ /^\d* *(\S+)\s+inputs\s+(.+?)outputs\s+(.+?)(?:\s*#(.*))?$/;
	my @inputs = split(/(?<=[\]}])\s+(?=[[{])/,$in);
	my @outputs = split(/(?<=[\]}])\s+(?=[[{])/,$out);
	if($#inputs == $#outputs)
	{
		print $xml "\t<!-- $comment -->\n" if(defined $comment);
		print $xml "\t<Constraint name=\"$constraint\" test=\"$case\">\n";
		print $xml "\t\t<Inputs>\n";
		foreach (@inputs) { print $xml "\t\t\t",getDomain($_),"\n"; }
		print $xml "\t\t</Inputs>\n";

		print $xml "\t\t<Outputs>\n";
		for(my $i = 0; $i <= $#outputs; $i++) { print $xml "\t\t\t",getDomain($outputs[$i],getValueType($inputs[$i])),"\n"; }
		print $xml "\t\t</Outputs>\n";

		print $xml "\t</Constraint>\n";
		$case++;
	}
	else
	{
		warn "Skipping test, count of inputs and outputs should match.";
	}
}

print $xml "</ConstraintTests>\n";

close $xml;
close $file;
