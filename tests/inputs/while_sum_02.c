int main(void) {
	int i = 0;
	int total = 0;
	while (i < 8) {
		total = total + i;
		i = i + 1;
	}
	if (total == 28) return 0;
	return 1;
}
