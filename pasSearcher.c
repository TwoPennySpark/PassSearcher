#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

//#define D printf("%d\n", __LINE__);
#define BUFFER_SIZE 128
#define PASSWORD_SIZE 32
#define PTHREAD_COUNT 16

char buffer[BUFFER_SIZE][PASSWORD_SIZE] = {};
char password[PASSWORD_SIZE];
int cancel = 0;
pthread_t pth_num[PTHREAD_COUNT/2] = {};

typedef struct thread_data_d 
{
	int from;
	int to;
} thread_data;


void* password_cmp(void* arg)
{
	thread_data data = *(thread_data*)arg;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	printf("[%x] from: %d\tto: %d\n", pthread_self(), data.from, data.to);
	for (int l = data.from;l < data.to;l++)
	{	
		//pthread_testcancel();
		if(!strcmp(buffer[l], password))
		{
			cancel++;
			printf(	"======================================================\n"
				"[+]Match found. Your password is unreliable!\nPassword is:%s\n"
				"======================================================\n", buffer[l]);
		}
	}		
	printf("[-][%x]Can't find match\n", pthread_self());
	pthread_exit(NULL);
}

void division_into_threads(char buf[BUFFER_SIZE][PASSWORD_SIZE])
{
	thread_data thr_data[PTHREAD_COUNT/2];

	//check if we've already found the right word
	if (cancel != 0)
		return;

	for (int j = 0; j < (PTHREAD_COUNT/2); j++)
	{
		//divides buffer on threads
		thr_data[j].from = PTHREAD_COUNT*j;
		thr_data[j].to   = PTHREAD_COUNT*(j+1);  
		if((pthread_create(&pth_num[j], NULL, password_cmp,&thr_data[j]))!= 0)
		{		
			printf("[-]Can't create thread\n");
			return;
		}
	}
	
 	for (int k = 0; k < (PTHREAD_COUNT/2); k++)
		pthread_join(pth_num[k], NULL);	
}

int main(void)
{
   	FILE* stream;
        char* line = NULL;
        size_t len = 0;
        ssize_t read;
	int i = 0;	
	
	printf("Enter password:\n");
	scanf("%s", password);	

	//Test if file is open		
        stream = fopen("pass_file", "r");
        if (stream == NULL)
	{
		printf("[-]Can't open the file\n");
       		return -1;
	}
 
	//Read a piece of text from file and place it into the buffer
	while((read = fscanf(stream, "%s", buffer[i])) != -1)
	{
		i++;
		if (i >= BUFFER_SIZE)
		{
			//Sending buffer to a function that divides it into threads
			division_into_threads(buffer);
			i = 0;
		}
        }

        fclose(stream);
	
	return 0;   		    
}
