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
char recBuff[BUFFSIZE];
char keyBuff[BUFFSIZE];
char cipherBuff[BUFFSIZE];

int main(int argc, char* argv[]){

	if(argc != 2){ 									//validate argument format
		fprintf(stderr, "Error: Invalid number of arguments\n");
		exit(1);
	}
	
	//set up socket------------------------------------------------------------
	struct	sockaddr_in serverAddress, clientAddress;
	socklen_t sizeofClientInfo;

	memset((char*)&serverAddress, '\0', sizeof(serverAddress));			//clear address struct

	int portNumber = atoi(argv[1]);							//convert command string to port number

	serverAddress.sin_family = AF_INET;						//mark socket as network capable
	serverAddress.sin_port = htons(portNumber);					//store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY;					//allow any address to connect

	int listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);				//create the socket

	if(listenSocketFD < 0){								//check for creation error
		fprintf(stderr, "Error: Unable to open server socket\n");
		exit(1);
	}

	if(bind(listenSocketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){	//enable listening
		fprintf(stderr, "Error: Unable to bind server socket\n");
		exit(1);
	}
	
	listen(listenSocketFD, 5);							//turn on the socket

	while(1){

		sizeofClientInfo = sizeof(clientAddress);					//get the size of connecting address

		int establishedConnectionFD = accept(listenSocketFD, (struct sockaddr*)&clientAddress, &sizeofClientInfo);	//accept connection

		if(establishedConnectionFD < 0){						//check for connection error
			fprintf(stderr, "Error: Unable to accept client connection");
		}
		
		pid_t cpid = -2;								//start fork process
		cpid = fork();

		switch(cpid) {

			case -1: {								//check for child error
				fprintf(stderr, "Unable to create child process\n");
				break;
			}

			case 0: {

			//receive ciphertext from client-----------------------------------------------
				memset(recBuff, '\0', BUFFSIZE);						//clear buffer

				int cipherText = recv(establishedConnectionFD, recBuff, BUFFSIZE - 1, 0);	//read plaintext data
				//printf("Server read %d bytes from plaintext\n", cipherText);		
			
				if(cipherText < 0){
					fprintf(stderr, "Error: Server unable to receive ciphertext\n");
				}

				//printf("Server ciphertext received: %s\n", recBuff);fflush(stdout);


			//send data to client for test receive----------------------------------------
				int testSend = send(establishedConnectionFD, "decserv?", 8, 0);			//send specific test message to check connection

				if(testSend < 0){
					fprintf(stderr, "Error: Server unable to write to client socket\n");
				}

			//receive key from client------------------------------------------------------
				memset(keyBuff, '\0', BUFFSIZE);						//clear buffer

				int keyText = recv(establishedConnectionFD, keyBuff, BUFFSIZE - 1, 0);		//read key data
				
				if(keyText < 0){
					fprintf(stderr, "Error: Server unable to receive key\n");
				}

				//printf("Server key received: %s\n", keyBuff);fflush(stdout);

			//decrypt ciphertext and key to plaintext--------------------------------------
				int i, j, buffInt, keyInt, cipherInt;
				char buffChar, keyChar, cipherChar;

				memset(cipherBuff, '\0', BUFFSIZE);

				for(i = 0; i < (cipherText - 1); i++){						//for the size of the ciphertext file
				
					buffChar = recBuff[i];							//examine first character of text and key
					keyChar = keyBuff[i];
					//printf("Converting %c from plaintext and %c from key\n", buffChar, keyChar);

					for(j = 0; j < 27; j++){						//convert text character to an integer
						if(alphas[j] == buffChar){
							buffInt = j;
						}
					}

					for(j = 0; j < 27; j++){						//convert key character to an integer
						if(alphas[j] == keyChar){
							keyInt = j;
						}
					}

					//printf("Converted %d from plaintext and %d from key\n\n", buffInt, keyInt);
				
					int temp = buffInt - keyInt;						//subtract key value from cipher
					//printf("subtracted value: %d\n", temp);					
	
					if(temp < 0){								//if negative, add 27
						temp = temp + 27;
					}

					cipherInt = temp % 27;							//modulo 27
					//printf("modulo value: %d\n", cipherInt);

					cipherChar = alphas[cipherInt];						//convert back to character
					//printf("converted to %c\n", cipherChar);
			
					cipherBuff[i] = cipherChar;						//build up plaintext string
					//printf("%c\n", cipherBuff[i]);
				}

			//send back unencrypted plaintext------------------------------------------------
				int cipherSend = send(establishedConnectionFD, cipherBuff, cipherText, 0);	//send the data

				if(cipherSend < 0 || cipherSend < cipherText){					//check for error
					fprintf(stderr, "Error: Unable to return encrypted text to client\n");
				}

				//printf("printing string:%s\n", cipherBuff);

				close(establishedConnectionFD);							//close current connection
				break;
			}

		}//end of switch

	}//end of while

	close(listenSocketFD);											//close the socket
	return 0;
}
