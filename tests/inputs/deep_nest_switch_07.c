int main(void) {
	int total = 0;
	for (int i = 0; i < 4; i = i + 1) {
		int j = 0;
		while (j < 2) {
			int k = 0;
			do {
				switch (k) {
				case 0:
					total = total + 5;
					break;
				default:
					total = total + 1;
				}
				k = k + 1;
			} while (k < 3);
			j = j + 1;
		}
	}
	if (total == 56) return 0;
	return 1;
}
