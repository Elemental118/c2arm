int main(void) {
	int i = 0;
	int total = 0;
	while (i < 11) {
		if (i == 2) {
			i = i + 1;
			continue;
		}
		total = total + i;
		i = i + 1;
	}
	if (total == 53) return 0;
	return 1;
}
