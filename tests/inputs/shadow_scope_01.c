int main(void) {
	int x = 11;
	{
		int x = 22;
		if (x != 22) {
			return 1;
		}
	}
	if (x == 11) return 0;
	return 1;
}
