int main(void) {
	int x = 2;
	int result = 0;
	if (x > 0) {
		result = 1;
	} else {
		if (x < 0) {
			result = 2;
		} else {
			result = 3;
		}
	}
	if (result == 1) return 0;
	return 1;
}
