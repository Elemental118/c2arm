int main(void) {
	int i = 0;
	int total = 0;
	do {
		switch (i % 2) {
		case 0:
			if (i == 4) {
				total = total + 50;
			} else {
				total = total + 2;
			}
			break;
		default:
			if (i == 4) {
				total = total + 50;
			} else {
				total = total + 3;
			}
		}
		i = i + 1;
	} while (i < 6);
	if (total == 63) return 0;
	return 1;
}
