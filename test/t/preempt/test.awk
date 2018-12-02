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
	exit((1 - d <= r && r <= 1 + d) ? 0 : 1)
}
