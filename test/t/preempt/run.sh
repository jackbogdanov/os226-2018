#!/bin/bash

. $TESTDIR/lib.sh

check() {
	"$@" | awk '
	{
		s[$1]++
		printf "\r"
		for (i in s)
			printf("%s : %d; ", i, s[i])
		printf("     ")
	}
	END {
		r = s[1] / s[2]
		d = 0.2
		exit(0.8 <= r && r <= 1.2 ? 0 : 1)
	}'
}

check timeout -s 9 10 $IMAGE < $CDIR/1.in
