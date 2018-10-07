#!/bin/sh

. $TESTDIR/lib.sh

make_eduos

check_out $CDIR/1 timeout -s 9 10 $IMAGE
