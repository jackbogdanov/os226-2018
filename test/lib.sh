
CDIR=$(dirname $0)

check_out() {
	local pat=$1
	shift 1

	"$@" <$pat.in >$pat.out
	diff -q $pat.out $pat.out.gold
}

check_out2() {
	local pat=$1
	shift 1

	while read l; do echo $l; sleep 0.1; done <$pat.in | "$@" >$pat.out
	diff -u $pat.out $pat.out.gold
}

make_eduos() {
	make clean all
}

cd $TESTDIR/..

IMAGE="./build/image ./build/rootfs.cpio"
