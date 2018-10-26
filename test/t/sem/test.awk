BEGIN {
	verbose = 1
}

verbose { print }

$1 == "in" {
	if (s) {
		if (verbose) {
			printf "%d enters critical section while it holded by %d\n", $2, s
		}
		exit 1
	}
	s = $2
}

$1 == "out" {
	if (s != $2) {
		if (verbose) {
			printf "%d leaves critical section while it holded by %d\n", $2, s
		}
	}
	s = 0
}
