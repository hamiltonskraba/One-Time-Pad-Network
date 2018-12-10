#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFSIZE 1024

char alphas[27] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char readBuff[BUFFSIZE];
char keyBuff[BUFFSIZE];
char recBuff[BUFFSIZE];
char cipherBuff[BUFFSIZE];


void validateRead(int read, char* buffer){
	int i, j;									//check for bad characters	
	for(i = 0; i < read - 1; i++){	
		for(j = 0; j < 27; j++){						//check each byte against available characters
			if(buffer[i] == alphas[j]){					//break if found	
				//printf("buffer pos %d is %c\n", i, alphas[j]);
				break;
			}
			else if (j == 26){						//error if not found 
				fprintf(stderr, "Error: Bad character %c in plaintext file \n", buffer[i]);
				exit(1);
			}
		}
	}
	//printf("validated input\n");
}

int main(int argc, char* argv[]){

	if(argc != 4){ 									//validate argument format
		fprintf(stderr, "Error: Invalid number of arguments\n");
		exit(1);
	}

	//open and read plaintext file-----------------------------------------------
	int firstFD = open(argv[1], 0444);

	if(firstFD < 0){								//check for error opening
		fprintf(stderr, "Error: Cannot open plaintext file %s\n", argv[1]);
		exit(1);
	}	

	memset(readBuff, '\0', BUFFSIZE);						//clear the buffer

	int firstBytes = read(firstFD, readBuff, BUFFSIZE);				//read the file

	if(firstBytes < 0){								//check for error reading
		fprintf(stderr, "Error: Cannot read plaintext file %s\n", argv[1]);
		exit(1);
	}

	//printf("%d bytes read from %s\n", firstBytes, argv[1]);

	validateRead(firstBytes, readBuff);						//check for bad characters

	close(firstFD);									//close the file

	//open and read key file ----------------------------------------------------
	int secondFD = open(argv[2], 0444);

	if(secondFD < 0){								//check for error opening
		fprintf(stderr, "Error: Cannot open key file %s\n", argv[2]);
		exit(1);
	}	

	memset(keyBuff, '\0', BUFFSIZE);						//clear the buffer

	int secondBytes = read(secondFD, keyBuff, BUFFSIZE);				//read the file

	if(secondBytes < 0){								//check for error reading
		fprintf(stderr, "Error: Cannot read key file %s\n", argv[2]);
		exit(1);
	}

	//printf("%d bytes read from %s\n", secondBytes, argv[2]);

	validateRead(secondBytes, keyBuff);						//check for bad characters

	close(secondFD);								//close the file

	if(secondBytes < firstBytes){							//check for adequate key file length
		fprintf(stderr, "Error: Key file is too short\n");
		exit(1);
	}

	int portNumber = atoi(argv[3]);						//convert command string to port number

	//printf("port number %d\n", portNumber);

	//initialize sockets---------------------------------------------------------

	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	memset((char*)&serverAddress, '\0', sizeof(serverAddress));			//clear the address struct
	
	serverAddress.sin_family = AF_INET;						//mark socket as network capable
	serverAddress.sin_port = htons(portNumber);					//store the port number

	serverHostInfo = gethostbyname("localhost");					//set host to local machine

	if(serverHostInfo == NULL){							//check for host error
		fprintf(stderr, "Error: Unable to set client host\n");
		exit(2);
	}	

	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);	//preserve special host address

	int socketFD = socket(AF_INET, SOCK_STREAM, 0);					//create the socket

	if(socketFD < 0){								//check for creation errors
		fprintf(stderr, "Error: Unable to open client socket\n");
		exit(2);
	}

	if(connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){	//connect the socket to the server address
		fprintf(stderr, "Error: Unable to connect client socket\n");
		exit(2);
	}


	//write ciphertext data to server---------------------------------------------------------
	int dataSent = send(socketFD, readBuff, strlen(readBuff), 0);			

	if(dataSent < 0){								//check for writing error
		fprintf(stderr, "Error: Unable to write to client socket\n");
		exit(2);
	}

	if(dataSent < strlen(readBuff)){						//check that all bytes were written
		fprintf(stderr, "Warning: not all data written to socket\n");
	}


	//check receivable connection to server-------------------------------------------------
	memset(recBuff, '\0', sizeof(recBuff));						//clear the buffer

	int testRead = recv(socketFD, recBuff, sizeof(recBuff), 0);			//read the string
	
	if(testRead < 0){								//check for read error
		fprintf(stderr, "Unable to read from server socket\n");
		exit(2);
	}

	if(strcmp(recBuff, "decserv?") != 0){						//make sure not connected to otp_enc
		fprintf(stderr, "Error: mismatch in connection port\n");
		exit(2);
	}

	//printf("Client recieved from server: %s\n", recBuff);

	//write key data to server--------------------------------------------------------------
	dataSent = send(socketFD, keyBuff, strlen(keyBuff), 0);				//send the data

	if(dataSent < 0){								//check for writing error
		fprintf(stderr, "Error: Unable to write to client socket\n");
		exit(2);
	}

	if(dataSent < strlen(keyBuff)){							//check that all bytes were written
		fprintf(stderr, "Warning: not all data written to socket\n");
	}

	//read back unencrypted data from server-------------------------------------------------
	memset(recBuff, '\0', BUFFSIZE);						//clear the buffer

	int cipherRead = recv(socketFD, recBuff, BUFFSIZE, 0);				//read the data
	
	if(testRead < 0){								//check for error
		fprintf(stderr, "Unable to read from server socket\n");
	}

	printf("%s\n", recBuff);							//print plaintext to console

	return 0;
}
