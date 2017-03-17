/*
**	otp_dec.c
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

#define BUFFER_SIZE 100000

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int i;
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[BUFFER_SIZE];
	int plainTextFile, keyFile;
	int plnLen, keyLen; 
	char *plainText, *keyText; 
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	//Open plain text and key files 
	plainTextFile = open(argv[1], O_RDONLY);
	keyFile = open(argv[2], O_RDONLY);
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
	lseek(keyFile, 0, SEEK_SET);
	read(plainTextFile, plainText, plnLen);				//read files and store in buffers
	read(keyFile, keyText, keyLen);
	
	//find and remove newlines
	char delim[] = "\n";
	int newline = -1; 
	newline = strcspn(plainText, delim);
	//printf("Newline in plainText (length %d) at %d\n", newline, plnLen);	
	
	//newline = -1;
	newline = strcspn(keyText, delim);
	//printf("Newline in keyText (length %d) at %d\n", newline, keyLen);
	
	plainText[strlen(plainText) - 2] = '\0';	//append '\0' to each, overwriting '\n'	
	keyText[strlen(keyText) - 1] = '\0';		

	//Validate characters in plain text and key, minus null terminator
	
	for (i = 0; i < plnLen - 1; i++) {
		if (plainText[i] < 65 && plainText[i] > 90 && plainText[i] != 32) {		
			fprintf(stderr, "input contains bad characters\n");
			exit(1);
		}
	}

	for (i = 0; i < keyLen - 1; i++) {
		if (keyText[i] < 65 && keyText[i] > 90 && keyText[i] != 32) {		
			fprintf(stderr, "input contains bad characters\n");
			exit(1);
		}
	}
	
	/*
	printf("KEY:\n");
	for (i = 0;i < keyLen; i++) {
		printf("%d	%c	%d\n", i, keyText[i], keyText[i]);
	}

	printf("TEXT:\n");
	for (i = 0;i < plnLen; i++) {
		printf("%d	%c	%d\n", i, plainText[i], plainText[i]);
	}
	*/

	//test print 
	//printf("plain text: %s\n key: %s\n", plainText, keyText);
	
	//Combine key and plain text into one string to send 
	//Method from Professor Brewster's example on Piazza 
	memset(buffer, '\0', sizeof(buffer));
	strcat(buffer, "DEC%%");					//signal to daemon that this is encoding 
	strcat(buffer, keyText);
	strcat(buffer, "##");
	strcat(buffer, plainText);
	strcat(buffer, "@@");
	
	//printf("combined strings: %s\n", buffer);

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
		fprintf(stderr, "Error: could not contact otp_enc_d on port %d\n", portNumber);



	// Send message to server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

	printf("%s\n", buffer);
	close(socketFD); // Close the socket

	free(plainText);
	free(keyText);

	return 0;
}
