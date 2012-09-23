#!/usr/bin/perl -w

open(PRED, "<$ARGV[0]"); # Prediction file (tagging).
open(GOLD, "<$ARGV[1]"); # Gold file (tagging).

$ntok = 0;
$ncorr = 0;
while($line = <GOLD>)
{
	$line_pred = <PRED>;
	chomp($line);
	chomp($line_pred);
	if (length($line) > 0)
	{
		$ntok++;
		@fields = split(/\t/, $line);
		$pos = $fields[1];
		@fields = split(/\t/, $line_pred);
		$pos_pred = $fields[1];

		if ($pos eq $pos_pred)
		{
			$ncorr++;
		}
		else
		{
			#print $pos, "->", $pos_pred, "\n";
		}
	}
}

print "Accuracy = ", $ncorr/$ntok, "\n";

