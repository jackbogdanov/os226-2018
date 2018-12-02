#!/bin/sh

. $TESTDIR/lib.sh

timeout -s 9 10 $IMAGE < $CDIR/1.in | awk -f $CDIR/test.awk

