#!/bin/sh

. $TESTDIR/lib.sh

check() {
	t1=`date +%s.%N`
	"$@" | \
		awk 'NR == 1 { l = $2 } NR == 2 { exit(($1 - l >= 2000000) ? 0 : 1) }'
	r1=$?
	t2=`date +%s.%N`
	echo "$t1 $t2" | awk '{ exit(($2 - $1 >= 2.0) ? 0 : 1) }'
	r2=$?
	[ $r1 = 0 ] && [ $r2 = 0 ]
}

check timeout -s 9 10 $IMAGE < $CDIR/1.in
