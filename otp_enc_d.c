/*
**	otp_enc_d.c
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

void catchSIGCHLD(int signo);				//catch SIGCHLD to manage child processes 

int main(int argc, char *argv[])
{
	int i;
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[100000];
	struct sockaddr_in serverAddress, clientAddress;
	char temp, key, encrypt;
	
	char messageType[4];
	char keyEnd[] = "#"; //strEnd[] = "@";
	char typeEnd[] = "%";
	int plainZero, endOfString; 				//1st index of plain text 
	
	char keyText[100000];
	char plainText[100000];

	//Initialize sigaction struct, signal handler, and override default actions
	struct sigaction SIGCHLD_action = { 0 }; 
	SIGCHLD_action.sa_handler = catchSIGCHLD;
	sigfillset(&SIGCHLD_action.sa_mask);
	SIGCHLD_action.sa_flags = 0; 
	sigaction(SIGCHLD, &SIGCHLD_action, NULL);

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections


	//Loop to accept connectons 
	while (1) {
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		//Use a separate process to handle the rest of the transaction 
		pid_t childPid = -5;	int childExitMethod;

		//make sure child is communicating with otp_enc
		childPid = fork();
		if (childPid == -1) {
			perror("Hull Breach\n");
			exit(1);
		}
		else if (childPid == 0) {
			
			memset(buffer, '\0', 100000);
			charsRead = recv(establishedConnectionFD, buffer, 99999, 0); // Read the client's message from the socket
			if (charsRead < 0) error("ERROR reading from socket");
			printf("SERVER: I received this from the client: \"%s\"\n", buffer);
			
			//printf("BUFFER IN SERVER: %s\n", buffer);
			//printf("Buffer size: %d\n", strlen(buffer));
			//Split key and plain text 
			/*
			char messageType[4];
			char keyEnd[] = "#"; //strEnd[] = "@";
			char typeEnd[] = "%";
			int plainZero, endOfString; 				//1st index of plain text 
			*/
			
			plainZero = strcspn(buffer, keyEnd);
			plainZero += 2; 

			
			//endOfString = strcspn(buffer, strEnd);
			//printf("end of string: %s\n", buffer[endOfString]);

			//printf("1st char of plain text: \'%c\'\n", buffer[plainZero]);
			//plainText = malloc((plnLen + 1) * sizeof(char));	//allocate space for buffers (+1 for '\0')
			//char *keyText = malloc((plainZero - 1) * sizeof(char));
			//char *plainText = malloc((strlen(buffer) - plainZero) * sizeof(char));
			
			//copy key into separate string
			strncpy(keyText, buffer, plainZero - 2);
			//printf("%s\n", keyText);
			
			//copy plain text into separate string 
			int j = 0;
			for (i = plainZero; i < strlen(buffer) - 2; i++) {
					plainText[j] = buffer[i];
					j++;
			}
			
			//printf("plain text: %s\n", plainText);
			//printf("keyText size: %d, plainText size:%d\n", strlen(keyText), strlen(plainText));

			
			// Encrypt Message
			//char *testMessage = "HELLO", *testKey = "XMCKL"; 
			char *encryptedMessage = malloc(strlen(plainText) * sizeof(char));
			memset(encryptedMessage, '\0', strlen(encryptedMessage));

			//convert ASCII values to 0...26 for A...' '
			for (i = 0; i < strlen(plainText); i++) {
				if (plainText[i] == ' ') 
					temp = 26; 
				else 
					temp = plainText[i] - 65;
				if (keyText[i] == ' ')
					key = 26;
				else 
					key = keyText[i] - 65; 

				encrypt = (temp + key) % 27;
				encryptedMessage[i] = encrypt;
			}

			//convert encrypted string back to ASCII values 
			for (i = 0; i < strlen(encryptedMessage); i++) {
				if (encryptedMessage[i] == 26) {
					encryptedMessage[i] = ' ';
				}	
				else {
					encryptedMessage[i] += 65; 
				}
			}
			
			printf("Encrypted message: %s\n", encryptedMessage);
			printf("Key: %s\n", keyText);


			// Send a Success message back to the client
			charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
			if (charsRead < 0) error("ERROR writing to socket");
			
			//free(keyText);
			//free(plainText);
			
			//close(listenSocketFD);
			//exit(0);

		}					//if childPid == 0
		else {				//childPid == parentPid 
			
			close(establishedConnectionFD); // Close the existing socket which is connected to the client
		}

		
	}						//while loop
	//free(plainText);
	//free(keyText);
	//free(encryptedMessage);
	//free(decryptedMessage);
	close(listenSocketFD); // Close the listening socket
	return 0; 
}


void catchSIGCHLD(int signo) {
	while (waitpid(-1, 0, WNOHANG) > 0) {}
}
