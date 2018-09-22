#!/bin/sh

. $TESTDIR/lib.sh

make_eduos

check_out $CDIR/1 timeout 10 $IMAGE
