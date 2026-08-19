int main(void) {
	int i = 0;
	int count = 0;
	while (i < 12) {
		if (i == 7) {
			break;
		}
		count = count + 1;
		i = i + 1;
	}
	if (count == 7) return 0;
	return 1;
}
