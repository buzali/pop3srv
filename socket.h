
/* Set up the socket */
int socket_setup(int port);

/* This call will block untill a new incoming connection.*/
int socket_get_new_connection(int socket_descriptor);


/* The functions used by the POP thread. */

char* socket_get_line(int connection_descriptor);
void socket_write(int connection_descriptor, char* string);
void close_connection(int connection_descriptor);
void close_socket(int socket_descriptor);

