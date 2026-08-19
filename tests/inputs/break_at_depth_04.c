int main(void) {
	int total = 0;
	for (int i = 0; i < 6; i = i + 1) {
		if (i == 2) {
			switch (i) {
			case 2:
				total = total + 1;
				break;
			default:
				total = total + 100;
			}
		}
		total = total + 1;
	}
	if (total == 7) return 0;
	return 1;
}
