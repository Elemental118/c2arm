int main(void) {
	int x = 18;
	{
		int x = 28;
		if (x != 28) {
			return 1;
		}
	}
	if (x == 18) return 0;
	return 1;
}
