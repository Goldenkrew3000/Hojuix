int multiply(int a, int b);
void do_something();

int main(int argc, char* argv) {
	int num1, num2, num3 = 0;
	num1 = 8;
	num2 = 10;
	num3 = multiply(num1, num2);
	do_something();
	return num3;
}

int multiply(int a, int b) {
	int c = a * b;
	return c;
}

void do_something() {
	int d = 1 * 3 * 5 + 2;
	return;
}
