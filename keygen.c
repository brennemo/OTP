/*
**	keygen.c
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
	int i, numChars;
	char randomKey[numChars];
	char alphabet[27] = {
					'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
					'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
					'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' '
					};

	srand(time(NULL));

	numChars = atoi(argv[1]);

	//print all args 
	for (i = 0; i < argc; i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");

	//1st arg 
	printf("%s\n", argv[1]);
}