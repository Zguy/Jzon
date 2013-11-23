#!/bin/bash
prog=$1
outfile="test.out"
FAIL='\E[1;31m'"FAIL"'\E[0m'
PASS='\E[1;32m'"PASS"'\E[0m'

run_prog() {
	success=true
	$prog "spec/"$1$2".json" 2>$outfile || success=false
}

run_success() {
	echo "Running pass test ${1}"
	run_prog "pass" $1
	if $success; then
		echo -e $PASS
	else
		echo -en $FAIL" "
		cat $outfile
		failure=true
	fi
}
touch $outfile

failure=false

for i in $(seq 1 3); do
	run_success $i
done


rm $outfile

if [ $failure ]; then
	exit 1
else
	exit 0
fi


