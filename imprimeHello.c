#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char* argv[]){

	if(argc == 1){
		printf("Hello world\n");
		return 0;
	}

	int t = (argv[1][0]%10)+1;
	
	sleep(t);//Espera uma quantidade aleatória com base na primeira letra

	printf("[Esperei %ds] Ola %s, estou terminando\n", t, argv[1]);

	return 0;
}