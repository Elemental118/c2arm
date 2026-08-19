int main(void) {
	int total = 0;
	for (int i = 0; i < 9; i = i + 1) {
		if (i == 4) {
			switch (i) {
			case 4:
				total = total + 1;
				break;
			default:
				total = total + 100;
			}
		}
		total = total + 1;
	}
	if (total == 10) return 0;
	return 1;
}
