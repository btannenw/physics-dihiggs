#!/bin/bash

# quick script to compare the uncertainties on NPs before and after HESSE inversion
# works with FCC log files

# $1 = log file
# $2 = threshold for warnings in % (optional)
# $3 = grep NPs (optional)

f=$1
th="20"

if [ ! "$2" = "" ]; then
  th="$2"
fi

if [ "$3" = "" ]; then
  3="="
fi

echo
echo "Printing pre and post HESSE inversion NPs. Warning for relative difference > ${th}%."
NPs=$(cat $f | grep "+/-" | grep "=" | grep "$3" | awk '{print $1}')
echo $1
cat $1 | grep "Minuit2Minimizer :"
for NP in $NPs; do
    pre=$(cat $f | grep "$NP	" | grep "+/-" | grep "=" | awk '{printf("%.3f",$5)}')
    post=$(cat $f | grep "$NP " | grep "+/-" | grep -v "=" | awk '{printf("%.3f",$4)}')
    warn=$(echo "$pre $post" | awk "{if (\$1 == 0 || \$2 == 0) {print \"ZERO    \"; exit;} if (\$1 / \$2 - 1 > $th / 100 || \$2 / \$1 - 1 > $th / 100 ) print \"WARNING \"; else print \"OK      \";}")
    echo "$pre  $post  $warn  $NP"
done
