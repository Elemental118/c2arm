int main(void) {
	int count = 0;
	for (int i = 0; i < 3; i = i + 1) {
		for (int j = 0; j < 3; j = j + 1) {
			count = count + 1;
		}
	}
	if (count == 9) return 0;
	return 1;
}
