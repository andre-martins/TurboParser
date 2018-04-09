#!/usr/bin/perl -w
open(CONLL, "<$ARGV[0]");
open(TAGGING, "<$ARGV[1]");

$define_coarse_tags = 0;
$replace_lemmas_and_morph = 1;

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
    $cpos = $pos;
    if ($define_coarse_tags) {
        # Define coarse POS tags which are the first two characters of the POS tag.
        if (($pos ne "PRP") && ($pos ne "PRP\$")) {
            $cpos = substr($pos, 0, 2);
        }
    }
    # Replace gold tags by predicted tags, and eliminate lemmas and
    # morpho-syntactic features.
    if ($replace_lemmas_and_morph) {
        print $fields[0] . "\t" . $fields[1] . "\t" . "_" . "\t" . $cpos . "\t" . $pos . "\t" . "_" . "\t" . $fields[6] . "\t" . $fields[7] . "\n";
    } else {
        print $fields[0] . "\t" . $fields[1] . "\t" . $fields[2] . "\t" . $cpos . "\t" . $pos . "\t" . $fields[5] . "\t" . $fields[6] . "\t" . $fields[7] . "\n";
    }
  }
}

close(CONLL);
close(TAGGING);
