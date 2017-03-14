/*
**	otp_dec_d.c
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	int i;
	// Decrypt Message
	char *testMessage = "DQNVZ", *testKey = "XMCKL"; 
	char decryptedMessage[7];
	char temp, key, decrypt;
	memset(decryptedMessage, '\0', 7);

	//convert ASCII values to 0...26 for A...' '
	for (i = 0; i < strlen(testMessage); i++) {
		if (testMessage[i] == ' ') 
			temp = 26; 
		else 
			temp = testMessage[i] - 65;
		if (testKey[i] == ' ')
			key = 26;
		else 
			key = testKey[i] - 65; 

		decrypt = (temp - key + 27) % 27;
		decryptedMessage[i] = decrypt;
	}

	//convert decrypted string back to ASCII values 
	for (i = 0; i < strlen(decryptedMessage); i++) {
		if (decryptedMessage[i] == 26) {
			decryptedMessage[i] = ' ';
		}	
		else {
			decryptedMessage[i] += 65; 
		}
	}

	//test print 
	printf("%s\n", decryptedMessage);

	return 0;
}