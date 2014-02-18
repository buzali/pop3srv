
/*
 *Subject	: COMP30017 Operating System and Network Services
 *Project	: Project 2
 *FileName	: main.c
 *Author		: Tofi Buzali 513816 & Lucy Munanto 346003
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>


#include "maildir.h"
#include "logging.h"


/*Global Variables*/ 
char *dir_path;

/*
 * This function is to be called once at the beginning
 * to set the root path of the maildirs.
 */
void dir_set_path(char* path){
    dir_path = path;        
}


/*
 * This function checks the given username and password
 * against the stored values.
 *
 * If the username is valid (has a corresponding folder)
 * and the given password matches the pass file 
 * then the function should return 'true'.
 *
 * Otherwise it is to return 'false'.
 *
 * Note 'true' and 'false' are defined in maildir.h
 */
bool check_user(char* username, char* password){
    
    char pass_path[1024];    
    sprintf(pass_path, "%s%s/pass", dir_path, username);
    printf("%s\n",pass_path);
    
    FILE *pass_file= fopen(pass_path, "r");
    if (!pass_file){
        return false;
    }
	
    //Buffer for password
    char real_pass[20];
    
    //Read up to 20 characters of password 
    fgets(real_pass, 21, pass_file);
    fclose(pass_file);
    
    //Compare password with file pass
    if (strcmp(password, real_pass) == 0)
        return true;
    else
        return false;
}

/*
 * Constructs a FilesStruct as described in maildir.h
 * for the maildir of the given user.
 */
FilesStruct* dir_get_list(char* user){
    
    DIR *dir;
    struct dirent *ent;
    char user_dir[1024];
    sprintf(user_dir, "%s/%s", dir_path, user);
    dir = opendir(user_dir);
    
    FilesStruct *fs = NULL;
    char *file_name;
    if (dir !=NULL)
    {
        //Allocate space for FilesStruct
        fs = malloc(sizeof(FilesStruct));
        fs->count = 0;
        int size;
        char **files_a = malloc(1000*sizeof(char));
        int *sizes = malloc(1000*sizeof(int));
        char *file_path = malloc(100*sizeof(char));
        struct stat file_status;
        FILE *f;
        
        
        //Read files in directory
        while ((ent = readdir(dir)) != NULL)
        {
            file_name = malloc(100*sizeof(char));
            strcpy(file_name, ent->d_name);
            //ignores password file and files beggining with dot
            if (strcspn(file_name, ".")!=0 && strcmp(file_name, "pass")!=0)
            {
                
                sprintf(file_path, "%s%s/%s", dir_path,user, file_name);
                files_a[fs->count] = file_name;
                
                if(stat(file_path, &file_status) != 0)
                    printf("ERROR file stat");
                f= fopen(file_path, "rb");
                if (!f){
                    printf("FILE %s does not exist\n", file_path);
                }
                
                //Calculate file size
                fseek(f, 0, SEEK_END); // seek to end of file
                size = (int) ftell(f); // get current file pointer
                fseek(f, 0, SEEK_SET); // seek back to beginning of file
                
                sizes[fs->count] = size;
                
                fs->count++;
            }
            
            
        }
        (void)closedir(dir);
        fs->FileNames = files_a;
        fs->FileSize = sizes;
        
    }
    else
        printf("Couldn't open the directory");
    
	log_write("MailDir", user, "GetList", ""); 
    return fs;
}

/*
 * Delete the given file from the maildir of the given user.
 */
void delete_mail(char* user, char* filename){
    if (strcmp(filename, "pass") ==0)
        return;
    char file_path[1024];
    sprintf(file_path, "%s%s/%s", dir_path, user, filename);
	log_write("MailDir", user, "Delete", filename); 
    if (remove(file_path) !=0)
    {
        printf("ERROR DELETING %s\n", file_path);
    }
    
    
}

/*
 * Returns the contents of a given file from a user's maildir.
 *
 * This function MUST also byte-stuff the termination character 
 * as described in the project spec.
 *
 *This function MUST also append the termination character (`.') and a CRLF pair 
 * to the end.
 */
char* get_file(char* user, char* filename){
    
    char file_path[1024];
    sprintf(file_path, "%s%s/%s", dir_path, user, filename);
    FILE *f = fopen(file_path, "r");
    if (!f){
        printf("ERROR reading file %s\n", file_path);
        exit(1);
    }
    
    //Get file size
    struct stat file_status;
    stat(file_path, &file_status);
    int size = (int) file_status.st_size;
    
    //Allocate string space with size of file + 100 xtra for byte-stuffing
    char *mail = malloc((size+100)*sizeof(char));
    char lin[515];
    int pos;
    //BOOL flag to indicate whether mail string is initialized 
    int first = 0;
    //Read file line by line
    while(fgets(lin, 515, f)){
        pos = (int)strcspn(lin,".");
        //Prepend dot when there's a dot in the beginning
        if (pos == 0)
        {
            //If first, initialize mail string with strcpy
            if (first ==0){
                strcpy(mail, ".");
                first = 1;
            }
            else
                strcat(mail, ".");
        }
        //If first, initialize mail string with strcpy
        if (first ==0){
            strcpy(mail, lin);
            first = 1;
        }
        else
            strcat(mail, lin);
    }
    //Append termination octet
    strcat(mail, ".\r\n");
    fclose(f);
	log_write("MailDir", user, "GetFile", filename); 
    return mail;
}

