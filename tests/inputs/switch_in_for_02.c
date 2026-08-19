int main(void) {
	int total = 0;
	for (int i = 0; i < 11; i = i + 1) {
		switch (i) {
		case 0:
			total = total + 10;
			break;
		default:
			total = total + 1;
		}
	}
	if (total == 20) return 0;
	return 1;
}
