int main(void) {
	int i = 0;
	int total = 0;
	while (i < 11) {
		switch (i) {
		case 8:
			i = i + 1;
			continue;
		default:
			total = total + i;
		}
		i = i + 1;
	}
	if (total == 47) return 0;
	return 1;
}
