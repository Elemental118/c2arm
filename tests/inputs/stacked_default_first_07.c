int main(void) {
	int x = 3;
	int total = 0;
	switch (x) {
	default:
		total = 999;
		break;
	case 1:
	case 2:
		total = 111;
		break;
	case 3:
		total = 333;
		break;
	case 4:
		total = 444;
		break;
	}
	if (total == 333) return 0;
	return 1;
}
