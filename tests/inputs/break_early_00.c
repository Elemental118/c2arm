int main(void) {
	int i = 0;
	int count = 0;
	while (i < 11) {
		if (i == 6) {
			break;
		}
		count = count + 1;
		i = i + 1;
	}
	if (count == 6) return 0;
	return 1;
}
