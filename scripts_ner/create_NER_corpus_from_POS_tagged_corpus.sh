file_conll2003=$1
file_tagged=$2
paste ${file_tagged} ${file_conll2003} | awk '{ if (NF>0) print $1" "$2" "$5; else print ""}'