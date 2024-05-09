#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define PATH_SIZE 100
#define MAX_BUFF_SIZE 4096

char *makeSnapshotName(char *path){
    char tempPath[PATH_SIZE] = "\0";
    strcpy(tempPath,path);
    for (int i = 0; i < strlen(tempPath); ++i) {
        if(tempPath[i] == '/')
            tempPath[i] = '_';
    }
    tempPath[strlen(tempPath)-1] = '\0';
    return strdup(tempPath);
}

char *makeSnapshotPath(char *snapshotPath, char *dirPath){
    unsigned long pathSize = PATH_SIZE + strlen("Snapshots/") + 5;
    char tempPath[pathSize];
    strcpy(tempPath, snapshotPath);
    strcat(tempPath, "snapshot");
    strcat(tempPath, makeSnapshotName(dirPath));
    strcat(tempPath, ".txt");
    return strdup(tempPath);
}

int getFileDescriptor(char *path){
    int fileDescriptor = open(path,O_WRONLY | O_CREAT | O_TRUNC, 0777);
    return fileDescriptor;
}

int checkPermissions(char *path){
    struct stat st;
    if (lstat(path, &st) == -1) {
        if (errno == ENOENT) {
            printf("File %s does not exist.\n", path);
        } else {
            //perror("stat - checkPermissions()");
        }
    }
    int hasPermissions = 0;

    //Check if it's readable, writable or executable in any group
    if (st.st_mode & S_IRUSR || st.st_mode & S_IRGRP || st.st_mode & S_IROTH
     || st.st_mode & S_IWUSR || st.st_mode & S_IWGRP || st.st_mode & S_IWOTH
     || st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH) {
        hasPermissions = 1;
    }

    return hasPermissions;
}

void moveFile(const char *filename, const char *destinationFolder) {
    char command[1024];

    // Construct the command to move the file using system function
    sprintf(command, "mv %s %s/", filename, destinationFolder);

    // Execute the command
    int result = system(command);

    // Check if the command was executed successfully
    if (result == 0) {
        perror("File moved successfully.\n");
    } else {
        perror("Failed to move the file.\n");
    }
}

int analyzeFile(char *path){
    int give_access=chmod(path, 7777); //giving read access to the "malicious file"
    path[strlen(path)-1] = '\0';
    char argument[100];  //making the command to run the script
    snprintf(argument, sizeof(argument), "./analysis.sh \'%s\'", path);

    int file_status = system(argument); //executing the script
    if(file_status != 0) printf("virusat\n");
    else printf("ok\n");

    give_access = chmod(path, 0); //changing again the acces rights to 0
    close(give_access);

    return file_status;
}

int checkFile(char *path){
    if(checkPermissions(path) == 0){
        printf("am intrat === ");
        return analyzeFile(path);
    }
    return 0;
}

void parseDirectory(DIR *directory, char path[],int fd){

    //We go through every file of the directory
    struct dirent *fileInfo;
    while (( fileInfo = readdir(directory)) != NULL) {
        char *nextFolderName = fileInfo->d_name;

        if (nextFolderName[0] == '.')
            continue; // if the folder is "." or ".." there is no need to parse it

        //We write info about the file in the snapshot .txt file
        char buff[MAX_BUFF_SIZE];
        strcpy(buff,"\0");
        strcat(buff, nextFolderName);
        strcat(buff," - ");
        strcat(buff, path);
        strcat(buff, "\n");
        write(fd,buff, strlen(buff));

        //We generate the path to the next possible directory
        char tempPath[PATH_SIZE] = "";
        strcat(tempPath,path);
        strcat(tempPath,nextFolderName);
        strcat(tempPath,"/");

        //printf("=== %d - %s ===\n", checkPermissions(tempPath),tempPath);
        if(checkFile(tempPath) == 0){
            printf("path -> %s\n",tempPath);
        }

        //The next directory is initialised and if not null it will be parsed
        DIR *nextFolder = opendir(tempPath);
        if (nextFolder != NULL) {
            //printf("%s === %d\n",tempPath,fd);
            parseDirectory(nextFolder, tempPath, fd);
        }
    }
}

int main(int argc, char* argv[]){
    if (argc <= 4) {
        printf("Wrong number of arguments\n");
        return -1;
    }
    if (strcmp(argv[1],"-o") != 0) {
        printf("Second argument should be -o\n");
        printf("And state the output folder for the snapshots \n");
        return -1;
    }
    if (strcmp(argv[3],"-s") != 0) {
        printf("Fourth argument should be -o\n");
        printf("And state the isolation folder for malicious files\n");
        return -1;
    }
    char path[PATH_SIZE] = "\0";

    for (int i = 3; i < argc; ++i) {
        pid_t childPid;
        childPid = fork();

        if(childPid == 0) {
            int fd = getFileDescriptor(makeSnapshotPath(argv[2], argv[i]));
            strcpy(path, argv[i]);
            //printf("%s\n",path);
            DIR *directory = opendir(path);
            if (directory == NULL) {
                printf("Could not open directory\n");
                return -1;
            }
            parseDirectory(directory, path, fd);
            return 0;
        }
    }
    for (int i = 3; i < argc; ++i) {
        wait(NULL);
    }
    return 0;
}
