
typedef int bool;

#define true 1
#define false 0


struct files_struct{
	/*The number of email messages in a maildir.*/
	int count; 
	
	/*A pointer to an array of pointers to the strings of the filenames. */
	char **FileNames;
	
	/*A pointer to an array of ints that give the corresponding size of the file.*/
	int  *FileSize;
    
};

typedef struct files_struct FilesStruct;


void dir_set_path(char* path);
bool check_user(char* username, char* pass);
FilesStruct* dir_get_list(char* user);
void delete_mail(char* user, char* filename);
char* get_file(char* user, char* filename);

