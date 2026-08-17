void main(void) {
	int x = 7;
	{int x = 3; int y = x;}
	int y = x;
}