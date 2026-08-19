int main(void) {
	int i = 0;
	int total = 0;
	while (i < 14) {
		switch (i) {
		case 7:
			i = i + 1;
			continue;
		default:
			total = total + i;
		}
		i = i + 1;
	}
	if (total == 84) return 0;
	return 1;
}
