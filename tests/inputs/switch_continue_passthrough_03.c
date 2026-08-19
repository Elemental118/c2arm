int main(void) {
	int i = 0;
	int total = 0;
	while (i < 7) {
		switch (i) {
		case 5:
			i = i + 1;
			continue;
		default:
			total = total + i;
		}
		i = i + 1;
	}
	if (total == 16) return 0;
	return 1;
}
