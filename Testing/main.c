#include <stdio.h>
#include <stdlib.h>

int main() {
	int number = 10;
	printf("Number: %d\n", number);

	int* ptr = &number; // Stores the address
	printf("Pointer Address: %p\n", ptr);
	printf("Pointer Number: %d\n", *ptr);




	return EXIT_SUCCESS;
}
