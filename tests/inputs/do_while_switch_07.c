int main(void) {
	int i = 0;
	int total = 0;
	do {
		switch (i % 2) {
		case 0:
			if (i == 2) {
				total = total + 50;
			} else {
				total = total + 2;
			}
			break;
		default:
			if (i == 2) {
				total = total + 50;
			} else {
				total = total + 3;
			}
		}
		i = i + 1;
	} while (i < 4);
	if (total == 58) return 0;
	return 1;
}
