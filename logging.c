
/*
*Subject	: COMP30017 Operating System and Network Services
*Project	: Project 2
*FileName	: main.c
*Author		: Tofi Buzali 513816 & Lucy Munanto 346003
*/

#include "logging.h"
#include <string.h>
#include <time.h>
#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h>

//The File pointer
static FILE *fp; 
//The Mutex Pointer
pthread_mutex_t mtxptr = PTHREAD_MUTEX_INITIALIZER;   
char *log_file_pathName;
void *smalloc(size_t size); 
char* getTimeStamp();
 
/*
* This function is to be called once, 
* before the first call to log_write is made.
*  
* log_file_name is the path and file name of the file to be used for logging.
*/
void log_setUp(char* log_file_name){

	//We need to create the log path 	 
	log_file_pathName = smalloc (1024); 
	strcpy (log_file_pathName, log_file_name);
	fp = fopen(log_file_name, "a+"); 

}

void* smalloc (size_t size){
	void *p = malloc(size); 
	if (p == NULL){
		fprintf(stderr, "There isn't enough space"); 
		exit(1); 
	}
	return p;
}
/*
* Writes a log entry to the log file.
* entries must be formatted with this pattern: "%s: %s: %s: %s: %s\n"
* From left to right the %s are to be:
*  - a time stamp, in local time.
*  - module (which part of the program created the entry?)
*  - p1
*  - p2
*  - p3
* 
* Note: This function may be called from multiple threads and hence must
*       ensure that only one thread is writing to the file at any given time.
*/
void log_write(char* module, char* p1, char* p2, char* p3){

	//Check to see if the mutex is already locked 
    
    while (pthread_mutex_trylock(&mtxptr) !=0);

		fopen(log_file_pathName, "a+"); 	
		
        char *time = getTimeStamp();
        if (time[strlen(time)-1] == '\n')
            time[strlen(time)-1] = '\0';
        
		fprintf(fp, "%s: %s: %s: %s: %s\n", time, module,
			p1, p2, p3);
		        
        
		fclose(fp); 
		//Unlock the mutex
		pthread_mutex_unlock(&mtxptr); 
}

/*
* Gets the local time stamp. Returns a character string representing the
* current date and time
*/
char* getTimeStamp(){

	time_t now = time(NULL);
	return asctime (localtime (&now)); 
}
