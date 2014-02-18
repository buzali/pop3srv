
/*
*Subject	: COMP30017 Operating System and Network Services
*Project	: Project 2
*FileName	: main.c
*Author		: Tofi Buzali 513816 & Lucy Munanto 346003
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "socket.h"
#include "maildir.h"
#include "logging.h"
#include "pop.h"


int main(int args, char **argv){
    
	 if(args != 4){
		printf("USAGE: pop3 [port] [maildir path] [log file]\n");
		printf("NOTE: the maildir path must end with an '/'\n");
		exit(1);
	}
	
	int sockfd, newSockfd; 	
	
	//Process command line arguments
	int c;
	for (c = 1; c < args; c++){
		if (c == 1){
			sockfd = socket_setup(atoi(argv[c])); 
		}
		else if (c == 2){
			dir_set_path(argv[c]); 
		}
		else if (c == 3){
			log_setUp (argv[c]); 
		}
	}
    
	//Creation of thread 
    pthread_t thread_id[100];
    int c_tnum = 0;
 
	//Listen for new connections and create a new thread for every new 
	//connection 
    log_write("Main" ,"POP3 Server Started" , "", "");
    while(1){
        newSockfd = socket_get_new_connection(sockfd);
        int rc;
        rc = pthread_create(&thread_id[c_tnum], NULL, 
                            (void *)pop_protocol, (void *)newSockfd);
        
        log_write("Main" ,"New Connection" , "", "");
        c_tnum++;
    }
		
}
