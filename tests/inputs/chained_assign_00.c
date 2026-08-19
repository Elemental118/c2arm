int main(void) {
	int x = 0;
	int y = 0;
	int z = 0;
	x = y = z = 41;
	if (x == 41) {
		if (y == 41) {
			if (z == 41) return 0;
		}
	}
	return 1;
}
