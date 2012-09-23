#!/usr/bin/perl -w

# Input file containing the original sentences (one per line, tokenized, words separated by whitespaces).
open(DOC, "<$ARGV[0]"); 

while($str = <DOC>) {
  chomp($str);
  # Split on spaces to form array of tagged words.
  @words = split(/[\s]+/, $str); 
  $iword = 0;
  foreach $word (@words) {
    print $iword+1, "\t", $word, "\t_\t_\t_\t_\t_\t_\t_\t_\n";
    $iword++;
  }
  print "\n";
}

close(DOC);

