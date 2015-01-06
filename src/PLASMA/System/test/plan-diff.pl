#!/usr/bin/perl -w

use warnings qw/all/;
use strict;

use Algorithm::Diff;

my %plans = ();
my $token_key_rx = qr/Key=\d+\s+Master=(?:\d+|NONE)/;
my $merge_key_rx = qr/Merged Key=\d+/;

exit(plan_compare($ARGV[0], $ARGV[1]));

sub plan_compare {
  my $planfile1 = shift;
  my $planfile2 = shift;
  
  my $plan1 = get_plan($planfile1);
  my $plan2 = get_plan($planfile2);
  
  my $diff = Algorithm::Diff->new($plan1, $plan2);
  
  my %diffs1 = ();
  my %diffs2 = ();
  my @lines1 = ();
  my @lines2 = ();
  my $retval = 0;
  $diff->Base(1); #use line numbers
  while($diff->Next()) {
    next if($diff->Same()); #skip anything that's the same
    my @items1 = $diff->Items(1); #empty if the diff is only insertions
    my @items2 = $diff->Items(2); #empty if the diff is only deletions
    
    @items1 = remove_rx($token_key_rx, @items1); #token key differences don't matter
    @items2 = remove_rx($token_key_rx, @items2);
    next if (@items1 == @items2 && @items1 == 0);
    
    if(@items1 == @items2) {
      @items1 = remove_rx($merge_key_rx, @items1); #merged key differences don't matter
      @items2 = remove_rx($merge_key_rx, @items2); #as long as there are the same number of merged tokens
      next if(@items1 == @items2 && @items1 == 0);
    }
    
    @items1 = remove_rx(qr/world\./, @items1); #differences in the world object don't matter
    @items2 = remove_rx(qr/world\./, @items2);
    
    @items1 = remove_rx(qr/ound plan/, @items1); #differences in step numbers don't matter
    @items2 = remove_rx(qr/ound plan/, @items2);
    next if(@items1 == @items2 && @items1 == 0);
    
    if(@items1 > 0) {
      $diffs1{$diff->Min(1)} = \@items1;
      push @lines1, $diff->Min(1);
    }
    if(@items2 > 0) {
      $diffs2{$diff->Min(2)} = \@items2 ;
      push @lines2, $diff->Min(2);
    }
  }
  
  if(@lines1 != @lines2) {
  # print "Plans $planfile1 and $planfile2 are very definitely different.\n";
   $retval = 1;
  }
  
  my $min = (@lines1 < @lines2 ? @lines1 : @lines2);
 # print "The minimum number of differing lines is: " . $min . "\n";
  foreach my $i (0..$min) {
    next if !(defined($lines1[$i]) && defined($lines2[$i]));

    my $subdiff = Algorithm::Diff->new($diffs1{$lines1[$i]},
				       $diffs2{$lines2[$i]});
    while($subdiff->Next()) {
      next if ($subdiff->Same());
      my @subitems1 = $subdiff->Items(1);
      my @subitems2 = $subdiff->Items(2);
      print "=====================\n";
      print "$planfile1: [", $lines1[$i] + $subdiff->Min(1), "]\n";
      map {print $_} @subitems1;
      print "======================\n";
      print "$planfile2: [", $lines2[$i] + $subdiff->Min(2), "]\n";
      map {print $_} @subitems2;
      $retval = 1;
    }
  }
  
  if(@lines1 > @lines2) {
    foreach(($min == 0 ? $min : $min+1)..$#lines1) {
      print "===================\n";
      print "$planfile1: [", $lines1[$_], "]\n";
      map {print $_} @{$diffs1{$lines1[$_]}};
      $retval = 1;
    }
  }
  elsif(@lines2 > @lines1) {
    foreach(($min == 0 ? $min : $min+1)..$#lines2) {
      print "===================\n";
      print "$planfile2: [", $lines2[$_], "]\n";
      map {print $_} @{$diffs2{$lines2[$_]}};
      $retval = 1;
    }
  }
  return $retval;
}

sub get_plan {
  my $file = shift;
  open my $fh, $file or die "Failed to open file $file: $!\n";
  return extract_plan($fh);
}

sub extract_plan {
  my $fh = shift;
  my @plan = ();
  local $_;
  while(<$fh>) {
    #print "$.: $_";
    last if /Objects\s+\*+/;
  }
  #print "Pushing line: $_";
  push @plan, $_;
  while(<$fh>) {
    #given the current plan output
    #there are only merged and inactive tokens after this point,
    #which don't really matter
    last if(/Merged Tokens:\s*\*{4,}/);

    if(/.+\s*\*{4,}/ || # Objects **** or Variables ***** etc.
       /.+=.+:.+/ || #object.var=type:DOMAIN
       /\[\s+.+:.+\s+\]/ || # [ INT_INTERVAL:CLOSED[50, 65] ]
       /\.+\((?:.+=.+[}\]])*\)/ || #object.predicate(parameter=type:DOMAINparameter=type:DOMAIN)
       /$token_key_rx/ || #Key=123 Master=none
       /$merge_key_rx/ ) {
      #print "Pushing line: $_";
      push @plan, $_;
    }
  }
  return \@plan;
}

sub remove_rx {
  my $rx = shift;
  return grep {$_ !~ /$rx/} @_;
}
