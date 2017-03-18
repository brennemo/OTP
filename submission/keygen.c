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
	char alphabet[27] = {
					'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
					'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
					'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' '
					};

	srand(time(NULL));

	numChars = atoi(argv[1]) + 1;
	char randomKey[numChars];

	for (i = 0; i < numChars - 1; i++) {
		randomKey[i] = alphabet[rand() % 27]; 
	}
	randomKey[numChars - 1] = '\n';

	for (i = 0; i < numChars; i++) {
		printf("%c", randomKey[i]);
	}

}