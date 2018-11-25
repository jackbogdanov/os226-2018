BEGIN {
	verbose = 1
}

verbose { print }

$1 == "in" {
	if (s) {
		err = 1
		if (verbose) {
			printf "%d enters critical section while it holded by %d\n", $2, s
		}
	}
	s = $2
	if (max < s) {
		max = s
	}
	seen[s] = 1
}

$1 == "out" {
	if (s != $2) {
		err = 1
		if (verbose) {
			printf "%d leaves critical section while it holded by %d\n", $2, s
		}
	}
	s = 0
}

END {
	for (i = 1; i <= max; ++i) {
		if (!seen[i]) {
			err = 1
			if (verbose) {
				printf "%d never holded mutex\n", i
			}
		}
	}
	exit(err)
}
