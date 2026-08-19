int main(void) {
	int x = 0;
	int total = 0;
	int flag = 1;
	switch (x) {
		if (flag) {
		case 0:
			total = 100;
			break;
		case 1:
			total = 200;
			break;
		} else {
		case 2:
			total = 300;
			break;
		default:
			total = 400;
		}
	}
	if (total == 100) return 0;
	return 1;
}
