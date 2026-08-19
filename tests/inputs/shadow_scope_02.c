int main(void) {
	int x = 8;
	{
		int x = 22;
		if (x != 22) {
			return 1;
		}
	}
	if (x == 8) return 0;
	return 1;
}
