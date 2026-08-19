int main(void) {
	int x = -7;
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
	if (result == 2) return 0;
	return 1;
}
