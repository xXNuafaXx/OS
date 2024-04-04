#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
//Description: The proposed project combines functionalities for monitoring a directory to manage differences between two captures (snapshots) of it.
// The user will be able to observe and intervene in the changes in the monitored directory.

//Directory Monitoring:
//The user can specify the directory to be monitored as an argument in the command line, and the program will track
// changes occurring in it and its subdirectories, parsing recursively each entry from the directory.
//With each run of the program, the snapshot of the directory will be updated, storing the metadata of each entry.

void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}


char  *itoa(int n){
    char s[32];
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
    return strdup(s);
}

int findAvailableSnapshotNumber(char *path){

    unsigned long pathSize = strlen(path) + strlen("Snapshots/") + 5;
    char tempPath[pathSize];
    strcpy(tempPath,path);
    strcat(tempPath,"Snapshots/");

    struct stat st = {0};

    //If the ~/Snapshots folder does not exist we create it
    if (stat(tempPath, &st) == -1) {
        mkdir(tempPath, 0700);
    }

    //In case mkdir fails we check for the existance of the directory
    DIR *directory;
    directory = opendir(tempPath);
    if(directory == NULL){
        perror("Could not open ~/Snapshots");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    struct dirent *fileInfo;
    while (( fileInfo = readdir(directory)) != NULL){
        if(strstr(fileInfo->d_name,"snapshot"))
            count++;
    }
    return count;
}

char *generateName(char *path){
    char tempName[1024] = "\0";
    strcpy(tempName,path);
    for (int i = 0; i < strlen(path); ++i) {
        if(tempName[i] == '/')
            tempName[i] = '-';
    }
    /*time_t t;
    time(&t);
    strcat(tempName, ctime(&t));*/
    return strdup(tempName);
}

char *makeName(char *path){
    unsigned long pathSize = strlen(path) + strlen("Snapshots/") + 5;
    char tempPath[pathSize];
    strcpy(tempPath,path);
    strcat(tempPath, "Snapshots/snapshot");
    strcat(tempPath, generateName(path));
    strcat(tempPath, ".txt");
    return strdup(tempPath);
}

char *makePath(char *path){
    unsigned long pathSize = strlen(path) + strlen("Snapshots/") + 5;
    char tempPath[pathSize];
    strcpy(tempPath,path);
    strcat(tempPath, "Snapshots/snapshot");
    //strcat(tempPath, itoa(findAvailableSnapshotNumber(path)));
    strcat(tempPath, ".txt");
    return strdup(tempPath);
}

int getFileDescriptor(char *path){
    int fileDescriptor = open(path,O_WRONLY | O_CREAT | O_TRUNC, 0777);
    return fileDescriptor;
}

void parseDirectory(DIR *directory, char path[],int fd){

    //We go through every file of the directory
    struct dirent *fileInfo;
    while (( fileInfo = readdir(directory)) != NULL) {
        char *nextFolderName = fileInfo->d_name;




        if (nextFolderName[0] != '.'){

            //We write info about the file in snapshot.txt ===TBD===
            char buff[4096];
            strcpy(buff,"\0");
            strcat(buff, nextFolderName);
            strcat(buff," - ");
            strcat(buff, path);
            strcat(buff, "\n");
            write(fd,buff, strlen(buff));

            //We generate the path to the next possible directory
            char tempPath[1000] = "";
            strcat(tempPath,path);
            strcat(tempPath,nextFolderName);
            strcat(tempPath,"/");

            //Debug purposes
            printf("%s - %s\n", nextFolderName, tempPath);

            //The next directory is initialised
            //If not null it will be parsed
            DIR *nextFolder = opendir(tempPath);
            if (nextFolder != NULL) {
                parseDirectory(nextFolder, tempPath, fd);
            }
            close(fd);
        }

    }
}


int main(int argc, char* argv[]){
    if (argc <= 2) {
        printf("Wrong number of arguments\n");
        return -1;
    }
    char path[100] = "";
    strcpy(path,argv[3]);
    /*printf("%s\n", path);
    printf("%s", makePath(path));*/
    //char *path = "/home/florin/Desktop/os/";

    DIR *directory = opendir(path);
    if (directory == NULL){
        printf("Could not open directory\n");
        return -1;
    }
    //printf("%d\n",findAvailableSnapshotNumber(path));
    char snapshotPath[1024] = "\0";
    strcpy(snapshotPath,argv[2]);
    strcat(snapshotPath,"snapshots/");
    //printf("%s",snapshotPath);
    int fd = getFileDescriptor(makePath(snapshotPath));
    parseDirectory(directory, path, fd);
    //printf("%s", makePath(path));
    //printf(" %d", getFileDescriptor(makePath(path)));
    return 0;
}
