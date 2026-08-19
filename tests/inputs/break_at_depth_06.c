int main(void) {
	int total = 0;
	for (int i = 0; i < 10; i = i + 1) {
		if (i == 5) {
			switch (i) {
			case 5:
				total = total + 1;
				break;
			default:
				total = total + 100;
			}
		}
		total = total + 1;
	}
	if (total == 11) return 0;
	return 1;
}
