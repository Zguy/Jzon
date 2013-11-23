#!/bin/bash
prog=$1
outfile="test.out"
FAIL='\E[1;31m'"FAIL"'\E[0m'
PASS='\E[1;32m'"PASS"'\E[0m'

run_prog() {
	success=true
	$prog "spec/"$1$2".json" 2>$outfile || success=false
}

passes=0
fails=0

run_success() {
	echo "Running pass test ${1}"
	run_prog "pass" $1
	if $success; then
		((passes++))
		echo -e $PASS" JSON parsed successfully!"
	else
		((fails++))
		echo -en $FAIL" "
		cat $outfile
		failure=true
	fi
}

run_failure() {
	echo "Running fail test ${1}"
	run_prog "fail" $1
	if $success; then
		((fails++))
		echo -e $FAIL" JSON parsed successfully!"
	else
		((passes++))
		echo -en $PASS" "
		cat $outfile
		failure=true
	fi
}

touch $outfile

failure=false

for i in $(seq 1 3); do
	run_success $i
done

for i in $(seq 1 33); do
	run_failure $i
done

rm $outfile

echo
echo "Tests completed with "$passes" passes and "$fails" failures"

if [ $failure ]; then
	exit 1
else
	exit 0
fi


