#!/usr/bin/perl -w                                                                                                                                           
open(CONLL, "<$ARGV[0]");
open(TAGGING, "<$ARGV[1]");

$i = 0;
while($line = <CONLL>) {
  $line_tagging = <TAGGING>;  
  chomp($line);
  chomp($line_tagging);
  if (length($line) == 0) {
    print "\n";
    $i = 0;
  } else {
    $i++;
    @fields = split(/\t/, $line);
    @fields_tagging = split(/\t/, $line_tagging);
    $pos = $fields_tagging[1];
    # Define coarse POS tags which are the first two characters of the POS tag.
    if (($pos ne "PRP") && ($pos ne "PRP\$")) {
      $cpos = substr($pos, 0, 2);
    } else {
      $cpos = $pos;
    }
    # Replace gold tags by predicted tags, and eliminate lemmas and 
    # morpho-syntactic features.
    print $fields[0] . "\t" . $fields[1] . "\t" . "_" . "\t" . $cpos . "\t" . $pos . "\t" . "_" . "\t" . $fields[6] . "\t" . $fields[7] . "\n";
  }
}

close(CONLL);
close(TAGGING);
