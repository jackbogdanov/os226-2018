#!/bin/sh

. $TESTDIR/lib.sh

make_eduos

check() {
	t1=`date +%s.%N`
	"$@" | \
		awk 'NR == 1 { l = $2 } NR == 2 { if ($1 - l >= 2.0) print "OK" }' | \
		grep "OK"
	r1=$?
	t2=`date +%s.%N`
	echo "$t1 $t2" | awk '{if ($2-$1 >= 2.0) print "OK"}' | grep "OK"
	r2=$?
	[ $r1 = 0 ] && [ $r2 = 0 ]
}

check $IMAGE < $CDIR/1.in
