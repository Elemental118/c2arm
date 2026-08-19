int main(void) {
	int count = 0;
	for (int i = 0; i < 4; i = i + 1) {
		count = count + 1;
	}
	if (count == 4) return 0;
	return 1;
}
