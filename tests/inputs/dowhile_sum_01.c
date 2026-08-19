int main(void) {
	int i = 0;
	int total = 0;
	do {
		total = total + i;
		i = i + 1;
	} while (i < 1);
	if (total == 0) return 0;
	return 1;
}
