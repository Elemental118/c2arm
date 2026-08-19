int main(void) {
	int count = 0;
	for (int i = 0; i < 5; i = i + 1) {
		for (int j = 0; j < 4; j = j + 1) {
			count = count + 1;
		}
	}
	if (count == 20) return 0;
	return 1;
}
