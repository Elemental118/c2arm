int main(void) {
	int count = 15;
	int n = 5;
	int total = 0;
	switch (count % n) {
	case 0:
		do {
			total = total + 1;
	case 4:
			total = total + 1;
	case 3:
			total = total + 1;
	case 2:
			total = total + 1;
	case 1:
			total = total + 1;
			count = count - n;
		} while (count > 0);
	}
	if (total == 15) return 0;
	return 1;
}
