/*
**	otp_enc.c
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int i;
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[2048];
	FILE *plainTextFile, *keyFile;
	int plnLen, keyLen; 
	char *plainText, *keyText; 
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	//Open plain text and key files 
	plainTextFile = open(argv[1], "r");
	keyFile = open(argv[2], "r");
	if ((plainTextFile < 0) || (keyFile < 0)) { fprintf(stderr, "Cannot open file\n"); exit(1); };

	//Get lengths of text in files and compare to ensure key >= plain text  
	plnLen = lseek(plainTextFile, 0, SEEK_END);
	keyLen = lseek(keyFile, 0, SEEK_END);
	if (keyLen < plnLen) { fprintf(stderr, "key \'%s\' is too short\n", argv[2]); exit(1); };

	//Store text from files in buffers, 
	plainText = malloc((plnLen + 1) * sizeof(char));	//allocate space for buffers (+1 for '\0')
	keyText = malloc((keyLen + 1) * sizeof(char));
	//memset(plainText, '\0', plnLen + 1);				//fill buffers with null terminators 
	//memset(keyText, '\0', keyLen + 1);
	lseek(plainTextFile, 0, SEEK_SET);					//return to beginning of each file 
	lseek(keyText, 0, SEEK_SET);
	read(plainTextFile, plainText, plnLen);				//read files and store in buffers
	read(keyFile, keyText, keyLen);
	plainText[plnLen] = '\0';							//append '\0' to each 
	keyText[keyLen] = '\0';							

	//Validate characters in plain text and key, minus null terminator
	for (i = 0; i < plnLen - 1; i++) {
		if (plainText[i] < 65 || plainText[i] > 90 || plainText[i] != 32) {		
			fprintf(stderr, "input contains bad characters\n");
			exit(1);
		}
	}

	for (i = 0; i < keyLen - 1; i++) {
		if (keyText[i] < 65 || keyText[i] > 90 || keyText[i] != 32) {		
			fprintf(stderr, "input contains bad characters\n");
			exit(1);
		}
	}

	//test print 
	printf("plain text: %s\n key: %s\n", plainText, keyText);

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Get input message from user
	/*
	printf("CLIENT: Enter text to send to the server, and then hit enter: ");
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	fgets(buffer, sizeof(buffer) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
	buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds
	*/


	// Send message to server
	
	charsWritten = send(socketFD, plainText, strlen(plainText), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

	close(socketFD); // Close the socket

	free(plainText);
	free(keyText);

	return 0;
}
