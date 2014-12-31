#!/usr/bin/env perl
# $Id: eval08.pl,v 1.8 2008/10/22 15:00:00 mihais Exp $
# Based on eval07.pl,v 1.4 2007/04/02 21:17:34

# Author: Yuval Krymolowski
# Addition of precision and recall 
#   and of frame confusion list: Sabine Buchholz
# Addition of DEPREL + ATTACHMENT:
#   Prokopis Prokopidis (prokopis at ilsp dot gr)
# Acknowledgements: 
#   to Markus Kuhn for suggesting the use of 
#   the Unicode category property
# Adaptation to CoNLL-07:
#   Deniz Yuret (denizyuret at gmail dot com)
#
# Adaptation to CoNLL-08 (added scores for semantic frames):
#   Mihai Surdeanu (mihai at surdeanu dot name)
# Acknowledgements:
#   James Henderson for the "Exact match" scores
#  

if ($] < 5.008001)
{
  printf STDERR <<EOM

 This script requires PERL 5.8.1 for running.
 The new version is needed for proper handling
 of Unicode characters.

 Please obtain a new version or contact the shared task team
 if you are unable to upgrade PERL.

EOM
;
  exit(1) ;
}

require Encode;

use strict ;
use warnings;
use Getopt::Std ;

my ($usage) = <<EOT

  CoNLL-08 evaluation script:

   [perl] eval08.pl [OPTIONS] -g <gold standard> -s <system output>

  This script evaluates a system output with respect to a gold standard.
  Both files should be in UTF-8 encoded CoNLL-08 tabular format.

  The output breaks down the errors according to their type and context.

  Optional parameters:
     -o FILE : output: print output to FILE (default is standard output)
     -q : quiet:       only print overall performance, without the details
     -b : evalb:       produce output in a format similar to evalb 
                       (http://nlp.cs.nyu.edu/evalb/); use together with -q
     -p : punctuation: do not score punctuation (default is to score)
     -u : SU preds/args: score SU predicates and arguments (default is NOT to score)
     -v : version:     show the version number
     -h : help:        print this help text and exit

EOT
;

my ($line_num) ;
my ($sep) = '0x01' ;

my ($START) = '.S' ;
my ($END) = '.E' ;

my ($con_err_num) = 3 ;
my ($freq_err_num) = 10 ;
my ($spec_err_loc_con) = 8 ;

our ($opt_g, $opt_s, $opt_o, $opt_h, $opt_v, $opt_q, $opt_p, $opt_u, $opt_b, $opt_d) ;
my ($word_mismatch_warning);

my $skipped_su_preds_gold = 0;
my $skipped_su_args_gold = 0;
my $skipped_su_preds_sys = 0;
my $skipped_su_args_sys = 0;

################################################################################
###                              subfunctions                                ###
################################################################################

# Whether a string consists entirely of characters with the Unicode
# category property "Punctuation" (see "man perlunicode")
sub is_uni_punct
{
  my ($word) = @_ ;

  return scalar(Encode::decode_utf8($word)=~ /^\p{Punctuation}+$/) ;
}

# The length of a unicode string, excluding non-spacing marks
# (for example vowel marks in Arabic)

sub uni_len
{
  my ($word) = @_ ;
  my ($ch, $l) ;

  $l = 0 ;
  foreach $ch (split(//,  Encode::decode_utf8($word)))
  {
    if ($ch !~ /^\p{NonspacingMark}/)
    {
      $l++ ;
    }
  }

  return $l ;
}

sub filter_context_counts
{ # filter_context_counts

  my ($vec, $num, $max_len) = @_ ;
  my ($con, $l, $thresh) ;

  $thresh = (sort {$b <=> $a} values %{$vec})[$num-1] ;

  foreach $con (keys %{$vec})
  {
    if (${$vec}{$con} < $thresh)
    {
      delete ${$vec}{$con} ;
      next ;
    }

    $l = uni_len($con) ;

    if ($l > ${$max_len})
    {
      ${$max_len} = $l ;
    }
  }

} # filter_context_counts

sub print_context
{ # print_context

  my ($counts, $counts_pos, $max_con_len, $max_con_pos_len) = @_ ;
  my (@v_con, @v_con_pos, $con, $con_pos, $i, $n) ;

  printf OUT "  %-*s | %-4s | %-4s | %-4s | %-4s", $max_con_pos_len, 'PPOSS', 'any', 'head', 'dep', 'both' ;
  printf OUT "  ||" ;
  printf OUT "  %-*s | %-4s | %-4s | %-4s | %-4s", $max_con_len, 'word', 'any', 'head', 'dep', 'both' ;
  printf OUT "\n" ;
  printf OUT "  %s-+------+------+------+-----", '-' x $max_con_pos_len;
  printf OUT "--++" ;
  printf OUT "--%s-+------+------+------+-----", '-' x $max_con_len;
  printf OUT "\n" ;

  @v_con = sort {${$counts}{tot}{$b} <=> ${$counts}{tot}{$a}} keys %{${$counts}{tot}} ;
  @v_con_pos = sort {${$counts_pos}{tot}{$b} <=> ${$counts_pos}{tot}{$a}} keys %{${$counts_pos}{tot}} ;

  $n = scalar @v_con ;
  if (scalar @v_con_pos > $n)
  {
    $n = scalar @v_con_pos ;
  }

  foreach $i (0 .. $n-1)
  {
    if (defined $v_con_pos[$i])
    {
      $con_pos = $v_con_pos[$i] ;
      printf OUT "  %-*s | %4d | %4d | %4d | %4d",
	$max_con_pos_len, $con_pos, ${$counts_pos}{tot}{$con_pos},
	  ${$counts_pos}{err_head}{$con_pos}, ${$counts_pos}{err_dep}{$con_pos},
	    ${$counts_pos}{err_dep}{$con_pos}+${$counts_pos}{err_head}{$con_pos}-${$counts_pos}{tot}{$con_pos} ;
    }
    else
    {
      printf OUT "  %-*s | %4s | %4s | %4s | %4s",
	$max_con_pos_len, ' ', ' ', ' ', ' ', ' ' ;
    }

    printf OUT "  ||" ;

    if (defined $v_con[$i])
    {
      $con = $v_con[$i] ;
      printf OUT "  %-*s | %4d | %4d | %4d | %4d",
	$max_con_len+length($con)-uni_len($con), $con, ${$counts}{tot}{$con},
	  ${$counts}{err_head}{$con}, ${$counts}{err_dep}{$con},
	    ${$counts}{err_dep}{$con}+${$counts}{err_head}{$con}-${$counts}{tot}{$con} ;
    }
    else
    {
      printf OUT "  %-*s | %4s | %4s | %4s | %4s",
	$max_con_len, ' ', ' ', ' ', ' ', ' ' ;
    }

    printf OUT "\n" ;
  }

  printf OUT "  %s-+------+------+------+-----", '-' x $max_con_pos_len;
  printf OUT "--++" ;
  printf OUT "--%s-+------+------+------+-----", '-' x $max_con_len;
  printf OUT "\n" ;

  printf OUT "\n\n" ;

} # print_context

sub num_as_word
{
  my ($num) = @_ ;

  $num = abs($num) ;

  if ($num == 1)
  {
    return ('one word') ;
  }
  elsif ($num == 2)
  {
    return ('two words') ;
  }
  elsif ($num == 3)
  {
    return ('three words') ;
  }
  elsif ($num == 4)
  {
    return ('four words') ;
  }
  else
  {
    return ($num.' words') ;
  }
}

sub describe_err
{ # describe_err

  my ($head_err, $head_aft_bef, $dep_err) = @_ ;
  my ($dep_g, $dep_s, $desc) ;
  my ($head_aft_bef_g, $head_aft_bef_s) = split(//, $head_aft_bef) ;

  if ($head_err eq '-')
  {
    $desc = 'correct head' ;

    if ($head_aft_bef_s eq '0')
    {
      $desc .= ' (0)' ;
    }
    elsif ($head_aft_bef_s eq 'e')
    {
      $desc .= ' (the focus word)' ;
    }
    elsif ($head_aft_bef_s eq 'a')
    {
      $desc .= ' (after the focus word)' ;
    }
    elsif ($head_aft_bef_s eq 'b')
    {
      $desc .= ' (before the focus word)' ;
    }
  }
  elsif ($head_aft_bef_s eq '0')
  {
    $desc = 'head = 0 instead of ' ;
    if ($head_aft_bef_g eq 'a')
    {
      $desc.= 'after ' ;
    }
    if ($head_aft_bef_g eq 'b')
    {
      $desc.= 'before ' ;
    }
    $desc .= 'the focus word' ;
  }
  elsif ($head_aft_bef_g eq '0')
  {
    $desc = 'head is ' ;
    if ($head_aft_bef_g eq 'a')
    {
      $desc.= 'after ' ;
    }
    if ($head_aft_bef_g eq 'b')
    {
      $desc.= 'before ' ;
    }
    $desc .= 'the focus word instead of 0' ;
  }
  else
  {
    $desc = num_as_word($head_err) ;
    if ($head_err < 0)
    {
      $desc .= ' before' ;
    }
    else
    {
      $desc .= ' after' ;
    }

    $desc = 'head '.$desc.' the correct head ' ;

    if ($head_aft_bef_s eq '0')
    {
      $desc .= '(0' ;
    }
    elsif ($head_aft_bef_s eq 'e')
    {
      $desc .= '(the focus word' ;
    }
    elsif ($head_aft_bef_s eq 'a')
    {
      $desc .= '(after the focus word' ;
    }
    elsif ($head_aft_bef_s eq 'b')
    {
      $desc .= '(before the focus word' ;
    }

    if ($head_aft_bef_g ne $head_aft_bef_s)
    {
      $desc .= ' instead of' ;
      if ($head_aft_bef_s eq '0')
      {
	$desc .= '0' ;
      }
      elsif ($head_aft_bef_s eq 'e')
      {
	$desc .= 'the focus word' ;
      }
      elsif ($head_aft_bef_s eq 'a')
      {
	$desc .= 'after the focus word' ;
      }
      elsif ($head_aft_bef_s eq 'b')
      {
	$desc .= 'before the focus word' ;
      }
    }

    $desc .= ')' ;
  }

  $desc .= ', ' ;

  if ($dep_err eq '-')
  {
    $desc .= 'correct dependency' ;
  }
  else
  {
    ($dep_g, $dep_s) = ($dep_err =~ /^(.*)->(.*)$/) ;
    $desc .= sprintf('dependency "%s" instead of "%s"', $dep_s, $dep_g) ;
  }

  return($desc) ;

} # describe_err

sub get_context
{ # get_context

  my ($sent, $i_w) = @_ ;
  my ($w_2, $w_1, $w1, $w2) ;
  my ($p_2, $p_1, $p1, $p2) ;

  if ($i_w >= 2)
  {
    $w_2 = ${${$sent}[$i_w-2]}{word} ;
    $p_2 = ${${$sent}[$i_w-2]}{pos} ;
  }
  else
  {
    $w_2 = $START ;
    $p_2 = $START ;
  }

  if ($i_w >= 1)
  {
    $w_1 = ${${$sent}[$i_w-1]}{word} ;
    $p_1 = ${${$sent}[$i_w-1]}{pos} ;
  }
  else
  {
    $w_1 = $START ;
    $p_1 = $START ;
  }

  if ($i_w <= scalar @{$sent}-2)
  {
    $w1 = ${${$sent}[$i_w+1]}{word} ;
    $p1 = ${${$sent}[$i_w+1]}{pos} ;
  }
  else
  {
    $w1 = $END ;
    $p1 = $END ;
  }

  if ($i_w <= scalar @{$sent}-3)
  {
    $w2 = ${${$sent}[$i_w+2]}{word} ;
    $p2 = ${${$sent}[$i_w+2]}{pos} ;
  }
  else
  {
    $w2 = $END ;
    $p2 = $END ;
  }

  return ($w_2, $w_1, $w1, $w2, $p_2, $p_1, $p1, $p2) ;

} # get_context

#
# Parses the SRL frames from an input sentence
#
sub parse_frames
{
    my $is_gold = shift;
    my ($frame_lines) = @_;
    #print @{$frame_lines};

    # this will store all the SRL props in this sentence
    my $sent = SRL::sentence->new();

    #
    # discover predicates first
    #
    my $pred_count = 0;
    for(my $i = 0; $i < $#{$frame_lines}; $i ++){
	my $line = ${$frame_lines}[$i];
	my @tokens = split /\s+/, $line;
	#print "@tokens\n";
	# predicates are stored at column 10!
	my $lemma_sense = $tokens[10];
	my $pred_pposs = $tokens[7];
	if($lemma_sense ne "_"){
	    my ($lemma, $sense) = split /\./, $lemma_sense;
	    # arguments are stored starting at column 11
            my $prop = SRL::prop->new($lemma, $sense, $pred_pposs, $i, $pred_count + 11);
            #printf STDERR "%s %d %d\n", $prop->lemma(), $prop->sense(), $prop->column();
            
            # skip SU props if $opt_u is not defined
            if($sense eq "SU"){
            	if(defined $opt_u){
            	  $sent->add_props($prop);
            	} else {
            	  if($is_gold == 1){
            	    $skipped_su_preds_gold ++;
            	  } else {
            	    $skipped_su_preds_sys ++;
            	  }
            	}
            } else {
            	$sent->add_props($prop);
            }
            $pred_count ++;
	}
    }
    
    #
    # read the args for each prop
    #
    foreach my $prop ($sent->props()){
    	#printf STDERR "%s %d %d\n", $prop->lemma(), $prop->sense(), $prop->column();
    	for(my $i = 0; $i < $#{$frame_lines}; $i ++){
    	    my $line = ${$frame_lines}[$i];
    	    my @tokens = split /\s+/, $line;
    	    my $label = $tokens[$prop->column()];
    	    if($label ne "_"){
    	    	my $arg = SRL::arg->new($label, $i);
    	    
    	    	# skip SU args if $opt_u is not defined
    	    	if($label eq "SU"){
    	    	  if(defined $opt_u){
    	    	    $prop->add_args($arg);
    	    	  } else {
    	    	    if($is_gold == 1){
    	    	      $skipped_su_args_gold ++;
    	    	    } else {
    	    	      $skipped_su_args_sys ++
    	    	    }
    	    	  }
    	    	} else {
    	    	  #printf STDERR "\tArg %s %d\n", $arg->label(), $arg->position();
    	    	  $prop->add_args($arg);
    	    	}
    	    }
    	}
    	#$prop->display();
    }
    
    return ($sent);
} # parse_frames 

#
# Verifies if two propositions are equal,
#   i.e., their full argument sets must be identical
#
sub same_prop
{
    my ($gold_prop, $sys_prop) = @_;
    my $found = 0;

    # must have the same number of args
    if($gold_prop->arg_count() != $sys_prop->arg_count()){
	return 0;
    }

    # each gold arg must be matched in sys
    foreach my $gold_arg ($gold_prop->args()) {
	$found = 0;
	foreach my $sys_arg ($sys_prop->args()){
	    if($gold_arg->label() eq $sys_arg->label() and
	       $gold_arg->position() == $sys_arg->position()){
		$found = 1;
		last if $found == 1;
	    }
	}
	if($found == 0){
	    return 0;
	}
    }

    return 1;
}


#
# Updates the SRL score counts for one sentence
#
sub update_srl_scores
{
    my ($gold_sent, $sys_sent, $counts) = @_;

    my %sent_counts = (
	tot_prop => 0,
	pred_prop => 0,
	corl_prop => 0,
	tot_arg => 0,
	pred_arg => 0,
	corl_arg => 0
	);

    #printf STDERR "GOLD SRL: ";
    #$gold_sent->display();
    #printf STDERR "\nSYS SRL: ";
    #$sys_sent->display();
    
    # compute P/R for predicting predicates and predicate senses
    # we model these as dependencies to a virtual ROOT node;
    #   the labels of these dependencies are the predicate senses
    #
    $counts->{tot_prop} += $gold_sent->prop_count();
    $sent_counts{tot_prop} += $gold_sent->prop_count();
    $counts->{pred_prop} += $sys_sent->prop_count();
    $sent_counts{pred_prop} += $sys_sent->prop_count();
    foreach my $gold_prop ($gold_sent->props()) {
	$counts->{tot_prop_per_tag}{$gold_prop->pposs()} ++;
	$counts->{prop_per_tag}{$gold_prop->pposs()} = 1;
    }
    foreach my $sys_prop ($sys_sent->props()){
	$counts->{pred_prop_per_tag}{$sys_prop->pposs()} ++;
	$counts->{prop_per_tag}{$sys_prop->pposs()} = 1;
    }
    foreach my $gold_prop ($gold_sent->props()) {
    	foreach my $sys_prop ($sys_sent->props()){
    	    if($gold_prop->position() == $sys_prop->position()){
    	    	$counts->{coru_prop} ++;
		$counts->{coru_prop_per_tag}{$gold_prop->pposs()} ++;
    	    	#if($gold_prop->sense() == $sys_prop->sense()){
    	    	if($gold_prop->sense() eq $sys_prop->sense()){
    	    	    $counts->{corl_prop} ++;
    	    	    $sent_counts{corl_prop} ++;
		    $counts->{corl_prop_per_tag}{$gold_prop->pposs()} ++;
		    if(same_prop($gold_prop, $sys_prop)){
			$counts->{full_corl_prop} ++;
		    }
    	    	}
    	    	last if $gold_prop->position() == $sys_prop->position();
    	    }
    	}
    }
    
    #
    # compute P/R for semantic dependencies, both labeled and unlabeled
    # An unlabeled dependency is considered correct if 
    #   the token positions for predicate and argument are correct
    # For a labeled dependency to be correct, 
    #   the argument label must also be correct
    # Note: the predicate sense is scored in the block above,
    #   in the dependencies to the virtual ROOT node
    #
    $counts->{tot_arg} += $gold_sent->arg_count();
    $sent_counts{tot_arg} += $gold_sent->arg_count();
    $counts->{pred_arg} += $sys_sent->arg_count();
    $sent_counts{pred_arg} += $sys_sent->arg_count();
    foreach my $gold_prop ($gold_sent->props()) {
	foreach my $gold_arg ($gold_prop->args()){
	    my $label = 
		substr($gold_prop->pposs(), 0, 2) . "* + " . $gold_arg->label();
	    $counts->{tot_arg_per_tag}{$label} ++;
	    $counts->{arg_per_tag}{$label} = 1;
	}
    }
    foreach my $sys_prop ($sys_sent->props()){
	foreach my $sys_arg ($sys_prop->args()){
	    my $label = 
		substr($sys_prop->pposs(), 0, 2) . "* + " . $sys_arg->label();
	    $counts->{pred_arg_per_tag}{$label} ++;
	    $counts->{arg_per_tag}{$label} = 1;
	}
    }
    foreach my $gold_prop ($gold_sent->props()) {
    	foreach my $sys_prop ($sys_sent->props()){
    	    # found two identical predicate positions
    	    if($gold_prop->position() == $sys_prop->position()){
    	    	# scan all the arguments for the gold prop
    	    	foreach my $gold_arg ($gold_prop->args()){
    	    	    foreach my $sys_arg ($sys_prop->args()){
    	    	    	# found a correct arg for unlabeled scoring
    	    	    	if($gold_arg->position() == $sys_arg->position()){
			    my $label = substr($gold_prop->pposs(), 0, 2) . 
				"* + " . $gold_arg->label();
    	    	    	    $counts->{coru_arg} ++;
			    $counts->{coru_arg_per_tag}{$label} ++;
    	    	    	    # found a correct arg for labeled scoring
    	    	    	    if($gold_arg->label() eq $sys_arg->label()){
    	    	    	        $counts->{corl_arg} ++;
    	    	    	        $sent_counts{corl_arg} ++;
				$counts->{corl_arg_per_tag}{$label} ++;
    	    	    	    }
    	    	    	}
    	    	        last if $gold_arg->position() == $sys_arg->position();
    	    	    }
    	    	}
    	    }
    	}
    }
    $counts->{tot_sent} ++;
    if ($sent_counts{tot_prop} == $sent_counts{corl_prop} &&
	$sent_counts{pred_prop} == $sent_counts{corl_prop} &&
	$sent_counts{tot_arg} == $sent_counts{corl_arg} &&
	$sent_counts{pred_arg} == $sent_counts{corl_arg}) {
	$counts->{cor_sent} ++;
    }

    #printf STDERR "%d %d %d %d\n", $counts->{coru_arg}, $counts->{corl_arg}, $counts->{tot_arg}, $counts->{pred_arg};
} # end update_srl_scores

sub read_sent
{ # read_sent

  my ($sent_gold, $sent_sys) = @_ ;
  my ($line_g, $line_s, $new_sent) ;
  my (%fields_g, %fields_s) ;
  my ($gold_lines, $sys_lines); # all the lines for the current sentence
  my ($gold_srl_sent, $sys_srl_sent); # the SRL::sentence objects for this sentence

  $new_sent = 1 ;

  @{$sent_gold} = () ;
  @{$sent_sys} = () ;

  @{$gold_lines} = ();
  @{$sys_lines} = ();

  while (1)
  { # main reading loop

    $line_g = <GOLD> ;
    $line_s = <SYS> ;

    if(defined $line_g) {
	push @{$gold_lines}, $line_g;
    }
    if(defined $line_s) {
	push @{$sys_lines}, $line_s;
    }

    $line_num++ ;

    # system output has fewer lines than gold standard
    if ((defined $line_g) && (! defined $line_s))
    {
	if ($line_g =~ /^\s*$/) {
	    warn "Warning: ignoring missing blank line at the end of $opt_s.\n";
	    next;
	}
	printf STDERR "Fatal: line mismatch, line %d:\n", $line_num ;
	printf STDERR " gold: %s", $line_g ;
	printf STDERR " sys : past end of file\n" ;
	exit(1) ;
    }

    # system output has more lines than gold standard
    if ((! defined $line_g) && (defined $line_s))
    {
	if ($line_s =~ /^\s*$/) {
	    warn "Warning: ignoring extra blank line at the end of $opt_s.\n";
	    next;
	}
	printf STDERR "Fatal: line mismatch, line %d:\n", $line_num ;
	printf STDERR " gold: past end of file\n" ;
	printf STDERR " sys : %s", $line_s ;
	exit(1) ;
    }
    
    # end of file reached for both
    if ((! defined $line_g) && (! defined $line_s))
    {
    	$gold_srl_sent = parse_frames(1, $gold_lines);
	$sys_srl_sent = parse_frames(0, $sys_lines);
	return (1, $gold_srl_sent, $sys_srl_sent) ;
    }

    # one contains end of sentence but other one does not
    if (($line_g =~ /^\s+$/) != ($line_s =~ /^\s+$/))
    {
      printf STDERR "Fatal: line mismatch, line %d:\n", $line_num ;
      printf STDERR " gold: %s", $line_g ;
      printf STDERR " sys : %s", $line_s ;
      exit(1) ;
    }

    # end of sentence reached
    if ($line_g =~ /^\s+$/)
    {
	$gold_srl_sent = parse_frames(1, $gold_lines);
	$sys_srl_sent = parse_frames(0, $sys_lines);
	return(0, $gold_srl_sent, $sys_srl_sent) ;
    }

    # now both lines contain information

    if ($new_sent)
    {
      $new_sent = 0 ;
    }

    # 'official' column names
    # options.output = ['id','form','lemma','gpos','ppos',
    #                   'split_form', 'split_lemma', 'pposs', 
    #                   'head', 'deprel', 'pred', 'arg'+]

    @fields_g{'word', 'pos', 'head', 'dep'} = (split (/\s+/, $line_g))[5, 7, 8, 9] ;

    push @{$sent_gold}, { %fields_g } ;

    @fields_s{'word', 'pos', 'head', 'dep'} = (split (/\s+/, $line_s))[5, 7, 8, 9] ;

# Some teams like to change the word or the pos in the answer file...
# So do not make this fatal and only give one warning.

    if ((not defined $word_mismatch_warning) &&
	(($fields_g{word} ne $fields_s{word}) ||
	 ($fields_g{pos} ne $fields_s{pos})))
    {
	$word_mismatch_warning = 1;
	printf STDERR "Warning: ignoring word/pos mismatch, line %d:\n", $line_num ;
	printf STDERR " gold: %s", $line_g ;
	printf STDERR " sys : %s", $line_s ;
	# exit(1) ;
    }

    push @{$sent_sys}, { %fields_s } ;

  } # main reading loop
  
} # read_sent

################################################################################
###                                  main                                    ###
################################################################################

my ($sent_num, $eof, $word_num, @err_sent) ;
my (@sent_gold, @sent_sys, @starts) ;
my ($gold_srl_sent, $sys_srl_sent);
my ($word, $pos, $wp, $head_g, $dep_g, $head_s, $dep_s) ;
my (%counts, $err_head, $err_dep, $con, $con1, $con_pos, $con_pos1, $thresh) ;
my ($head_err, $dep_err, @cur_err, %err_counts, $err_counter, $err_desc) ;
my ($loc_con, %loc_con_err_counts, %err_desc) ;
my ($head_aft_bef_g, $head_aft_bef_s, $head_aft_bef) ;
my ($con_bef, $con_aft, $con_bef_2, $con_aft_2, @bits, @e_bits, @v_con, @v_con_pos) ;
my ($con_pos_bef, $con_pos_aft, $con_pos_bef_2, $con_pos_aft_2) ;
my ($max_word_len, $max_pos_len, $max_con_len, $max_con_pos_len) ;
my ($max_word_spec_len, $max_con_bef_len, $max_con_aft_len) ;
my (%freq_err, $err) ;

my %srl_counts; # counts for SRL scoring

my ($i, $j, $i_w, $l, $n_args) ;
my ($w_2, $w_1, $w1, $w2) ;
my ($wp_2, $wp_1, $wp1, $wp2) ;
my ($p_2, $p_1, $p1, $p2) ;

my ($short_output) ;
my ($score_on_punct, $score_on_deriv) ;
$counts{punct} = 0; # initialize
$counts{deriv} = 0;

getopts("g:o:s:qvhpubd") ;

if (defined $opt_v)
{
    my $id = '$Id: eval08.pl,v 1.8 2008/10/22 15:00:00 mihais Exp $';
    my @parts = split ' ',$id;
    print "Version $parts[2]\n";
    exit(0);
}

if ((defined $opt_h) || ((! defined $opt_g) && (! defined $opt_s)))
{
  die $usage ;
}

if (! defined $opt_g)
{
  die "Gold standard file (-g) missing\n" ;
}

if (! defined $opt_s)
{
  die "System output file (-s) missing\n" ;
}

if (! defined $opt_o)
{
  $opt_o = '-' ;
}

if (defined $opt_q)
{
    $short_output = 1 ;
} else {
    $short_output = 0 ;
}

if (defined $opt_p)
{
    $score_on_punct = 0 ;
} else {
    $score_on_punct = 1 ;
}

#
# Removed. The DERIV links only exist in the Turkish dependency treebank 
# and are therefore not relevant this year.
#
#if (defined $opt_d)
#{
#    $score_on_deriv = 0 ;
#} else {
#    $score_on_deriv = 1 ;
#}
$score_on_deriv = 1 ;

$line_num = 0 ;
$sent_num = 0 ;
$eof = 0 ;

@err_sent = () ;
@starts = () ;

%{$err_sent[0]} = () ;

$max_pos_len = length('PPOSS') ;

################################################################################
###                              reading input                               ###
################################################################################

open (GOLD, "<$opt_g") || die "Could not open gold standard file $opt_g\n" ;
open (SYS,  "<$opt_s") || die "Could not open system output file $opt_s\n" ;
open (OUT,  ">$opt_o") || die "Could not open output file $opt_o\n" ;


if (defined $opt_b) {  # produce output similar to evalb
    print OUT "     Sent.          Attachment      Correct        Scoring          \n";
    print OUT "    ID Tokens  -   Unlab. Lab.   HEAD HEAD+DEPREL   tokens   - - - -\n";
    print OUT "  ============================================================================\n";
}

%srl_counts = (
	tot_prop => 0,
	pred_prop => 0,
	corl_prop => 0,
	coru_prop => 0,
        full_corl_prop => 0,
        tot_sent => 0,
        cor_sent => 0
);

while (! $eof)
{ # main reading loop

  $starts[$sent_num] = $line_num+1 ;
  ($eof, $gold_srl_sent, $sys_srl_sent) = read_sent(\@sent_gold, \@sent_sys) ;
  my $old_srl_cor_sent = $srl_counts{cor_sent};
  update_srl_scores($gold_srl_sent, $sys_srl_sent, \%srl_counts);
  my $srl_sent_cor = $srl_counts{cor_sent} - $old_srl_cor_sent;

  $sent_num++ ;

  %{$err_sent[$sent_num]} = () ;
  $word_num = scalar @sent_gold ;
  # bug fix by James Henderson: 
  # if the files has extra blank lines, 
  # then they were counted as correct sentences in the ExactMatch score.
  # solution: skip empty sentences
  if ($word_num == 0)
  {
      next;
  } 

  # for accuracy per sentence
  my %sent_counts = ( tot      => 0,
		      err_any  => 0,
		      err_head => 0
		      ); 

  # printf "$sent_num $word_num\n" ;

  my @frames_g = ('** '); # the initial frame for the virtual root
  my @frames_s = ('** '); # the initial frame for the virtual root
  foreach $i_w (0 .. $word_num-1)
  { # loop on words
      push @frames_g, ''; # initialize
      push @frames_s, ''; # initialize
  }

  foreach $i_w (0 .. $word_num-1)
  { # loop on words

    ($word, $pos, $head_g, $dep_g)
      = @{$sent_gold[$i_w]}{'word', 'pos', 'head', 'dep'} ;
    $wp = $word.' / '.$pos ;

    # printf "%d: %s %s %s %s\n", $i_w,  $word, $pos, $head_g, $dep_g ;

    if ((! $score_on_punct) && is_uni_punct($word))
    {
      $counts{punct}++ ;
      # ignore punctuations
      next ;
    }

    if ((! $score_on_deriv) && ($dep_g eq 'DERIV'))
    {
      $counts{deriv}++ ;
      # ignore deriv links
      next ;
    }

    if (length($pos) > $max_pos_len)
    {
      $max_pos_len = length($pos) ;
    }

    ($head_s, $dep_s) = @{$sent_sys[$i_w]}{'head', 'dep'} ;

    $counts{tot}++ ;
    $counts{word}{$wp}{tot}++ ;
    $counts{pos}{$pos}{tot}++ ;
    $counts{head}{$head_g-$i_w-1}{tot}++ ;

    # for frame confusions
    # add child to frame of parent
    $frames_g[$head_g] .= "$dep_g ";
    $frames_s[$head_s] .= "$dep_s ";
    # add to frame of token itself
    $frames_g[$i_w+1] .= "*$dep_g* "; # $i_w+1 because $i_w starts counting at zero
    $frames_s[$i_w+1] .= "*$dep_g* ";

    # for precision and recall of DEPREL
    $counts{dep}{$dep_g}{tot}++ ;     # counts for gold standard deprels
    $counts{dep2}{$dep_g}{$dep_s}++ ; # counts for confusions
    $counts{dep_s}{$dep_s}{tot}++ ;   # counts for system deprels
    $counts{all_dep}{$dep_g} = 1 ;    # list of all deprels that occur ...
    $counts{all_dep}{$dep_s} = 1 ;    # ... in either gold or system output

    # for precision and recall of HEAD direction
    my $dir_g;
    if ($head_g == 0) {
	$dir_g = 'to_root';
    } elsif ($head_g < $i_w+1) { # $i_w+1 because $i_w starts counting at zero
                                 # also below
	$dir_g = 'left';
    } elsif ($head_g > $i_w+1) {
	$dir_g = 'right';
    } else {
        # token links to itself; should never happen in correct gold standard
	$dir_g = 'self'; 
    }
    my $dir_s;
    if ($head_s == 0) {
	$dir_s = 'to_root';
    } elsif ($head_s < $i_w+1) {
	$dir_s = 'left';
    } elsif ($head_s > $i_w+1) {
	$dir_s = 'right';
    } else {
        # token links to itself; should not happen in good system 
        # (but not forbidden in shared task)
	$dir_s = 'self'; 
    }
    $counts{dir_g}{$dir_g}{tot}++ ;   # counts for gold standard head direction
    $counts{dir2}{$dir_g}{$dir_s}++ ; # counts for confusions
    $counts{dir_s}{$dir_s}{tot}++ ;   # counts for system head direction

    # for precision and recall of HEAD distance
    my $dist_g;
    if ($head_g == 0) {
	$dist_g = 'to_root';
    } elsif ( abs($head_g - ($i_w+1)) <= 1 ) {
	$dist_g = '1'; # includes the 'self' cases
    } elsif ( abs($head_g - ($i_w+1)) <= 2 ) {
	$dist_g = '2';
    } elsif ( abs($head_g - ($i_w+1)) <= 6 ) {
	$dist_g = '3-6';
    } else {
	$dist_g = '7-...';
    }
    my $dist_s;
    if ($head_s == 0) {
	$dist_s = 'to_root';
    } elsif ( abs($head_s - ($i_w+1)) <= 1 ) {
	$dist_s = '1'; # includes the 'self' cases
    } elsif ( abs($head_s - ($i_w+1)) <= 2 ) {
	$dist_s = '2';
    } elsif ( abs($head_s - ($i_w+1)) <= 6 ) {
	$dist_s = '3-6';
    } else {
	$dist_s = '7-...';
    }
    $counts{dist_g}{$dist_g}{tot}++ ;    # counts for gold standard head distance
    $counts{dist2}{$dist_g}{$dist_s}++ ; # counts for confusions
    $counts{dist_s}{$dist_s}{tot}++ ;    # counts for system head distance


    $err_head = ($head_g ne $head_s) ; # error in head
    $err_dep = ($dep_g ne $dep_s) ;    # error in deprel

    $head_err = '-' ;
    $dep_err = '-' ;

    # for accuracy per sentence
    $sent_counts{tot}++ ;
    if ($err_dep || $err_head) {
	$sent_counts{err_any}++ ;
    }
    if ($err_head) {
	$sent_counts{err_head}++ ;
    }

    # total counts and counts for PPOSS involved in errors

    if ($head_g eq '0')
    {
      $head_aft_bef_g = '0' ;
    }
    elsif ($head_g eq $i_w+1)
    {
      $head_aft_bef_g = 'e' ;
    }
    else
    {
      $head_aft_bef_g = ($head_g <= $i_w+1 ? 'b' : 'a') ;
    }

    if ($head_s eq '0')
    {
      $head_aft_bef_s = '0' ;
    }
    elsif ($head_s eq $i_w+1)
    {
      $head_aft_bef_s = 'e' ;
    }
    else
    {
      $head_aft_bef_s = ($head_s <= $i_w+1 ? 'b' : 'a') ;
    }

    $head_aft_bef = $head_aft_bef_g.$head_aft_bef_s ;

    if ($err_head)
    {
      if ($head_aft_bef_s eq '0')
      {
	$head_err = 0 ;
      }
      else
      {
	$head_err = $head_s-$head_g ;
      }

      $err_sent[$sent_num]{head}++ ;
      $counts{err_head}{tot}++ ;
      $counts{err_head}{$head_err}++ ;

      $counts{word}{err_head}{$wp}++ ;
      $counts{pos}{$pos}{err_head}{tot}++ ;
      $counts{pos}{$pos}{err_head}{$head_err}++ ;
    }

    if ($err_dep)
    {
      $dep_err = $dep_g.'->'.$dep_s ;
      $err_sent[$sent_num]{dep}++ ;
      $counts{err_dep}{tot}++ ;
      $counts{err_dep}{$dep_err}++ ;

      $counts{word}{err_dep}{$wp}++ ;
      $counts{pos}{$pos}{err_dep}{tot}++ ;
      $counts{pos}{$pos}{err_dep}{$dep_err}++ ;

      if ($err_head)
      {
	$counts{err_both}++ ;
	$counts{pos}{$pos}{err_both}++ ;
      }
    }

    ### DEPREL + ATTACHMENT
    if ((!$err_dep) && ($err_head)) {
	$counts{err_head_corr_dep}{tot}++ ;
	$counts{err_head_corr_dep}{$dep_s}++ ;
    }
    ### DEPREL + ATTACHMENT

    # counts for words involved in errors

    if (! ($err_head || $err_dep))
    {
      next ;
    }

    $err_sent[$sent_num]{word}++ ;
    $counts{err_any}++ ;
    $counts{word}{err_any}{$wp}++ ;
    $counts{pos}{$pos}{err_any}++ ;

    ($w_2, $w_1, $w1, $w2, $p_2, $p_1, $p1, $p2) = get_context(\@sent_gold, $i_w) ;

    if ($w_2 ne $START)
    {
      $wp_2 = $w_2.' / '.$p_2 ;
    }
    else
    {
      $wp_2 = $w_2 ;
    }

    if ($w_1 ne $START)
    {
      $wp_1 = $w_1.' / '.$p_1 ;
    }
    else
    {
      $wp_1 = $w_1 ;
    }

    if ($w1 ne $END)
    {
      $wp1 = $w1.' / '.$p1 ;
    }
    else
    {
      $wp1 = $w1 ;
    }

    if ($w2 ne $END)
    {
      $wp2 = $w2.' / '.$p2 ;
    }
    else
    {
      $wp2 = $w2 ;
    }

    $con_bef = $wp_1 ;
    $con_bef_2 = $wp_2.' + '.$wp_1 ;
    $con_aft = $wp1 ;
    $con_aft_2 = $wp1.' + '.$wp2 ;

    $con_pos_bef = $p_1 ;
    $con_pos_bef_2 = $p_2.'+'.$p_1 ;
    $con_pos_aft = $p1 ;
    $con_pos_aft_2 = $p1.'+'.$p2 ;

    if ($w_1 ne $START)
    {
      # do not count '.S' as a word context
      $counts{con_bef_2}{tot}{$con_bef_2}++ ;
      $counts{con_bef_2}{err_head}{$con_bef_2} += $err_head ;
      $counts{con_bef_2}{err_dep}{$con_bef_2} += $err_dep ;
      $counts{con_bef}{tot}{$con_bef}++ ;
      $counts{con_bef}{err_head}{$con_bef} += $err_head ;
      $counts{con_bef}{err_dep}{$con_bef} += $err_dep ;
    }

    if ($w1 ne $END)
    {
      # do not count '.E' as a word context
      $counts{con_aft_2}{tot}{$con_aft_2}++ ;
      $counts{con_aft_2}{err_head}{$con_aft_2} += $err_head ;
      $counts{con_aft_2}{err_dep}{$con_aft_2} += $err_dep ;
      $counts{con_aft}{tot}{$con_aft}++ ;
      $counts{con_aft}{err_head}{$con_aft} += $err_head ;
      $counts{con_aft}{err_dep}{$con_aft} += $err_dep ;
    }

    $counts{con_pos_bef_2}{tot}{$con_pos_bef_2}++ ;
    $counts{con_pos_bef_2}{err_head}{$con_pos_bef_2} += $err_head ;
    $counts{con_pos_bef_2}{err_dep}{$con_pos_bef_2} += $err_dep ;
    $counts{con_pos_bef}{tot}{$con_pos_bef}++ ;
    $counts{con_pos_bef}{err_head}{$con_pos_bef} += $err_head ;
    $counts{con_pos_bef}{err_dep}{$con_pos_bef} += $err_dep ;

    $counts{con_pos_aft_2}{tot}{$con_pos_aft_2}++ ;
    $counts{con_pos_aft_2}{err_head}{$con_pos_aft_2} += $err_head ;
    $counts{con_pos_aft_2}{err_dep}{$con_pos_aft_2} += $err_dep ;
    $counts{con_pos_aft}{tot}{$con_pos_aft}++ ;
    $counts{con_pos_aft}{err_head}{$con_pos_aft} += $err_head ;
    $counts{con_pos_aft}{err_dep}{$con_pos_aft} += $err_dep ;

    $err = $head_err.$sep.$head_aft_bef.$sep.$dep_err ;
    $freq_err{$err}++ ;

  } # loop on words

  foreach $i_w (0 .. $word_num) # including one for the virtual root
  { # loop on words
      if ($frames_g[$i_w] ne $frames_s[$i_w]) {
	  $counts{frame2}{"$frames_g[$i_w]/ $frames_s[$i_w]"}++ ;
      }
  }

  $counts{tot_sent}++ ;
  if ($sent_counts{err_any} == 0) {
    $counts{cor_sent}++ ;
    if ($srl_sent_cor) {
	$counts{allcor_sent}++ ;
    }
  }
  if (defined $opt_b) { # produce output similar to evalb
      if ($word_num > 0) {
	  my ($unlabeled,$labeled) = ('NaN', 'NaN');
	  if ($sent_counts{tot} > 0) { # there are scoring tokens
	      $unlabeled = 100-$sent_counts{err_head}*100.0/$sent_counts{tot};
	      $labeled   = 100-$sent_counts{err_any} *100.0/$sent_counts{tot};
	  }
	  printf OUT "  %4d %4d    0  %6.2f %6.2f  %4d    %4d        %4d    0 0 0 0\n", 
	  $sent_num, $word_num, 
	  $unlabeled, $labeled, 
	  $sent_counts{tot}-$sent_counts{err_head}, 
	  $sent_counts{tot}-$sent_counts{err_any}, 
	  $sent_counts{tot},;
      }
  }

} # main reading loop

################################################################################
###                             printing output                              ###
################################################################################

if (defined $opt_b) {  # produce output similar to evalb
    print OUT "\n\n";
}

#
# The overall score
#

#
# The syntactic scores
#
printf OUT "  SYNTACTIC SCORES:\n";
my $lab_attach = 100 - $counts{err_any}*100.0/$counts{tot};
printf OUT "  Labeled   attachment score: %d / %d * 100 = %.2f %%\n", 
    $counts{tot}-$counts{err_any}, $counts{tot}, $lab_attach;
my $unlab_attach = 100 - $counts{err_head}{tot}*100.0/$counts{tot} ;
printf OUT "  Unlabeled attachment score: %d / %d * 100 = %.2f %%\n", 
    $counts{tot}-$counts{err_head}{tot}, $counts{tot}, $unlab_attach;
my $lab_acc = 100 - $counts{err_dep}{tot}*100.0/$counts{tot} ;
printf OUT "  Label accuracy score:       %d / %d * 100 = %.2f %%\n", 
    $counts{tot}-$counts{err_dep}{tot}, $counts{tot}, $lab_acc;

my $emdep = $counts{cor_sent} * 100 / $counts{tot_sent} ;
printf OUT "  Exact syntactic match:      %d / %d * 100 = %.2f %%\n", 
    $counts{cor_sent}, $counts{tot_sent}, $emdep;

#
# The semantic scores
#
printf OUT "\n  SEMANTIC SCORES: \n";
my $apl = 
    ($srl_counts{corl_arg} + $srl_counts{corl_prop}) * 100 / 
    ($srl_counts{pred_arg} + $srl_counts{pred_prop});
my $arl = 
    ($srl_counts{corl_arg} + $srl_counts{corl_prop}) * 100 / 
    ($srl_counts{tot_arg} + $srl_counts{tot_prop});
my $afl = 2 * $apl * $arl / ($apl + $arl);
printf OUT "  Labeled precision:          (%d + %d) / (%d + %d) * 100 = %.2f %%\n",
    $srl_counts{corl_arg}, 
    $srl_counts{corl_prop},
    $srl_counts{pred_arg}, 
    $srl_counts{pred_prop},
    $apl;
printf OUT "  Labeled recall:             (%d + %d) / (%d + %d) * 100 = %.2f %%\n",
    $srl_counts{corl_arg}, 
    $srl_counts{corl_prop}, 
    $srl_counts{tot_arg}, 
    $srl_counts{tot_prop}, 
    $arl;
printf OUT "  Labeled F1:                 %.2f \n", $afl;

my $apu = 
    ($srl_counts{coru_arg} + $srl_counts{coru_prop}) * 100 / 
    ($srl_counts{pred_arg} + $srl_counts{pred_prop});
my $aru = 
    ($srl_counts{coru_arg} + $srl_counts{coru_prop}) * 100 / 
    ($srl_counts{tot_arg} + $srl_counts{tot_prop});
my $afu = 2 * $apu * $aru / ($apu + $aru);
printf OUT "  Unlabeled precision:        (%d + %d) / (%d + %d) * 100 = %.2f %%\n",
    $srl_counts{coru_arg}, 
    $srl_counts{coru_prop}, 
    $srl_counts{pred_arg}, 
    $srl_counts{pred_prop}, 
    $apu;
printf OUT "  Unlabeled recall:           (%d + %d) / (%d + %d) * 100 = %.2f %%\n",
    $srl_counts{coru_arg}, 
    $srl_counts{coru_prop}, 
    $srl_counts{tot_arg}, 
    $srl_counts{tot_prop}, 
    $aru;
printf OUT "  Unlabeled F1:               %.2f \n", $afu;

my $ppl = 
    $srl_counts{full_corl_prop} * 100 / 
    $srl_counts{pred_prop};
my $prl = 
    $srl_counts{full_corl_prop} * 100 / 
    $srl_counts{tot_prop};
my $pfl = 0;
if($ppl != 0 and $prl != 0){
    $pfl = 2 * $ppl * $prl / ($ppl + $prl);
}
printf OUT "  Proposition precision:      %d / %d * 100 = %.2f %%\n",
    $srl_counts{full_corl_prop}, 
    $srl_counts{pred_prop}, 
    $ppl;
printf OUT "  Proposition recall:         %d / %d * 100 = %.2f %%\n",
    $srl_counts{full_corl_prop}, 
    $srl_counts{tot_prop}, 
    $prl;
printf OUT "  Proposition F1:             %.2f \n", $pfl;

my $emsrl = $srl_counts{cor_sent} * 100 / $srl_counts{tot_sent};
printf OUT "  Exact semantic match:       %d / %d * 100 = %.2f %%\n",
    $srl_counts{cor_sent}, 
    $srl_counts{tot_sent}, 
    $emsrl;

#
# The overall scores, combined at MACRO level
#
my $sem_weight = 0.5;
printf OUT "\n  OVERALL MACRO SCORES (Wsem = %.2f):\n", $sem_weight;
# this is the weight given to the semantic task

my $lab_macro_prec = $sem_weight * $apl + (1 - $sem_weight) * $lab_attach;
my $lab_macro_rec = $sem_weight * $arl + (1 - $sem_weight) * $lab_attach;
my $lab_macro_f1 = 
    2 * $lab_macro_prec * $lab_macro_rec / 
    ($lab_macro_prec + $lab_macro_rec);
printf OUT "  Labeled macro precision:    %.2f %%\n", $lab_macro_prec;
printf OUT "  Labeled macro recall:       %.2f %%\n", $lab_macro_rec;
printf OUT "  Labeled macro F1:           %.2f %%\n", $lab_macro_f1;

my $unlab_macro_prec = $sem_weight * $apu + (1 - $sem_weight) * $unlab_attach;
my $unlab_macro_rec = $sem_weight * $aru + (1 - $sem_weight) * $unlab_attach;
my $unlab_macro_f1 = 
    2 * $unlab_macro_prec * $unlab_macro_rec / 
    ($unlab_macro_prec + $unlab_macro_rec);
printf OUT "  Unlabeled macro precision:  %.2f %%\n", $unlab_macro_prec;
printf OUT "  Unlabeled macro recall:     %.2f %%\n", $unlab_macro_rec;
printf OUT "  Unlabeled macro F1:         %.2f %%\n", $unlab_macro_f1;

my $emall = $counts{allcor_sent} * 100 / $counts{tot_sent} ;
printf OUT "  Exact overall match:        %d / %d * 100 = %.2f %%\n", 
    $counts{allcor_sent}, $counts{tot_sent}, $emall;

#
# The overall scores, combined at MICRO level
#
printf OUT "\n  OVERALL MICRO SCORES:\n";
my $lab_micro_corr = 
    $srl_counts{corl_arg} + 
    $srl_counts{corl_prop} + 
    ($counts{tot} - $counts{err_any});
my $lab_micro_pred = 
    $srl_counts{pred_arg} + 
    $srl_counts{pred_prop} + 
    $counts{tot};
my $lab_micro_tot = 
    $srl_counts{tot_arg} +
    $srl_counts{tot_prop} +
    $counts{tot};
my $lab_micro_prec = 100 * $lab_micro_corr / $lab_micro_pred;
my $lab_micro_rec = 100 * $lab_micro_corr / $lab_micro_tot;
my $lab_micro_f1 = 
    2 * $lab_micro_prec * $lab_micro_rec / 
    ($lab_micro_prec + $lab_micro_rec);
printf OUT "  Labeled micro precision:    (%d + %d + %d) / (%d + %d + %d) * 100 = %.2f %%\n",
    ($counts{tot} - $counts{err_any}),
    $srl_counts{corl_arg},
    $srl_counts{corl_prop}, 
    $counts{tot},    
    $srl_counts{pred_arg},
    $srl_counts{pred_prop}, 
    $lab_micro_prec;
printf OUT "  Labeled micro recall:       (%d + %d + %d) / (%d + %d + %d) * 100 = %.2f %%\n",
    ($counts{tot} - $counts{err_any}),
    $srl_counts{corl_arg},
    $srl_counts{corl_prop}, 
    $counts{tot},    
    $srl_counts{tot_arg},
    $srl_counts{tot_prop}, 
    $lab_micro_rec;
printf OUT "  Labeled micro F1:           %.2f \n", $lab_micro_f1;

my $unlab_micro_corr = 
    $srl_counts{coru_arg} + 
    $srl_counts{coru_prop} + 
    ($counts{tot} - $counts{err_head}{tot});
my $unlab_micro_pred = 
    $srl_counts{pred_arg} + 
    $srl_counts{pred_prop} + 
    $counts{tot};
my $unlab_micro_tot = 
    $srl_counts{tot_arg} + 
    $srl_counts{tot_prop} + 
    $counts{tot};
my $unlab_micro_prec = 100 * $unlab_micro_corr / $unlab_micro_pred;
my $unlab_micro_rec = 100 * $unlab_micro_corr / $unlab_micro_tot;
my $unlab_micro_f1 = 
    2 * $unlab_micro_prec * $unlab_micro_rec / 
    ($unlab_micro_prec + $unlab_micro_rec);
printf OUT "  Unlabeled micro precision:  (%d + %d + %d) / (%d + %d + %d) * 100 = %.2f %%\n",
    ($counts{tot} - $counts{err_head}{tot}),
    $srl_counts{coru_arg},
    $srl_counts{coru_prop},
    $counts{tot},
    $srl_counts{pred_arg},
    $srl_counts{pred_prop},
    $unlab_micro_prec;
printf OUT "  Unlabeled micro recall:     (%d + %d + %d) / (%d + %d + %d) * 100 = %.2f %%\n",
    ($counts{tot} - $counts{err_head}{tot}),
    $srl_counts{coru_arg},
    $srl_counts{coru_prop},
    $counts{tot},
    $srl_counts{tot_arg},
    $srl_counts{tot_prop},
    $unlab_micro_rec;
printf OUT "  Unlabeled micro F1:         %.2f \n", $unlab_micro_f1;

if(not defined $opt_u) {
  printf OUT "\n  Note: skipped SU predicates and arguments\n";
  printf OUT "  \tSkipped %d SU predicates and %d SU arguments in gold output\n", 
  	$skipped_su_preds_gold, $skipped_su_args_gold;
  printf OUT "  \tSkipped %d SU predicates and %d SU arguments is system output\n\n", 
  	$skipped_su_preds_sys, $skipped_su_args_sys;
  $skipped_su_preds_gold = 0;
  $skipped_su_args_gold = 0;
  $skipped_su_preds_sys = 0;
  $skipped_su_args_sys = 0;
} else {
  printf OUT "\n  Note: NOT skipping SU predicates and arguments\n\n";
}

################################################################################
###                             end printing output                          ###
################################################################################


if ($short_output)
{
    exit(0) ;
}
printf OUT "\n  %s\n\n", '=' x 80 ;
printf OUT "  Evaluation of the syntactic results in %s\n  vs. gold standard %s:\n\n", $opt_s, $opt_g ;

printf OUT "  Legend: '%s' - the beginning of a sentence, '%s' - the end of a sentence\n\n", $START, $END ;

printf OUT "  Number of non-scoring tokens: " . ($counts{deriv} + $counts{punct}) . "\n\n";

printf OUT "  The overall accuracy and its distribution over PPOSS tags\n\n" ;
printf OUT "%s\n", "  -----------+-------+-------+------+-------+------+-------+-------" ;

printf OUT "  %-10s | %-5s | %-5s |   %%  | %-5s |   %%  | %-5s |   %%\n",
  'Accuracy', 'words', 'right', 'right', 'both' ;
printf OUT "  %-10s | %-5s | %-5s |      | %-5s |      | %-5s |\n",
  ' ', ' ', 'head', ' dep', 'right' ;

printf OUT "%s\n", "  -----------+-------+-------+------+-------+------+-------+-------" ;
printf OUT "  %-10s | %5d | %5d | %3.0f%% | %5d | %3.0f%% | %5d | %3.0f%%\n",
    'total', $counts{tot},
    $counts{tot}-$counts{err_head}{tot}, 
    100-$counts{err_head}{tot}*100.0/$counts{tot},
    $counts{tot}-$counts{err_dep}{tot}, 
    100-$counts{err_dep}{tot}*100.0/$counts{tot},
    $counts{tot}-$counts{err_any}, 
    100-$counts{err_any}*100.0/$counts{tot} ;
printf OUT "%s\n", "  -----------+-------+-------+------+-------+------+-------+-------" ;

foreach $pos (sort {$counts{pos}{$b}{tot} <=> $counts{pos}{$a}{tot}} keys %{$counts{pos}})
{
    if (! defined($counts{pos}{$pos}{err_head}{tot}))
    {
	$counts{pos}{$pos}{err_head}{tot} = 0 ;
    }
    if (! defined($counts{pos}{$pos}{err_dep}{tot}))
    {
	$counts{pos}{$pos}{err_dep}{tot} = 0 ;
    }
    if (! defined($counts{pos}{$pos}{err_any}))
    {
	$counts{pos}{$pos}{err_any} = 0 ;
    }

    printf OUT "  %-10s | %5d | %5d | %3.0f%% | %5d | %3.0f%% | %5d | %3.0f%%\n",
    $pos, $counts{pos}{$pos}{tot},
    $counts{pos}{$pos}{tot}-$counts{pos}{$pos}{err_head}{tot}, 100-$counts{pos}{$pos}{err_head}{tot}*100.0/$counts{pos}{$pos}{tot},
    $counts{pos}{$pos}{tot}-$counts{pos}{$pos}{err_dep}{tot}, 100-$counts{pos}{$pos}{err_dep}{tot}*100.0/$counts{pos}{$pos}{tot},
    $counts{pos}{$pos}{tot}-$counts{pos}{$pos}{err_any}, 100-$counts{pos}{$pos}{err_any}*100.0/$counts{pos}{$pos}{tot} ;
}

printf OUT "%s\n", "  -----------+-------+-------+------+-------+------+-------+-------" ;

printf OUT "\n\n" ;

printf OUT "  The overall error rate and its distribution over PPOSS tags\n\n" ;
printf OUT "%s\n", "  -----------+-------+-------+------+-------+------+-------+-------" ;

printf OUT "  %-10s | %-5s | %-5s |   %%  | %-5s |   %%  | %-5s |   %%\n",
  'Error', 'words', 'head', ' dep', 'both' ;
printf OUT "  %-10s | %-5s | %-5s |      | %-5s |      | %-5s |\n",

  'Rate', ' ', 'err', ' err', 'wrong' ;

printf OUT "%s\n", "  -----------+-------+-------+------+-------+------+-------+-------" ;

printf OUT "  %-10s | %5d | %5d | %3.0f%% | %5d | %3.0f%% | %5d | %3.0f%%\n",
  'total', $counts{tot},
  $counts{err_head}{tot}, $counts{err_head}{tot}*100.0/$counts{tot},
  $counts{err_dep}{tot}, $counts{err_dep}{tot}*100.0/$counts{tot},
  $counts{err_both}, $counts{err_both}*100.0/$counts{tot} ;

printf OUT "%s\n", "  -----------+-------+-------+------+-------+------+-------+-------" ;

foreach $pos (sort {$counts{pos}{$b}{tot} <=> $counts{pos}{$a}{tot}} keys %{$counts{pos}})
{
    if (! defined($counts{pos}{$pos}{err_both}))
    {
	$counts{pos}{$pos}{err_both} = 0 ;
    }

    printf OUT "  %-10s | %5d | %5d | %3.0f%% | %5d | %3.0f%% | %5d | %3.0f%%\n",
    $pos, $counts{pos}{$pos}{tot},
    $counts{pos}{$pos}{err_head}{tot}, $counts{pos}{$pos}{err_head}{tot}*100.0/$counts{pos}{$pos}{tot},
    $counts{pos}{$pos}{err_dep}{tot}, $counts{pos}{$pos}{err_dep}{tot}*100.0/$counts{pos}{$pos}{tot},
    $counts{pos}{$pos}{err_both}, $counts{pos}{$pos}{err_both}*100.0/$counts{pos}{$pos}{tot} ;
    
}

printf OUT "%s\n", "  -----------+-------+-------+------+-------+------+-------+-------" ;

### added by Sabine Buchholz
printf OUT "\n\n";
printf OUT "  Precision and recall of DEPREL\n\n";
printf OUT "  ----------------+------+---------+--------+------------+---------------\n";
printf OUT "  deprel          | gold | correct | system | recall (%%) | precision (%%) \n";
printf OUT "  ----------------+------+---------+--------+------------+---------------\n";
foreach my $dep (sort keys %{$counts{all_dep}}) {
    # initialize
    my ($tot_corr, $tot_g, $tot_s, $prec, $rec) = (0, 0, 0, 'NaN', 'NaN');

    if (defined($counts{dep2}{$dep}{$dep})) {
	$tot_corr = $counts{dep2}{$dep}{$dep};
    } 
    if (defined($counts{dep}{$dep}{tot})) {
    	$tot_g = $counts{dep}{$dep}{tot};
	$rec = sprintf("%.2f",$tot_corr / $tot_g * 100);
    }
    if (defined($counts{dep_s}{$dep}{tot})) {
	$tot_s = $counts{dep_s}{$dep}{tot};
	$prec = sprintf("%.2f",$tot_corr / $tot_s * 100);
    }
    printf OUT "  %-15s | %4d | %7d | %6d | %10s | %13s\n",
    $dep, $tot_g, $tot_corr, $tot_s, $rec, $prec;
}

### DEPREL + ATTACHMENT:
### Same as Sabine's DEPREL apart from $tot_corr calculation
printf OUT "\n\n";
printf OUT "  Precision and recall of DEPREL + ATTACHMENT\n\n";
printf OUT "  ----------------+------+---------+--------+------------+---------------\n";
printf OUT "  deprel          | gold | correct | system | recall (%%) | precision (%%) \n";
printf OUT "  ----------------+------+---------+--------+------------+---------------\n";
foreach my $dep (sort keys %{$counts{all_dep}}) {
    # initialize
    my ($tot_corr, $tot_g, $tot_s, $prec, $rec) = (0, 0, 0, 'NaN', 'NaN');

    if (defined($counts{dep2}{$dep}{$dep})) {
	if (defined($counts{err_head_corr_dep}{$dep})) {
	    $tot_corr = $counts{dep2}{$dep}{$dep} - $counts{err_head_corr_dep}{$dep};
	} else {
	    $tot_corr = $counts{dep2}{$dep}{$dep};
	}
    } 
    if (defined($counts{dep}{$dep}{tot})) {
    	$tot_g = $counts{dep}{$dep}{tot};
	$rec = sprintf("%.2f",$tot_corr / $tot_g * 100);
    }
    if (defined($counts{dep_s}{$dep}{tot})) {
	$tot_s = $counts{dep_s}{$dep}{tot};
	$prec = sprintf("%.2f",$tot_corr / $tot_s * 100);
    }
    printf OUT "  %-15s | %4d | %7d | %6d | %10s | %13s\n",
    $dep, $tot_g, $tot_corr, $tot_s, $rec, $prec;
}
### DEPREL + ATTACHMENT

printf OUT "\n\n";
printf OUT "  Precision and recall of binned HEAD direction\n\n";
printf OUT "  ----------------+------+---------+--------+------------+---------------\n";
printf OUT "  direction       | gold | correct | system | recall (%%) | precision (%%) \n";
printf OUT "  ----------------+------+---------+--------+------------+---------------\n";
foreach my $dir ('to_root', 'left', 'right', 'self') {
    # initialize
    my ($tot_corr, $tot_g, $tot_s, $prec, $rec) = (0, 0, 0, 'NaN', 'NaN');

    if (defined($counts{dir2}{$dir}{$dir})) {
	$tot_corr = $counts{dir2}{$dir}{$dir};
    } 
    if (defined($counts{dir_g}{$dir}{tot})) {
    	$tot_g = $counts{dir_g}{$dir}{tot};
	$rec = sprintf("%.2f",$tot_corr / $tot_g * 100);
    }
    if (defined($counts{dir_s}{$dir}{tot})) {
	$tot_s = $counts{dir_s}{$dir}{tot};
	$prec = sprintf("%.2f",$tot_corr / $tot_s * 100);
    }
    printf OUT "  %-15s | %4d | %7d | %6d | %10s | %13s\n",
    $dir, $tot_g, $tot_corr, $tot_s, $rec, $prec;
}

printf OUT "\n\n";
printf OUT "  Precision and recall of binned HEAD distance\n\n";
printf OUT "  ----------------+------+---------+--------+------------+---------------\n";
printf OUT "  distance        | gold | correct | system | recall (%%) | precision (%%) \n";
printf OUT "  ----------------+------+---------+--------+------------+---------------\n";
foreach my $dist ('to_root', '1', '2', '3-6', '7-...') {
    # initialize
    my ($tot_corr, $tot_g, $tot_s, $prec, $rec) = (0, 0, 0, 'NaN', 'NaN');

    if (defined($counts{dist2}{$dist}{$dist})) {
	$tot_corr = $counts{dist2}{$dist}{$dist};
    } 
    if (defined($counts{dist_g}{$dist}{tot})) {
    	$tot_g = $counts{dist_g}{$dist}{tot};
	$rec = sprintf("%.2f",$tot_corr / $tot_g * 100);
    }
    if (defined($counts{dist_s}{$dist}{tot})) {
	$tot_s = $counts{dist_s}{$dist}{tot};
	$prec = sprintf("%.2f",$tot_corr / $tot_s * 100);
    }
    printf OUT "  %-15s | %4d | %7d | %6d | %10s | %13s\n",
    $dist, $tot_g, $tot_corr, $tot_s, $rec, $prec;
}

printf OUT "\n\n";
printf OUT "  Frame confusions (gold versus system; *...* marks the head token)\n\n";
foreach my $frame (sort {$counts{frame2}{$b} <=> $counts{frame2}{$a}} keys %{$counts{frame2}})
{
    if ($counts{frame2}{$frame} >= 5) # (make 5 a changeable threshold later)
    {
	printf OUT "  %3d  %s\n", $counts{frame2}{$frame}, $frame;
    }
}
### end of: added by Sabine Buchholz


#
# Leave only the 5 words mostly involved in errors
#


$thresh = (sort {$b <=> $a} values %{$counts{word}{err_any}})[4] ;

# ensure enough space for title
$max_word_len = length('word') ;

foreach $word (keys %{$counts{word}{err_any}})
{
  if ($counts{word}{err_any}{$word} < $thresh)
  {
    delete $counts{word}{err_any}{$word} ;
    next ;
  }

  $l = uni_len($word) ;
  if ($l > $max_word_len)
  {
    $max_word_len = $l ;
  }
}

# filter a case when the difference between the error counts
# for 2-word and 1-word contexts is small
# (leave the 2-word context)

foreach $con (keys %{$counts{con_aft_2}{tot}})
{
  ($w1) = split(/\+/, $con) ;
  
  if (defined $counts{con_aft}{tot}{$w1} &&
      $counts{con_aft}{tot}{$w1}-$counts{con_aft_2}{tot}{$con} <= 1)
  {
    delete $counts{con_aft}{tot}{$w1} ;
  }
}

foreach $con (keys %{$counts{con_bef_2}{tot}})
{
  ($w_2, $w_1) = split(/\+/, $con) ;

  if (defined $counts{con_bef}{tot}{$w_1} &&
      $counts{con_bef}{tot}{$w_1}-$counts{con_bef_2}{tot}{$con} <= 1)
  {
    delete $counts{con_bef}{tot}{$w_1} ;
  }
}

foreach $con_pos (keys %{$counts{con_pos_aft_2}{tot}})
{
  ($p1) = split(/\+/, $con_pos) ;

  if (defined($counts{con_pos_aft}{tot}{$p1}) &&
      $counts{con_pos_aft}{tot}{$p1}-$counts{con_pos_aft_2}{tot}{$con_pos} <= 1)
  {
    delete $counts{con_pos_aft}{tot}{$p1} ;
  }
}

foreach $con_pos (keys %{$counts{con_pos_bef_2}{tot}})
{
  ($p_2, $p_1) = split(/\+/, $con_pos) ;

  if (defined($counts{con_pos_bef}{tot}{$p_1}) &&
      $counts{con_pos_bef}{tot}{$p_1}-$counts{con_pos_bef_2}{tot}{$con_pos} <= 1)
  {
    delete $counts{con_pos_bef}{tot}{$p_1} ;
  }
}

# for each context type, take the three contexts most involved in errors

$max_con_len = 0 ;

filter_context_counts($counts{con_bef_2}{tot}, $con_err_num, \$max_con_len) ;

filter_context_counts($counts{con_bef}{tot}, $con_err_num, \$max_con_len) ;

filter_context_counts($counts{con_aft}{tot}, $con_err_num, \$max_con_len) ;

filter_context_counts($counts{con_aft_2}{tot}, $con_err_num, \$max_con_len) ;

# for each PPOSS context type, take the three PPOSS contexts most involved in errors

$max_con_pos_len = 0 ;

$thresh = (sort {$b <=> $a} values %{$counts{con_pos_bef_2}{tot}})[$con_err_num-1] ;

foreach $con_pos (keys %{$counts{con_pos_bef_2}{tot}})
{
  if ($counts{con_pos_bef_2}{tot}{$con_pos} < $thresh)
  {
    delete $counts{con_pos_bef_2}{tot}{$con_pos} ;
    next ;
  }
  if (length($con_pos) > $max_con_pos_len)
  {
    $max_con_pos_len = length($con_pos) ;
  }
}

$thresh = (sort {$b <=> $a} values %{$counts{con_pos_bef}{tot}})[$con_err_num-1] ;

foreach $con_pos (keys %{$counts{con_pos_bef}{tot}})
{
  if ($counts{con_pos_bef}{tot}{$con_pos} < $thresh)
  {
    delete $counts{con_pos_bef}{tot}{$con_pos} ;
    next ;
  }
  if (length($con_pos) > $max_con_pos_len)
  {
    $max_con_pos_len = length($con_pos) ;
  }
}

$thresh = (sort {$b <=> $a} values %{$counts{con_pos_aft}{tot}})[$con_err_num-1] ;

foreach $con_pos (keys %{$counts{con_pos_aft}{tot}})
{
  if ($counts{con_pos_aft}{tot}{$con_pos} < $thresh)
  {
    delete $counts{con_pos_aft}{tot}{$con_pos} ;
    next ;
  }
  if (length($con_pos) > $max_con_pos_len)
  {
    $max_con_pos_len = length($con_pos) ;
  }
}

$thresh = (sort {$b <=> $a} values %{$counts{con_pos_aft_2}{tot}})[$con_err_num-1] ;

foreach $con_pos (keys %{$counts{con_pos_aft_2}{tot}})
{
  if ($counts{con_pos_aft_2}{tot}{$con_pos} < $thresh)
  {
    delete $counts{con_pos_aft_2}{tot}{$con_pos} ;
    next ;
  }
  if (length($con_pos) > $max_con_pos_len)
  {
    $max_con_pos_len = length($con_pos) ;
  }
}

# printing

# ------------- focus words

printf OUT "\n\n" ;
printf OUT "  %d focus words where most of the errors occur:\n\n", scalar keys %{$counts{word}{err_any}} ;

printf OUT "  %-*s | %-4s | %-4s | %-4s | %-4s\n", $max_word_len, ' ', 'any', 'head', 'dep', 'both' ;
printf OUT "  %s-+------+------+------+------\n", '-' x $max_word_len;

foreach $word (sort {$counts{word}{err_any}{$b} <=> $counts{word}{err_any}{$a}} keys %{$counts{word}{err_any}})
{
    if (!defined($counts{word}{err_head}{$word}))
    {
	$counts{word}{err_head}{$word} = 0 ;
    }
    if (! defined($counts{word}{err_dep}{$word}))
    {
	$counts{word}{err_dep}{$word} = 0 ;
    }
    if (! defined($counts{word}{err_any}{$word}))
    {
	$counts{word}{err_any}{$word} = 0;
    }
    printf OUT "  %-*s | %4d | %4d | %4d | %4d\n",
    $max_word_len+length($word)-uni_len($word), $word, $counts{word}{err_any}{$word},
    $counts{word}{err_head}{$word},
    $counts{word}{err_dep}{$word},
    $counts{word}{err_dep}{$word}+$counts{word}{err_head}{$word}-$counts{word}{err_any}{$word} ;
}

printf OUT "  %s-+------+------+------+------\n", '-' x $max_word_len;

# ------------- contexts

printf OUT "\n\n" ;

printf OUT "  one-token preceeding contexts where most of the errors occur:\n\n" ;

print_context($counts{con_bef}, $counts{con_pos_bef}, $max_con_len, $max_con_pos_len) ;

printf OUT "  two-token preceeding contexts where most of the errors occur:\n\n" ;

print_context($counts{con_bef_2}, $counts{con_pos_bef_2}, $max_con_len, $max_con_pos_len) ;

printf OUT "  one-token following contexts where most of the errors occur:\n\n" ;

print_context($counts{con_aft}, $counts{con_pos_aft}, $max_con_len, $max_con_pos_len) ;

printf OUT "  two-token following contexts where most of the errors occur:\n\n" ;

print_context($counts{con_aft_2}, $counts{con_pos_aft_2}, $max_con_len, $max_con_pos_len) ;

# ------------- Sentences

printf OUT "  Sentence with the highest number of word errors:\n" ;
$i = (sort { (defined($err_sent[$b]{word}) && $err_sent[$b]{word})
		 <=> (defined($err_sent[$a]{word}) && $err_sent[$a]{word}) } 1 .. $sent_num)[0] ;
printf OUT "   Sentence %d line %d, ", $i, $starts[$i-1] ;
printf OUT "%d head errors, %d dependency errors, %d word errors\n",
  $err_sent[$i]{head}, $err_sent[$i]{dep}, $err_sent[$i]{word} ;

printf OUT "\n\n" ;

printf OUT "  Sentence with the highest number of head errors:\n" ;
$i = (sort { (defined($err_sent[$b]{head}) && $err_sent[$b]{head}) 
		 <=> (defined($err_sent[$a]{head}) && $err_sent[$a]{head}) } 1 .. $sent_num)[0] ;
printf OUT "   Sentence %d line %d, ", $i, $starts[$i-1] ;
printf OUT "%d head errors, %d dependency errors, %d word errors\n",
  $err_sent[$i]{head}, $err_sent[$i]{dep}, $err_sent[$i]{word} ;

printf OUT "\n\n" ;

printf OUT "  Sentence with the highest number of dependency errors:\n" ;
$i = (sort { (defined($err_sent[$b]{dep}) && $err_sent[$b]{dep}) 
		 <=> (defined($err_sent[$a]{dep}) && $err_sent[$a]{dep}) } 1 .. $sent_num)[0] ;
printf OUT "   Sentence %d line %d, ", $i, $starts[$i-1] ;
printf OUT "%d head errors, %d dependency errors, %d word errors\n",
  $err_sent[$i]{head}, $err_sent[$i]{dep}, $err_sent[$i]{word} ;

#
# Second pass, collect statistics of the frequent errors
#

# filter the errors, leave the most frequent $freq_err_num errors

$i = 0 ;

$thresh = (sort {$b <=> $a} values %freq_err)[$freq_err_num-1] ;

foreach $err (keys %freq_err)
{
  if ($freq_err{$err} < $thresh)
  {
    delete $freq_err{$err} ;
  }
}

# in case there are several errors with the threshold count

$freq_err_num = scalar keys %freq_err ;

%err_counts = () ;

$eof = 0 ;

seek (GOLD, 0, 0) ;
seek (SYS, 0, 0) ;

while (! $eof)
{ # second reading loop

  ($eof, $gold_srl_sent, $sys_srl_sent) = read_sent(\@sent_gold, \@sent_sys) ;
  $sent_num++ ;

  $word_num = scalar @sent_gold ;

  # printf "$sent_num $word_num\n" ;
  
  foreach $i_w (0 .. $word_num-1)
  { # loop on words
    ($word, $pos, $head_g, $dep_g)
      = @{$sent_gold[$i_w]}{'word', 'pos', 'head', 'dep'} ;

    # printf "%d: %s %s %s %s\n", $i_w,  $word, $pos, $head_g, $dep_g ;

    if ((! $score_on_punct) && is_uni_punct($word))
    {
      # ignore punctuations
      next ;
    }

    if ((! $score_on_deriv) && ($dep_g eq 'DERIV'))
    {
      # ignore deriv
      next ;
    }

    ($head_s, $dep_s) = @{$sent_sys[$i_w]}{'head', 'dep'} ;

    $err_head = ($head_g ne $head_s) ;
    $err_dep = ($dep_g ne $dep_s) ;

    $head_err = '-' ;
    $dep_err = '-' ;

    if ($head_g eq '0')
    {
      $head_aft_bef_g = '0' ;
    }
    elsif ($head_g eq $i_w+1)
    {
      $head_aft_bef_g = 'e' ;
    }
    else
    {
      $head_aft_bef_g = ($head_g <= $i_w+1 ? 'b' : 'a') ;
    }

    if ($head_s eq '0')
    {
      $head_aft_bef_s = '0' ;
    }
    elsif ($head_s eq $i_w+1)
    {
      $head_aft_bef_s = 'e' ;
    }
    else
    {
      $head_aft_bef_s = ($head_s <= $i_w+1 ? 'b' : 'a') ;
    }

    $head_aft_bef = $head_aft_bef_g.$head_aft_bef_s ;

    if ($err_head)
    {
      if ($head_aft_bef_s eq '0')
      {
	$head_err = 0 ;
      }
      else
      {
	$head_err = $head_s-$head_g ;
      }
    }

    if ($err_dep)
    {
      $dep_err = $dep_g.'->'.$dep_s ;
    }

    if (! ($err_head || $err_dep))
    {
      next ;
    }

    # handle only the most frequent errors

    $err = $head_err.$sep.$head_aft_bef.$sep.$dep_err ;

    if (! exists $freq_err{$err})
    {
      next ;
    }

    ($w_2, $w_1, $w1, $w2, $p_2, $p_1, $p1, $p2) = get_context(\@sent_gold, $i_w) ;

    $con_bef = $w_1 ;
    $con_bef_2 = $w_2.' + '.$w_1 ;
    $con_aft = $w1 ;
    $con_aft_2 = $w1.' + '.$w2 ;

    $con_pos_bef = $p_1 ;
    $con_pos_bef_2 = $p_2.'+'.$p_1 ;
    $con_pos_aft = $p1 ;
    $con_pos_aft_2 = $p1.'+'.$p2 ;

    @cur_err = ($con_pos_bef, $con_bef, $word, $pos, $con_pos_aft, $con_aft) ;

    # printf "# %-25s %-15s %-10s %-25s %-3s %-30s\n",
    #  $con_bef, $word, $pos, $con_aft, $head_err, $dep_err ;
    
    @bits = (0, 0, 0, 0, 0, 0) ;
    $j = 0 ;

    while ($j == 0)
    {
      for ($i = 0; $i <= $#bits; $i++)
      {
	if ($bits[$i] == 0)
	{
	  $bits[$i] = 1 ;
	  $j = 0 ;
	  last ;
	}
	else
	{
	  $bits[$i] = 0 ;
	  $j = 1 ;
	}
      }

      @e_bits = @cur_err ;

      for ($i = 0; $i <= $#bits; $i++)
      {
	if (! $bits[$i])
	{
	  $e_bits[$i] = '*' ;
	}
      }

      # include also the last case which is the most general
      # (wildcards for everything)
      $err_counts{$err}{join($sep, @e_bits)}++ ;

    }

  } # loop on words
} # second reading loop

printf OUT "\n\n" ;
printf OUT "  Specific errors, %d most frequent errors:", $freq_err_num ;
printf OUT "\n  %s\n", '=' x 41 ;


# deleting local contexts which are too general

foreach $err (keys %err_counts)
{
  foreach $loc_con (sort {$err_counts{$err}{$b} <=> $err_counts{$err}{$a}}
		    keys %{$err_counts{$err}})
  {
    @cur_err = split(/\Q$sep\E/, $loc_con) ;

    # In this loop, one or two elements of the local context are
    # replaced with '*' to make it more general. If the entry for
    # the general context has the same count it is removed.

    foreach $i (0 .. $#cur_err)
    {
      $w1 = $cur_err[$i] ;
      if ($cur_err[$i] eq '*')
      {
	next ;
      }
      $cur_err[$i] = '*' ;
      $con1 = join($sep, @cur_err) ;
      if ( defined($err_counts{$err}{$con1}) && defined($err_counts{$err}{$loc_con})
	   && ($err_counts{$err}{$con1} == $err_counts{$err}{$loc_con}))
      {
	delete $err_counts{$err}{$con1} ;
      }
      for ($j = $i+1; $j <=$#cur_err; $j++)
      {
	if ($cur_err[$j] eq '*')
	{
	  next ;
	}
	$w2 = $cur_err[$j] ;
	$cur_err[$j] = '*' ;
	$con1 = join($sep, @cur_err) ;
	if ( defined($err_counts{$err}{$con1}) && defined($err_counts{$err}{$loc_con})
	     && ($err_counts{$err}{$con1} == $err_counts{$err}{$loc_con}))
	{
	  delete $err_counts{$err}{$con1} ;
	}
	$cur_err[$j] = $w2 ;
      }
      $cur_err[$i] = $w1 ;
    }
  }
}

# Leaving only the topmost local contexts for each error

foreach $err (keys %err_counts)
{
  $thresh = (sort {$b <=> $a} values %{$err_counts{$err}})[$spec_err_loc_con-1] || 0 ;

  # of the threshold is too low, take the 2nd highest count
  # (the highest may be the total which is the generic case
  #   and not relevant for printing)

  if ($thresh < 5)
  {
    $thresh = (sort {$b <=> $a} values %{$err_counts{$err}})[1] ;
  }

  foreach $loc_con (keys %{$err_counts{$err}})
  {
    if ($err_counts{$err}{$loc_con} < $thresh)
    {
      delete $err_counts{$err}{$loc_con} ;
    }
    else
    {
      if ($loc_con ne join($sep, ('*', '*', '*', '*', '*', '*')))
      {
	$loc_con_err_counts{$loc_con}{$err} = $err_counts{$err}{$loc_con} ;
      }
    }
  }
}

# printing an error summary

# calculating the context field length

$max_word_spec_len= length('word') ;
$max_con_aft_len = length('word') ;
$max_con_bef_len = length('word') ;
$max_con_pos_len = length('PPOSS') ;

foreach $err (keys %err_counts)
{
  foreach $loc_con (sort keys %{$err_counts{$err}})
  {
    ($con_pos_bef, $con_bef, $word, $pos, $con_pos_aft, $con_aft) =
      split(/\Q$sep\E/, $loc_con) ;

    $l = uni_len($word) ;
    if ($l > $max_word_spec_len)
    {
      $max_word_spec_len = $l ;
    }

    $l = uni_len($con_bef) ;
    if ($l > $max_con_bef_len)
    {
      $max_con_bef_len = $l ;
    }

    $l = uni_len($con_aft) ;
    if ($l > $max_con_aft_len)
    {
      $max_con_aft_len = $l ;
    }

    if (length($con_pos_aft) > $max_con_pos_len)
    {
      $max_con_pos_len = length($con_pos_aft) ;
    }

    if (length($con_pos_bef) > $max_con_pos_len)
    {
      $max_con_pos_len = length($con_pos_bef) ;
    }
  }
}

$err_counter = 0 ;

foreach $err (sort {$freq_err{$b} <=> $freq_err{$a}} keys %freq_err)
{

  ($head_err, $head_aft_bef, $dep_err) = split(/\Q$sep\E/, $err) ;

  $err_counter++ ;
  $err_desc{$err} = sprintf("%2d. ", $err_counter).
    describe_err($head_err, $head_aft_bef, $dep_err) ;
  
  # printf OUT "  %-3s %-30s %d\n", $head_err, $dep_err, $freq_err{$err} ;
  printf OUT "\n" ;
  printf OUT "  %s : %d times\n", $err_desc{$err}, $freq_err{$err} ;

  printf OUT "  %s-+-%s-+-%s-+-%s-+-%s-+-%s-+------\n",
    '-' x $max_con_pos_len, '-' x $max_con_bef_len,
       '-' x $max_pos_len, '-' x $max_word_spec_len,
	'-' x $max_con_pos_len, '-' x $max_con_aft_len ;

  printf OUT "  %-*s | %-*s | %-*s | %s\n",
      $max_con_pos_len+$max_con_bef_len+3, '  Before',
	$max_word_spec_len+$max_pos_len+3, '   Focus',
	  $max_con_pos_len+$max_con_aft_len+3, '  After',
	    'Count' ;

  printf OUT "  %-*s   %-*s | %-*s   %-*s | %-*s   %-*s |\n",
    $max_con_pos_len, 'PPOSS', $max_con_bef_len, 'word',
       $max_pos_len, 'PPOSS', $max_word_spec_len, 'word',
	$max_con_pos_len, 'PPOSS', $max_con_aft_len, 'word' ;
  
  printf OUT "  %s-+-%s-+-%s-+-%s-+-%s-+-%s-+------\n",
    '-' x $max_con_pos_len, '-' x $max_con_bef_len,
       '-' x $max_pos_len, '-' x $max_word_spec_len,
	'-' x $max_con_pos_len, '-' x $max_con_aft_len ;

  foreach $loc_con (sort {$err_counts{$err}{$b} <=> $err_counts{$err}{$a}}
		    keys %{$err_counts{$err}})
  {
    if ($loc_con eq join($sep, ('*', '*', '*', '*', '*', '*')))
    {
      next ;
    }

    $con1 = $loc_con ;
    $con1 =~ s/\*/ /g ;

    ($con_pos_bef, $con_bef, $word, $pos, $con_pos_aft, $con_aft) =
      split(/\Q$sep\E/, $con1) ;

    printf OUT "  %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %3d\n",
      $max_con_pos_len, $con_pos_bef, $max_con_bef_len+length($con_bef)-uni_len($con_bef), $con_bef,
	  $max_pos_len, $pos, $max_word_spec_len+length($word)-uni_len($word), $word,
	    $max_con_pos_len, $con_pos_aft, $max_con_aft_len+length($con_aft)-uni_len($con_aft), $con_aft,
	      $err_counts{$err}{$loc_con} ;
  }
  
  printf OUT "  %s-+-%s-+-%s-+-%s-+-%s-+-%s-+------\n",
    '-' x $max_con_pos_len, '-' x $max_con_bef_len,
       '-' x $max_pos_len, '-' x $max_word_spec_len,
	'-' x $max_con_pos_len, '-' x $max_con_aft_len ;

}

printf OUT "\n\n" ;
printf OUT "  Local contexts involved in several frequent errors:" ;
printf OUT "\n  %s\n", '=' x 51 ;
printf OUT "\n\n" ;

foreach $loc_con (sort {scalar keys %{$loc_con_err_counts{$b}} <=>
			  scalar keys %{$loc_con_err_counts{$a}}}
		  keys %loc_con_err_counts)
{

  if (scalar keys %{$loc_con_err_counts{$loc_con}} == 1)
  {
    next ;
  }
  
  printf OUT "  %s-+-%s-+-%s-+-%s-+-%s-+-%s-\n",
    '-' x $max_con_pos_len, '-' x $max_con_bef_len,
       '-' x $max_pos_len, '-' x $max_word_spec_len,
	'-' x $max_con_pos_len, '-' x $max_con_aft_len ;

  printf OUT "  %-*s | %-*s | %-*s \n",
      $max_con_pos_len+$max_con_bef_len+3, '  Before',
	$max_word_spec_len+$max_pos_len+3, '   Focus',
	  $max_con_pos_len+$max_con_aft_len+3, '  After' ;

  printf OUT "  %-*s   %-*s | %-*s   %-*s | %-*s   %-*s \n",
    $max_con_pos_len, 'PPOSS', $max_con_bef_len, 'word',
       $max_pos_len, 'PPOSS', $max_word_spec_len, 'word',
	$max_con_pos_len, 'PPOSS', $max_con_aft_len, 'word' ;
  
  printf OUT "  %s-+-%s-+-%s-+-%s-+-%s-+-%s-\n",
    '-' x $max_con_pos_len, '-' x $max_con_bef_len,
       '-' x $max_pos_len, '-' x $max_word_spec_len,
	'-' x $max_con_pos_len, '-' x $max_con_aft_len ;

  $con1 = $loc_con ;
  $con1 =~ s/\*/ /g ;

  ($con_pos_bef, $con_bef, $word, $pos, $con_pos_aft, $con_aft) =
      split(/\Q$sep\E/, $con1) ;

  printf OUT "  %-*s | %-*s | %-*s | %-*s | %-*s | %-*s \n",
    $max_con_pos_len, $con_pos_bef, $max_con_bef_len+length($con_bef)-uni_len($con_bef), $con_bef,
      $max_pos_len, $pos, $max_word_spec_len+length($word)-uni_len($word), $word,
	$max_con_pos_len, $con_pos_aft, $max_con_aft_len+length($con_aft)-uni_len($con_aft), $con_aft ;
	  
  printf OUT "  %s-+-%s-+-%s-+-%s-+-%s-+-%s-\n",
    '-' x $max_con_pos_len, '-' x $max_con_bef_len,
       '-' x $max_pos_len, '-' x $max_word_spec_len,
	'-' x $max_con_pos_len, '-' x $max_con_aft_len ;

  foreach $err (sort {$loc_con_err_counts{$loc_con}{$b} <=>
			$loc_con_err_counts{$loc_con}{$a}}
		keys %{$loc_con_err_counts{$loc_con}})
  {
    printf OUT "  %s : %d times\n", $err_desc{$err},
      $loc_con_err_counts{$loc_con}{$err} ;
  }

  printf OUT "\n" ;
}

###################################################################
#
#   Some stats for the parsing of semantic dependencies
#
###################################################################

printf OUT "\n  %s\n\n", '=' x 80 ;
printf OUT "  Evaluation of the semantic results in %s\n  vs. gold standard %s:\n\n", $opt_s, $opt_g ;

printf OUT "  Precision and recall for LABELED semantic dependencies to ROOT\n";
    printf OUT "  (i.e., predicate identification and classification)\n";
printf OUT "  ----------------+------+---------+--------+------------+---------------|--------|\n";
printf OUT "  PPOSS           | gold | correct | system | recall (%%) | precision (%%) |     F1 | \n";
printf OUT "  ----------------+------+---------+--------+------------+---------------|--------|\n";
foreach my $pposs (sort keys %{$srl_counts{prop_per_tag}}) {
    my ($tot_corr, $tot_g, $tot_s, $prec, $rec, $f1) = (0, 0, 0, 'NaN', 'NaN', 'NaN');

    if (defined($srl_counts{corl_prop_per_tag}{$pposs})) {
	$tot_corr = $srl_counts{corl_prop_per_tag}{$pposs};
    }
    if (defined($srl_counts{pred_prop_per_tag}{$pposs})) {
	$tot_s = $srl_counts{pred_prop_per_tag}{$pposs};
	$prec = sprintf("%.2f",$tot_corr / $tot_s * 100);
    }
    if(defined($srl_counts{tot_prop_per_tag}{$pposs})){
	$tot_g = $srl_counts{tot_prop_per_tag}{$pposs};
	$rec = sprintf("%.2f",$tot_corr / $tot_g * 100);

	if(defined($srl_counts{tot_prop_per_tag}{$pposs}) and $prec > 0 and $rec > 0){
	    $f1 = sprintf("%.2f", 2 * $prec * $rec / ($prec + $rec));
	}
    }

    printf OUT "  %-15s | %4d | %7d | %6d | %10s | %13s | %6s |\n",
    $pposs, $tot_g, $tot_corr, $tot_s, $rec, $prec, $f1;
}
printf OUT "  ----------------+------+---------+--------+------------+---------------|--------|\n\n";


printf OUT "  Precision and recall for UNLABELED semantic dependencies to ROOT\n";
printf OUT "  (i.e., predicate identification)\n";
printf OUT "  ----------------+------+---------+--------+------------+---------------|--------|\n";
printf OUT "  PPOSS           | gold | correct | system | recall (%%) | precision (%%) |     F1 | \n";
printf OUT "  ----------------+------+---------+--------+------------+---------------|--------|\n";
foreach my $pposs (sort keys %{$srl_counts{prop_per_tag}}) {
    my ($tot_corr, $tot_g, $tot_s, $prec, $rec, $f1) = (0, 0, 0, 'NaN', 'NaN', 'NaN');

    if (defined($srl_counts{coru_prop_per_tag}{$pposs})) {
	$tot_corr = $srl_counts{coru_prop_per_tag}{$pposs};
    }
    if(defined($srl_counts{tot_prop_per_tag}{$pposs})){
	$tot_g = $srl_counts{tot_prop_per_tag}{$pposs};
	$rec = sprintf("%.2f",$tot_corr / $tot_g * 100);
    }
    if (defined($srl_counts{pred_prop_per_tag}{$pposs})) {
	$tot_s = $srl_counts{pred_prop_per_tag}{$pposs};
	$prec = sprintf("%.2f",$tot_corr / $tot_s * 100);
	if(defined($srl_counts{tot_prop_per_tag}{$pposs}) and $prec > 0 and $rec > 0){
	    $f1 = sprintf("%.2f", 2 * $prec * $rec / ($prec + $rec));
	}
    }

    printf OUT "  %-15s | %4d | %7d | %6d | %10s | %13s | %6s |\n",
    $pposs, $tot_g, $tot_corr, $tot_s, $rec, $prec, $f1;
}
printf OUT "  ----------------+------+---------+--------+------------+---------------|--------|\n\n";

printf OUT "  Precision and recall for non-ROOT semantic dependencies\n";
printf OUT "  (i.e., identification and classification of arguments)\n";
printf OUT "  ------------------+------+---------+--------+------------+---------------|--------|\n";
printf OUT "  PPOSS(pred) + ARG | gold | correct | system | recall (%%) | precision (%%) |     F1 | \n";
printf OUT "  ------------------+------+---------+--------+------------+---------------|--------|\n";
foreach my $label (sort keys %{$srl_counts{arg_per_tag}}) {
    my ($tot_corr, $tot_g, $tot_s, $prec, $rec, $f1) = (0, 0, 0, 'NaN', 'NaN', 'NaN');

    if (defined($srl_counts{corl_arg_per_tag}{$label})) {
	$tot_corr = $srl_counts{corl_arg_per_tag}{$label};
    }

    if(defined($srl_counts{tot_arg_per_tag}{$label})){
	$tot_g = $srl_counts{tot_arg_per_tag}{$label};
	$rec = sprintf("%.2f",$tot_corr / $tot_g * 100);
    }
    if (defined($srl_counts{pred_arg_per_tag}{$label})) {
	$tot_s = $srl_counts{pred_arg_per_tag}{$label};
	$prec = sprintf("%.2f",$tot_corr / $tot_s * 100);
	if(defined($srl_counts{tot_arg_per_tag}{$label}) and $prec > 0 and $rec > 0){
	    $f1 = sprintf("%.2f", 2 * $prec * $rec / ($prec + $rec));
	}
    }
    
    printf OUT "  %-17s | %4d | %7d | %6d | %10s | %13s | %6s |\n",
    $label, $tot_g, $tot_corr, $tot_s, $rec, $prec, $f1;
}
printf OUT "  ------------------+------+---------+--------+------------+---------------|--------|\n\n";


###################################################################
#
#   END stats for the parsing of semantic dependencies
#
###################################################################

close GOLD ;
close SYS ;

close OUT ;


###################################################################
#
#   Package    s e n t e n c e: A sentence as a set of propositions
#
#   Adapted from the srl-eval.pl script of CoNLL 2005
#   March 2008
#
####################################################################

package SRL::sentence; 
use strict; 

sub new {
    my ($pkg) = @_; 
    
    my $s = []; 

    $s->[0] = [];       # props

    return bless $s, $pkg; 
}

sub add_props {
    my $s = shift;
    push @{$s->[0]}, @_;
}

sub props {
    my $s = shift; 
    return @{$s->[0]};
}

sub prop_count {
    my $s = shift; 
    return ($#{$s->[0]} + 1);
}

sub arg_count {
    my $s = shift;
    my $c = 0;
    foreach my $prop ($s->props()) {
    	my @args = $prop->args();
    	$c += ($#args + 1);
    }
    return $c;
}

sub display {
    my $s = shift; 
    foreach my $prop ($s->props()) {
    	$prop->display();
    }
}

1; # end package SRL::sentence;


##################################################################
#
#  Package    p r o p  :  A proposition (verb + args)
#
#  Adapted from the srl-eval.pl script of CoNLL 2005
#  March 2008
#
##################################################################

package SRL::prop;
use strict; 

# Constructor: creates a new prop, with empty arguments
# Parameters: verb form, position of verb
sub new {
    my ($pkg, $v, $sense, $pposs, $position, $arg_column) = @_;

    my $p = [];
    
    $p->[0] = $v;          # the verb
    $p->[1] = $position;   # verb position
    $p->[2] = $sense;      # verb sense
    $p->[3] = $pposs;      # the PPOSS tag for this verb
    $p->[4] = $arg_column; # which column contains my args
    $p->[5] = [];          # args, empty by default 

    return bless $p, $pkg; 
}

## Accessor/Initialization methods

# returns the verb form of the prop
sub lemma {
    my $p = shift; 
    return $p->[0];
}

# returns the verb position of the verb in the prop
sub position {
    my $p = shift; 
    return $p->[1];
}

# returns the verb sense of the verb in the prop
sub sense {
    my $p = shift; 
    return $p->[2];
}

# returns the PPOSS tag of this predicate
sub pposs {
    my $p = shift;
    return $p->[3];
}

sub column {
    my $p = shift; 
    return $p->[4];
}
# returns the list of arguments of the prop
sub args {
    my $p = shift; 
    return @{$p->[5]};
}

# initializes the list of arguments of the prop
sub set_args {
    my $p = shift; 
    @{$p->[5]} = @_;
}

# adds arguments to the prop
sub add_args {
    my $p = shift; 
    push @{$p->[5]}, @_;
}

# adds one argument to the prop
sub add_arg {
    my $p = shift; 
    my $arg = shift;
    push @{$p->[5]}, $arg;
}

# number of args
sub arg_count {
    my $p = shift;
    my $size = @{$p->[5]};
    return $size;
}

sub display {
   my $p = shift;
   printf STDERR "%s %d", $p->lemma(), $p->sense();
   foreach my $arg ($p->args()) {
   	printf STDERR " (%s, %d)", $arg->label(), $arg->position();
   }
   printf STDERR "\n";
}

1; # end SRL::prop package


##################################################################
#
#  Package    a r g  :  An argument
#
#  March 2008
#
##################################################################

package SRL::arg;
use strict; 

# Constructor: creates a new arg
sub new {
    my ($pkg, $label, $position) = @_;

    my $a = [];
    
    $a->[0] = $label;          # the label
    $a->[1] = $position;   # arg position

    return bless $a, $pkg; 
}

# returns the arg label
sub label {
    my $a = shift; 
    return $a->[0];
}

# returns the verb position of the verb in the prop
sub position {
    my $p = shift; 
    return $p->[1];
}

1; # end SRL::arg package


