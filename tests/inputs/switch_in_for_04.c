int main(void) {
	int total = 0;
	for (int i = 0; i < 7; i = i + 1) {
		switch (i) {
		case 2:
			total = total + 10;
			break;
		default:
			total = total + 1;
		}
	}
	if (total == 16) return 0;
	return 1;
}
