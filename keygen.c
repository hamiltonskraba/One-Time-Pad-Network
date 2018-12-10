#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

//acceptable characters
char alphas[27] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";


int main(int argc, char* argv[]){
	
	//validate argument format 
	if(argc != 2){	
		fprintf(stderr, "Incorrect number of arguments\n");
		exit(1);
	}

	int size = atoi(argv[1]);			//save size from command line arg

	char* key = malloc(sizeof (char) * (size + 1)); //allocate space for size + newline

	srand(time(0));					//enable random integer generator

	int i;
	for(i = 0; i < size; i++){			
		int choice = rand() % 27;		//choose random integer 0 - 26
		//printf("%d", choice); 
		char letter = alphas[choice];		//pull letter index from array
		key[i] = letter;			//push to allocated string
	}

	key[i] = '\n';					//tack on new line	

	fprintf(stdout, key);				//print to std out

	free(key);					//free allocated memory

	return 0;
}
