int main(void) {
	int i = 0;
	int total = 0;
	while (i < 4) {
		total = total + i;
		i = i + 1;
	}
	if (total == 6) return 0;
	return 1;
}
