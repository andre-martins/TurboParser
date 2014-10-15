file_conll2003=$1
awk '{ if (NF>0) print $1"\t_"; else print ""}' ${file_conll2003}
