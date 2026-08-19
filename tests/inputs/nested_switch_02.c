int main(void) {
	int outer_val = 0;
	int inner_val = 0;
	int total = 0;
	switch (outer_val) {
	case 0:
		total = 10;
		break;
	case 1:
		total = 20;
		switch (inner_val) {
		case 0:
			total = total + 1;
			break;
		case 1:
			total = total + 2;
			break;
		default:
			total = total + 3;
		}
		break;
	default:
		total = 30;
	}
	if (total == 10) return 0;
	return 1;
}
