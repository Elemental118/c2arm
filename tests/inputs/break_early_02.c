int main(void) {
	int i = 0;
	int count = 0;
	while (i < 7) {
		if (i == 4) {
			break;
		}
		count = count + 1;
		i = i + 1;
	}
	if (count == 4) return 0;
	return 1;
}
