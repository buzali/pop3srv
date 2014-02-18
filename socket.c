
/*
*Subject	: COMP30017 Operating System and Network Services
*Project	: Project 2
*FileName	: main.c
*Author		: Tofi Buzali 513816 & Lucy Munanto 346003
*/

#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>




/*
* Creates a new socket and returns the socket descriptor.
*/
int socket_setup(int portno){
	int sockfd;
	struct sockaddr_in serv_addr; 
	
	//If Port Number isn't specified
	if (portno == (int)NULL){
		fprintf(stderr, "ERROR, no port provided\n"); 
		exit(1); 
	}

	//Create a new socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		perror("ERROR opening socket"); 
		exit(1); 

	}
	
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr = INADDR_ANY; 
	serv_addr.sin_port = htons(portno); 

	//Bind the socket to a Port Number
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR on binding"); 
		exit(1); 
	}
	return sockfd;

}

/*
* Listens for a new connection,
* once there is a new connection
* the file descriptor of the new open connection is returned.
*/
int socket_get_new_connection(int sockfd){
	
	int newSockfd; 
	int clilen; 
	struct sockaddr_in cli_addr; 
	//Listen for a new connection
	listen(sockfd, 100); 
	clilen = sizeof(cli_addr); 

	/*Accept a connection - block until a connection is ready to be
	 * accepted. newSockfd is the new file descriptor to communicate on*/
	newSockfd = accept(sockfd, (struct sockaddr*)&cli_addr, (unsigned int*)&clilen);
	if (newSockfd < 0){
		perror ("ERROR on accept"); 
		exit(1); 
	}
	return newSockfd; 
}

/*
* Reads a 'line' from the given connection.
* A 'line' means a sequence of characters ending with a CRLF pair.
* The string being returned must include the CRLF pair.
*/
char* socket_get_line(int fd){
	int n;
    char *buffer = malloc(256);
	n = (int)read (fd, buffer, 255); 
	if (n < 0) {
		perror("ERROR reading from socket"); 
		exit(1); 
	}
	
	return buffer; 

}


/*
* Writes the given string to the given connection descriptor.
*/
void socket_write(int fd, char* string){
	
	int n; 
	
	n = (int) write (fd, string, strlen(string)); 
	if (n < 0){
		perror("ERROR writing to socket"); 
		exit(1); 
	}
}


/*
* Closes the given connection descriptor.
*/
void close_connection(int fd){
	
	close(fd); 
}

/*
* Closes the given socket.
*/
void close_socket(int fd){
    
    close(fd);

}


