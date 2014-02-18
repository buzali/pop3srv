
/*
*Subject	: COMP30017 Operating System and Network Services
*Project	: Project 2
*FileName	: main.c
*Author		: Tofi Buzali 513816 & Lucy Munanto 346003
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include "pop.h"
#include "socket.h"
#include "maildir.h"
#include "logging.h"

#define maxUserLen 50
#define maxPassLen 20

void send_ok(char* string, int cd);
void send_err(char* string, int cd);
void get_cmd(int connection_descriptor, char** cmd, char** rest);


/*
 * This function is the starting point of the POP protocol
 * on the given connection.
 */
void pop_protocol(int connection_descriptor){
    
    send_ok("Hi to my little server", connection_descriptor);
    
    int auth= 0; //Bool authentication status
    char *response;
    char *cmd;
    char *rest;
    
    
    //AUTHORIZATION STATE
    char *usr = NULL;
    char *pass = NULL;
    
    //While user not authorized
    while (!auth){
        
        get_cmd(connection_descriptor, &cmd, &rest);
        
        if (!rest){
            cmd = (char *)strtok_r(response, "\r\n", &rest);
        }
        
        if(cmd && !strcasecmp(cmd, "user")){
            usr = rest;
            log_write("POP","No Auth", "USER" , usr);
            send_ok("got username", connection_descriptor);
        }
        //If command PASS and we've got a user
        else if (cmd && usr && !strcasecmp(cmd, "pass") ){
            pass = rest;
            log_write("POP","No Auth", "PASS" , pass);
            
            //Check user password
            auth = check_user(usr, pass);
            if (auth){
                send_ok("authenticated ok", connection_descriptor);
                log_write("POP",usr , "Authenticated" , "");
            }
            else{
                send_err("auth failed", connection_descriptor);
                log_write("POP",usr , "Auth failed" , "");
                //If password incorrect, reset usr var.
                usr = NULL;
            }
        }
        else if (cmd && !strcasecmp(cmd, "quit")){
            log_write("POP","No Auth", "QUIT" , "");
            send_ok("closing connection", connection_descriptor);
            close_connection(connection_descriptor);
            //Return to main once the connection is closed
            return;
        }
        else
        {
            send_err("invalid command", connection_descriptor);
        }
        
    }
    
    //TRANSACTION STATE
    log_write("POP",usr, "TRANSACTION STATE" , "");
    
    //Obtain maildrop
    assert(auth == 1);
    int quit = 0;
    FilesStruct *md = dir_get_list(usr);
    
    int marked_del[md->count]; //array with bool deleted or not
    int num_deleted =0; //number of deleted messages
    int i;
    
    if (md !=NULL)
    {
        //initialize marked_del with zeros
        for (i=md->count;i--;)
            marked_del[i]=0;
        
        //Returning string 
        char return_str[1024];
        
        //While user hasn't quit
        while (!quit){
            get_cmd(connection_descriptor, &cmd, &rest);
            
            if(cmd && !strcasecmp(cmd, "stat")){
                log_write("POP",usr , "STAT" , "");
                
                int cnt = 0;
                int size = 0;
                for (i=md->count;i--;)
                {
                    //Check if file is deleted or not
                    if (!marked_del[i]){
                        cnt++;
                        size += md->FileSize[i];
                    }
                }
                sprintf(return_str, "%d %d", cnt, size);
                send_ok(return_str, connection_descriptor);
            }
            else if (cmd && !strcasecmp(cmd, "list")){
                
                //rest is the msg no.
                int msg_num =0;
                if (rest)
                    msg_num = atoi(rest);

                //List specific message details
                if (msg_num>0){
                    
                    log_write("POP",usr , "LIST" , rest);
                    
                    //check message is valid
                    if  ( msg_num>0 && msg_num <= (md->count )){
                        //check msg is not marked as deleted
                        if (!marked_del[msg_num - 1]){
                            sprintf(return_str,
                                "%d %d", msg_num, md->FileSize[msg_num - 1]);
                            send_ok(return_str, connection_descriptor);
                        }else
                        {
                            send_err("No such message", connection_descriptor);
                        }
                    }else
                    {
                        send_err("No such message", connection_descriptor);
                    }
                }
                //List all email messages
                else
                {
                    log_write("POP",usr , "LIST" , "");
                    sprintf(return_str, 
                            "got %d messages:", md->count - num_deleted);
                    send_ok(return_str, connection_descriptor);
                    for (i=0;i<md->count;i++)
                    {
                        if (!marked_del[i]){
                            sprintf(return_str, 
                                    "%d %d\r\n", i+1, md->FileSize[i]);
                            socket_write(connection_descriptor, return_str);
                        }
                    }
                    socket_write(connection_descriptor, ".\r\n");
                }
                
            }
            //Retrieve the mail
            else if (cmd && !strcasecmp(cmd, "retr")){
                int msg_num = atoi(rest);
                //check message is valid
                if  ( msg_num>0 && msg_num <= (md->count )){
                    //check msg is not marked as deleted
                    if (!marked_del[msg_num - 1]){
                        sprintf(return_str, 
                                "%d octets:", md->FileSize[msg_num -1]);
                        send_ok(return_str, connection_descriptor);
                        log_write("POP",usr , "RETR" , rest);
                        char *msg = get_file(usr, md->FileNames[msg_num -1]);
                        socket_write(connection_descriptor, msg);
                    }else
                    {
                        send_err("No such message", connection_descriptor);
                    }
                }else
                {
                    send_err("No such message", connection_descriptor);
                }
            }
            //Mark file as deleted
            else if (cmd && !strcasecmp(cmd, "dele")){
                
                int msg_num = atoi(rest);
                
                //check message is valid
                if  ( msg_num>0 && msg_num <= (md->count )){
                    //check msg is not marked as deleted
                    if (!marked_del[msg_num - 1]){
                        //Mark as deleted
                        marked_del[msg_num -1] = 1;
                        sprintf(return_str, "message %d deleted", msg_num);
                        num_deleted++;
                        send_ok(return_str,connection_descriptor);
                        log_write("POP",usr , "DELE" , rest);
                    }else
                    {
                        sprintf(return_str, 
                                "message %d already deleted", msg_num);
                        send_err(return_str, connection_descriptor);
                    }
                }else
                {
                    send_err("No such message", connection_descriptor);
                }
            }
            //Noop command
            else if (cmd && !strcasecmp(cmd, "noop")){
                send_ok(NULL, connection_descriptor);
                log_write("POP",usr , "NOOP" , "");
            } 
            //Reset command, that undoes deleting of all files marked deleted
            else if (cmd && !strcasecmp(cmd, "rset")){
                num_deleted = 0;
                for (i=md->count;i--;)
                    marked_del[i]=0;
                sprintf(return_str, "got %d messages", md->count);
                send_ok(return_str, connection_descriptor);
                log_write("POP",usr , "RSET" , "");
            }
            //Quit
            else if (cmd && !strcasecmp(cmd, "quit")){
                log_write("POP",usr , "QUIT" , "");
                quit = 1;
            }
            else
            {
                send_err("invalid command", connection_descriptor);
            }
        }
    }else
        printf("ERROR obtaining maildrop");
    
    //UPDATE STATE
    if (quit && auth){
        log_write("POP",usr, "UPDATE STATE" , "");
        for (i=md->count;i--;)
        {
            //Actually delete the files 
            if (marked_del[i] == 1)
            {
                delete_mail(usr, md->FileNames[i]);
            }
        }
        log_write("POP",usr, "Closing Connection" , "");
        send_ok("see ya", connection_descriptor);
        close_connection(connection_descriptor);
        
    }
    
}

/*
 *This function reads the line from the socket, and splits the line for 
 *command and argument (rest)
 */
void get_cmd(int connection_descriptor, char** cmd, char** rest)
{
    char *response =NULL;
    response = socket_get_line(connection_descriptor);
    *cmd = (char *)strtok_r(response, " ", rest);
    *rest = (char *)strtok_r(*rest, "\r\n", rest);
    
    if (!*rest){
        *cmd = (char *)strtok_r(*cmd, "\r\n", rest);
    }
    
}

//Send ok string to connection cd
void send_ok(char* string, int cd){
    
    char st[1024];
    if (string)
        sprintf(st, "+OK %s\r\n", string);
    else
        sprintf(st, "+OK\r\n");
    socket_write(cd, st);
    
}

//Send ok string to connection cd
void send_err(char* string, int cd){
    
    char st[1024];
    sprintf(st, "-ERR %s\r\n", string);
    socket_write(cd, st);
    
}
