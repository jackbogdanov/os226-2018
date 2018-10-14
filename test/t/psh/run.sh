#!/bin/sh

. $TESTDIR/lib.sh

e=0

check_out2 $CDIR/1 $IMAGE || e=1

check_out2 $CDIR/2 $IMAGE || e=1

exit $e
