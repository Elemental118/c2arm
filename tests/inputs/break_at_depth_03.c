int main(void) {
	int total = 0;
	for (int i = 0; i < 12; i = i + 1) {
		if (i == 8) {
			switch (i) {
			case 8:
				total = total + 1;
				break;
			default:
				total = total + 100;
			}
		}
		total = total + 1;
	}
	if (total == 13) return 0;
	return 1;
}
